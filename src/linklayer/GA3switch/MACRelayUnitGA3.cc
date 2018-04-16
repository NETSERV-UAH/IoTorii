
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
*/

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "MACRelayUnitGA3.h"
#include "src/linklayer/common/eGA3Frame.h"
#include "src/linklayer/common/HLMACAddress.h"
#include "HLMACAddressTable.h"
#include <vector>
#include "inet/linklayer/csma/CSMAFrame_m.h"


namespace iotorii {
using namespace inet;


Define_Module(MACRelayUnitGA3);

void MACRelayUnitGA3::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {

        numProcessedFrames = numDiscardedFrames = 0;

        helloInterval = par("helloInterval");
        WATCH(numProcessedFrames);
        WATCH(numDiscardedFrames);
        WATCH(HelloRcvd);
        WATCH(helloInterval);
        WATCH_VECTOR(neighborList);

    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        HelloTimer = new cMessage("HelloTimer");
        scheduleAt(simTime()+helloInterval, HelloTimer); //Next Hello broadcasting



        //startGA3Process();
/*

        //test HLMACAddress data structure
         HLMACAddress test("00.00.00.00.00.0");

         //test.setAddressByte(0, 15);
         test.setCore(1);
         test.addNewId(12);
         test.setAddressByte(5, 15);

         HLMACAddress prio1=test.getHLMACPrio();
         HLMACAddress hlmacsuffix=test.getHLMACSuffix();
         HLMACAddress priosuffix =prio1.getPrioSuffix();

         EV <<"HLMAC is :"<< test<<" prio is :"<< prio1<<" get hlmac hier "<<test.getHLMACHier()<<" prio hier "<<prio1.getPrioHier()<<" hl suf :"<<hlmacsuffix<<" prio suf :"<<priosuffix;
*/

/*
        //test eGA3Frame data structure
         HLMACAddress test("00.00.00.00.00.0");
         int type = 2;
         eGA3Frame eTest1(type,test),eTest2;
         eTest1.setCore(1);
         eTest1.addNewId(16);
         eTest1.addNewId(10);
         eTest1.addNewId(10);
         eTest1.addNewId(10);
         eTest1.addNewId(10);

         eTest2.setHLMACAddress(test);


         EV <<"eTest1 is :"<< eTest1<<" eTest2 :"<< eTest2<<" get test1 type: "<<eTest1.geteGA3FrameType()<<" get test2 type "<<eTest2.geteGA3FrameType()<<" test1 hlmac:"<<eTest1.getHLMACAddress()<<" test2 hlmac :"<<eTest2.getHLMACAddress();
*/
    }
}

void MACRelayUnitGA3::startGA3Process()   // GA3
{
    hlmacTable = check_and_cast<IHLMACAddressTable *>(getModuleByPath(par("hlmacTablePath")));

    ports = new (int[numVirtualPorts]);
    prio = new (HLMACAddress*[numVirtualPorts]); //numVirtualPorts rows of HLMACAddress*
    //startCoreEvent = new cMessage("startCoreEvent");
    startPortEvent = new cMessage("startPortEvent");
    startPortQueue = numVirtualPorts;

    for (int i=0; i<numVirtualPorts; i++)
    {
        EV<< "ID is assigned to  port port: "<<i+1<<" --> "<<i<< "\n";
        ports[i] = i+1;  // assign ID (i+1) to each port(i)
        prio[i] = nullptr; // assign prio (i+1) to each port(i)
        WATCH(ports[i]);
    }
    corePrefix = par("corePrefix");
    if (isCoreSwitch = par("isCoreSwitch"))
    {
        EV<< "This switch is a core switch and its prefix is  "<< corePrefix << "\n";
        corePrefixes.push_back(corePrefix);
        switchPrio = new HLMACAddress("00.00.00.00.00.0");  //highest priority

        timeBetweenCores = par("timeBetweenCores");
        timeBetweenPorts = par("timeBetweenPorts");
        if (startPortQueue > 0)
            scheduleAt(timeBetweenCores*(corePrefix-1), startPortEvent);  //  scheduleAt(0*timeBetweenPorts+(timeBetweenCores*(corePrefix-1)), startPortEvent);
    }
}

void MACRelayUnitGA3::flood(CSMAFrame *frame, int inputport)
{
    for (int i = 0; i < numVirtualPorts; i++) {
        if (i != inputport) {
            emit(LayeredProtocolBase::packetSentToLowerSignal, frame);
            eGA3Frame eGA3 = eGA3Frame(frame->getSrcAddr());
            eGA3.addNewId(ports[i]);
            CSMAFrame *dupFrame = frame->dup();
            dupFrame->setSrcAddr(MACAddress(eGA3.getInt()));
            send(dupFrame, "ifOut", i);
        }
    }

    delete frame;
}

void MACRelayUnitGA3::startCore(int core,int inputPort)
{
    //preparing SetHLMAC frame
    HLMACAddress coreAddress;             // create HLMAC
    coreAddress.setCore(core);  //insert core prefix in it
    coreAddress.addNewId(ports[inputPort]);  //add port id to HLMAC
    unsigned char type = 2;               // if assiume that type of frame is 2 : 2 ==> SetHLMAC
    eGA3Frame eGA3(type,coreAddress);
    MACAddress source(eGA3.getInt());     // like cast, converting eGA3 to MACAddress, and replacing to source variable
    //MACAddress destination;
    //destination.setBroadcast();

    //preparing Ethernet frame
    CSMAFrame *SetHLMACFrame = new CSMAFrame ("SetHLMAC");
    SetHLMACFrame->setSrcAddr(source);
    SetHLMACFrame->setDestAddr(MACAddress::BROADCAST_ADDRESS);


    //sending the frame
    emit(LayeredProtocolBase::packetSentToLowerSignal, SetHLMACFrame);
    send(SetHLMACFrame, "ifOut", inputPort);

}

void MACRelayUnitGA3::receiveMessage(CSMAFrame *SetHLMAC, int inputPort)
{
    eGA3Frame eGA3(SetHLMAC->getSrcAddr());  // extract eGA3 frame (data) from SetHLMAC CSMAFrame
    HLMACAddress HLMAC = eGA3.getHLMACAddress();  // extract HLMAC address from eGA3 frame (data)

    //if this SetHLMAC is the first frame, port table has no entry
    if (prio[inputPort] == nullptr) // (!hlmacTable->isPortInTable(inputPort))
    {
        EV<<"Beep0.1: port#" << inputPort <<" has no entry in HLMAC Address Table. HLMAC Address" << HLMAC << "is associated to port." << endl;

        //port: update HLMAC list
        hlmacTable->updateTableWithAddress(inputPort, HLMAC);

        //port: update prio
        //hlmacTable->setPrio(inputPort, HLMAC.getPrio());
        prio[inputPort] = new HLMACAddress(HLMAC.getHLMACPrio());
        EV << "Beep1.1: new port_prio (" << prio[inputPort][0] << ") extracted from HLMAC (" << HLMAC << ") is assignes to port " << inputPort << endl;

        //switch: update prio
        if (switchPrio == nullptr)  //if this SetHLMAC is the first SetHLMAC this switch is receiving
        {
            switchPrio = new HLMACAddress(HLMAC.getHLMACPrio());
            //switchPrio = HLMAC.getHLMACPrio();
            EV << "Beep2.1a: new switch_prio (" << *switchPrio << ") extracted from HLMAC (" << HLMAC << ") is assignes to switch prio" << endl;
            flood(SetHLMAC, inputPort);      //forward
        }else if ((*switchPrio) > HLMAC.getHLMACPrio()) // according line 16 pseducode
        {
            EV <<"previous switch prio : " << (*switchPrio) << ", new switch prio is : " << HLMAC.getHLMACPrio() <<" \n";
            (*switchPrio) = HLMAC.getHLMACPrio();
            EV << "Beep2.1b: new switch_prio (" << *switchPrio << ") extracted from HLMAC (" << HLMAC << ") is assignes to switch prio" << endl;
            flood(SetHLMAC, inputPort);      //forward
        }else
            EV <<"Beep2.1c: switch prio is not updated, previous switch prio (" << *switchPrio <<") has highest prio." << endl;

        //ports: update ids
        for (int p=0; (p<numVirtualPorts); p++)
        {
            if (prio[p] != nullptr) // or (hlmacTable->isPortInTable(p))
            {
                //crete new ID
                int pid;
                if ((prio[p][0].getPrioHier() > (*switchPrio).getPrioHier()) && ((prio[p][0].getPrioHier() - ((*switchPrio).getPrioHier()) <=2) || (isCoreSwitch)))
                {
                    pid = prio[p][0].getAddressByte((*switchPrio).getPrioHier());
                    EV<<"Beep3.1:, port#: "<< p << ", old pid: " << ports[p] <<", prio: "<<prio[p][0]<<", switch prio: "<< (*switchPrio) << ", switch hier:" <<(*switchPrio).getPrioHier()<<", new id: "<< pid <<endl;

                    bool isIdInPort = false;
                    for (int i=0; i<numVirtualPorts && !isIdInPort; i++)
                        if (pid == ports[i])
                            isIdInPort = true;

                    if (isIdInPort && isCoreSwitch)
                    {
                        EV << "Beep4.1: new port id (" << pid << ") is assignes to port " << p << "instead of previous id" << ports[p] << endl;
                        ports[p] = pid;  //new change
                        EV << "Beep5.1: new port id (" << pid << ") is similar to another port's id in this switch, new core prefix(" << corePrefix*10 + corePrefixes.size() << ") is created."<< endl;
                        corePrefixes.push_back(corePrefix*10 + corePrefixes.size());
                        startCore(corePrefix*10 + corePrefixes.size(), p);
                    }else{
                        EV << "Beep6.1: new port id (" << pid << ") is assigned to port " << p << "instead of previous id" << ports[p] << endl;
                        EV << "Beep7.1: new port id (" << pid << ") is unique in this switch, new core prefix is not created."<< endl;
                        ports[p] = pid;
                    }
                } //end pid
            }
         } // end for
    }else
         //article pseudocode
        if (prio[inputPort][0].getPrioHier() >= HLMAC.getHLMACHier())
        {
            //port: update HLMAC list
            if (prio[inputPort][0].getPrioSuffix() != HLMAC.getHLMACSuffix())
                hlmacTable->flush(inputPort);
            hlmacTable->updateTableWithAddress(inputPort, HLMAC);

            //port: update prio
            if (prio[inputPort][0] > HLMAC.getHLMACPrio())   //prio[inputPort][0] includes priority, not HLMAC
            {
                prio[inputPort][0] = HLMAC.getHLMACPrio();
                EV << "Beep1.2: new port_prio (" << prio[inputPort][0] << ") extracted from HLMAC (" << HLMAC << ") is assignes to port " << inputPort << endl;
            }

            //switch: update prio
            if ((*switchPrio)>HLMAC.getHLMACPrio())
            {
                EV <<"previous switch prio : " << (*switchPrio) << ", new switch prio is : " << HLMAC.getHLMACPrio() <<" \n";
                (*switchPrio) = HLMAC.getHLMACPrio();
                EV << "Beep2.2b: new switch_prio (" << *switchPrio << ") extracted from HLMAC (" << HLMAC << ") is assignes to switch prio" << endl;
                flood(SetHLMAC, inputPort);      //forward
            }else
                EV <<"Beep2.2c: switch prio is not updated, previous switch prio (" << *switchPrio <<") has highest prio." << endl;

            //ports: update ids
            for (int p=0; (p<numVirtualPorts); p++)
            {
                if (prio[p] != nullptr) // or (hlmacTable->isPortInTable(p))
                {
                    //crete new ID
                    int pid;
                    if ((prio[p][0].getPrioHier() > (*switchPrio).getPrioHier()) && ((prio[p][0].getPrioHier() - ((*switchPrio).getPrioHier()) <=2) || (isCoreSwitch)))
                    {
                        pid = prio[p][0].getAddressByte((*switchPrio).getPrioHier());
                        EV<<"Beep3.2:, port#: "<< p << ", old pid: " << ports[p] <<", prio: "<<prio[p][0]<<", switch prio: "<< (*switchPrio) << ", switch hier:" <<(*switchPrio).getPrioHier()<<", new id: "<< pid <<endl;

                        bool isIdInPort = false;
                        for (int i=0; i<numVirtualPorts; i++)
                            if (pid == ports[i])
                                isIdInPort = true;

                        if (isIdInPort && isCoreSwitch)
                        {
                            EV << "Beep4.2: new port id (" << pid << ") is assignes to port " << p << "instead of previous id" << ports[p] << endl;
                            ports[p] = pid;  //new change
                            EV << "Beep5.2: new port id (" << pid << ") is similar to another port's id in this switch, new core prefix(" << corePrefix*10 + corePrefixes.size() << ") is created."<< endl;
                            corePrefixes.push_back(corePrefix*10 + corePrefixes.size());
                            startCore(corePrefix*10 + corePrefixes.size(), p);
                        }else{
                            EV << "Beep6.2: new port id (" << pid << ") is assignes to port " << p << "instead of previous id" << ports[p] << endl;
                            EV << "Beep7.2: new port id (" << pid << ") is unique in this switch, new core prefix is not created."<< endl;
                            ports[p] = pid;
                        }
                    } //end pid
                }
             } // end for
        }//end else - end article pseudocode

}

void MACRelayUnitGA3::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

    if (msg->isSelfMessage()) {
        if (msg == HelloTimer) {
            sendHello();
            return;
        }

     /* if (msg == startCoreEvent) {
            for (int i=0; i<numVirtualPorts; i++)  // each core switch start broadcast SetHLMAC
                startCore(corePrefix,i);
            return;
        }*/

        if ((msg == startPortEvent) && (startPortQueue > 0)) {
            startCore(corePrefix, numVirtualPorts - startPortQueue);   // start broadcast SetHLMAC      numVirtualPorts - startPortQueue = portnumber
            if (startPortQueue > 1)
                scheduleAt((numVirtualPorts - startPortQueue-- + 1) * timeBetweenPorts+(timeBetweenCores*(corePrefix-1)), startPortEvent);
            return;
        }
    }

    CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);
    // Frame received from MAC unit
    emit(LayeredProtocolBase::packetReceivedFromLowerSignal, frame);
    handleAndDispatchFrame(frame);
}

//Method to create and send hello frames to other switches (currently only at the beginning).
void MACRelayUnitGA3::sendHello()
{
    EV << "->MACRelayUnitWGA3::sendHello()" << endl;
    CSMAFrame *macPkt = new CSMAFrame("Hello!");
    macPkt->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    send(macPkt, "ifOut", 0);

    //scheduling next Hello pavket
    scheduleAt(simTime()+helloInterval, HelloTimer); //Next Hello broadcasting

    EV << "MACRelayUnitWGA3::sendHello()<-" << endl;

}

void MACRelayUnitGA3::handleAndDispatchFrame(CSMAFrame *frame)
{
    int inputport = frame->getArrivalGate()->getIndex();

    numProcessedFrames++;

    if ((strcmp(frame->getName(),"Hello!")==0))
    {
        HelloRcvd++;
        neighborList.push_back(frame->getSrcAddr());
        EV << "Hello message is received from : " << frame->getSrcAddr() << "; Number of neighbors is " << neighborList.size() <<endl;
        return;
    }

    //GA3
    if ((strcmp(frame->getName(),"SetHLMAC")==0))
    {
        receiveMessage(frame, inputport);
        return;
    }
}

void MACRelayUnitGA3::start()
{
    hlmacTable->clearTable();  //GA3
    isOperational = true;
}

void MACRelayUnitGA3::stop()
{
    hlmacTable->clearTable();  //GA3
    isOperational = false;
}

bool MACRelayUnitGA3::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

void MACRelayUnitGA3::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDiscardedFrames);
    //GA3
    hlmacTable->printState();
    //cancelEvent(startCoreEvent);
    cancelEvent(startPortEvent);
    delete startPortEvent;
    startPortEvent = nullptr;
    delete switchPrio;
    switchPrio = nullptr;
    delete ports;
    ports = nullptr;


}

} // namespace iotorii

