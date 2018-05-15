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

#include "IoToriiOperation.h"

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "src/linklayer/common/eGA3Frame.h"
#include "src/linklayer/common/HLMACAddress.h"
#include <vector>

#include "HLMACAddressTable.h"
#include "inet/linklayer/csma/CSMAFrame_m.h"



namespace iotorii {
using namespace inet;


Define_Module(IoToriiOperation);

void IoToriiOperation::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {

        numProcessedFrames = numDiscardedFrames = 0;
        helloInterval = par("helloInterval");
        numNeighbors = 0;
        maxNeighbors = 3;
        HelloRcvd = 0;
        WATCH(numProcessedFrames);
        WATCH(numDiscardedFrames);
        WATCH(HelloRcvd);
        WATCH(helloInterval);
        WATCH(maxNeighbors);
        WATCH_VECTOR(neighborList);

    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        HelloTimer = new cMessage("HelloTimer");
        scheduleAt(simTime()+helloInterval, HelloTimer); //Next Hello broadcasting

        hlmacTable = check_and_cast<IHLMACAddressTable *>(getModuleByPath(par("hlmacTablePath")));

        corePrefix = par("corePrefix");
        if (isCoreSwitch = par("isCoreSwitch"))
        {
            EV<< "This switch is a core switch and its prefix is  "<< corePrefix << "\n";
            startCoreEvent = new cMessage("startCoreEvent");
            coreStartTime = par("coreStartTime");
            scheduleAt(simTime() + coreStartTime, startCoreEvent);

            WATCH(coreStartTime);
            WATCH(corePrefix);

        }
    }
}

void IoToriiOperation::sendAndScheduleHello()
{
    EV << "->IoToriiOperation::sendAndScheduleHello()" << endl;

    //scheduling next Hello packet
    scheduleAt(simTime()+helloInterval, HelloTimer); //Next Hello broadcasting

    CSMAFrame *macPkt = new CSMAFrame("Hello!");
    macPkt->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    EV << "Hello message from this node is broadcasted to all node in the range. " << endl;
    send(macPkt, "lowerLayerOut");

    EV << "<-IoToriiOperation::sendAndScheduleHello()" << endl;
}

void IoToriiOperation::sendToNeighbors(CSMAFrame *frame)
{
    EV << "->IoToriiOperation::sendToNeighbors()" << endl;

    EV << "This node has " << numNeighbors << " neighbors, " << "maximum number of allowed neighbors is " << maxNeighbors << endl;
    for (int i = 1; i <= numNeighbors && i <= maxNeighbors; i++) {  //Second condition is related on the width of HLMACAddress, 3 bits allowed 3 neighbor, 00 is reserved for NOTSPECIFIED address
        eGA3Frame eGA3(frame->getSrcAddr()); //eGA3Frame eGA3 = eGA3Frame(frame->getSrcAddr());
        HLMACAddress hlmac = eGA3.getHLMACAddress();
        //unsigned char type = eGA3.geteGA3FrameType();
        hlmac.addNewId(i);
        eGA3.setHLMACAddress(hlmac);
        CSMAFrame *dupFrame = frame->dup();
        dupFrame->setSrcAddr(MACAddress(eGA3.getInt()));
        dupFrame->setDestAddr(neighborList[i-1]);
        emit(LayeredProtocolBase::packetSentToLowerSignal, frame);
        EV << "SetHLMAC frame " << eGA3 << " is sent to the neighbor with suffix # (" << i << ") and MAC address (" << dupFrame->getDestAddr() << ")" << endl;
        send(dupFrame, "lowerLayerOut");

    }

    delete frame;
    EV << "<-IoToriiOperation::sendToNeighbors()" << endl;
}

void IoToriiOperation::startCore(int core)
{
    EV << "->IoToriiOperation::startCore()" << endl;

    //scheduling next Core event
    scheduleAt(simTime() + coreStartTime, startCoreEvent);

    //preparing SetHLMAC frame
    HLMACAddress coreAddress;             // create HLMAC
    coreAddress.setCore((unsigned char)core);  //insert core prefix in it
    EV << "Core address " << coreAddress << " is assigned to this node." << endl;
    saveHLMAC(coreAddress);     //assigns coreAddress to itself
    unsigned char type = 1;               // if assume that type of SetHLMAC frame is 1
    eGA3Frame eGA3(type,coreAddress);

    //preparing CSMA frame
    CSMAFrame *SetHLMACFrame = new CSMAFrame ("SetHLMAC");
    MACAddress source(eGA3.getInt());     // like cast, converting eGA3 to MACAddress, and replacing to source variable
    SetHLMACFrame->setSrcAddr(source);
    EV << "SetHLMAC frame " << eGA3 << " ( converted to MAC structure : " << SetHLMACFrame->getSrcAddr() << ") is disseminated to the neighbors." << endl;
    sendToNeighbors(SetHLMACFrame);

    EV << "<-IoToriiOperation::startCore()" << endl;
}

void IoToriiOperation::receiveSetHLMACMessage(CSMAFrame *frame)
{
    EV << "->IoToriiOperation::receiveSetHLMACMessage()" << endl;

    eGA3Frame eGA3(frame->getSrcAddr()); //eGA3Frame eGA3 = eGA3Frame(frame->getSrcAddr());  // extract eGA3 frame (data) from SetHLMAC CSMAFrame
    HLMACAddress hlmac = eGA3.getHLMACAddress();  // extract HLMAC address from eGA3 frame (data)

    if (!hasLoop(hlmac)){
        saveHLMAC(hlmac);
        EV << "HLMAC address " << hlmac << " is assigned to this node." << endl;
        EV << "Frame " << frame->getName() << " (dst mac address: " << frame->getDestAddr() << " )" << " is resent to neighbors." << endl;
        sendToNeighbors(frame);
    }else{
        EV << "Because of loop creation, HLMAC address " << hlmac << " is not assigned to this node." << endl;
        EV << "Frame " << frame->getName() << " (dst mac address: " << frame->getDestAddr() << " )" << " is deleted." << endl;
        delete frame;
    }

    EV << "<-IoToriiOperation::receiveSetHLMACMessage()" << endl;
}

bool IoToriiOperation::hasLoop(HLMACAddress hlmac)
{
    EV << "->IoToriiOperation::hasLoop()" << endl;

    HLMACAddress longestPrefix = hlmacTable->getlongestMatchedPrefix(hlmac);
    if (longestPrefix == HLMACAddress::UNSPECIFIED_ADDRESS){
        EV << "HLMAC adress " << hlmac << " does not create a loop in this node. Longest Matched Prefix is " << longestPrefix << endl;
        return false;
    }
    else{
        EV << "HLMAC adress " << hlmac << " creates a loop in this node. Longest Matched Prefix is " << longestPrefix << endl;
        return true;
    }

    EV << "<-IoToriiOperation::hasLoop()" << endl;
}

void IoToriiOperation::saveHLMAC(HLMACAddress hlmac)
{
    EV << "->IoToriiOperation::saveHLMAC()" << endl;

    hlmacTable->updateTableWithAddress(-1, hlmac);
    EV << "HLMAC adress " << hlmac << " is assigned to this node." << endl;

    EV << "<-IoToriiOperation::saveHLMAC()" << endl;
}

void IoToriiOperation::handleMessage(cMessage *msg)
{
    EV << "->IoToriiOperation::handleMessage()" << endl;

    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

    if (msg->isSelfMessage()) {
        if (msg == HelloTimer) {
            sendAndScheduleHello();
            return;
        }

      if (msg == startCoreEvent) {
          startCore(corePrefix);
          return;
      }

    }

 /*  data frames: sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
    nbRxFrames++;
    delete msg;    */

    CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);
    // Frame received from MAC unit
    emit(LayeredProtocolBase::packetReceivedFromLowerSignal, frame);
    handleAndDispatchFrame(frame);

    EV << "<-IoToriiOperation::handleMessage()" << endl;
}

void IoToriiOperation::handleAndDispatchFrame(CSMAFrame *frame)
{
    EV << "->IoToriiOperation::handleAndDispatchFrame()" << endl;

    numProcessedFrames++;

    if ((strcmp(frame->getName(),"Hello!")==0))
    {
        HelloRcvd++;
        //to check duplicate hello
        bool isDuplicate = false;
        for(int i=0; i<numNeighbors; i++)
            if (frame->getSrcAddr() == neighborList[i])
                isDuplicate = true;

        if (!isDuplicate){
            neighborList.push_back(frame->getSrcAddr());
            EV << "Hello message is received from : " << frame->getSrcAddr() << "; Number of neighbors is " << neighborList.size() <<endl;
            numNeighbors++;
            return;
        }
    }

    //SetHLMAC
    if ((strcmp(frame->getName(),"SetHLMAC")==0))
    {
        receiveSetHLMACMessage(frame);
        return;
    }

    EV << "<-IoToriiOperation::handleAndDispatchFrame()" << endl;
}

void IoToriiOperation::start()
{
    EV << "->IoToriiOperation::start()" << endl;

    hlmacTable->clearTable();
    neighborList.clear();
    isOperational = true;

    EV << "<-IoToriiOperation::start()" << endl;
}

void IoToriiOperation::stop()
{
    EV << "->IoToriiOperation::stop()" << endl;

    hlmacTable->clearTable();
    neighborList.clear();
    isOperational = false;

    EV << "<-IoToriiOperation::stop()" << endl;
}

bool IoToriiOperation::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

void IoToriiOperation::finish()
{
    EV << "->IoToriiOperation::finish()" << endl;

    recordScalar("processed frames", numProcessedFrames);
    recordScalar("discarded frames", numDiscardedFrames);
    if (hlmacTable != nullptr)
        hlmacTable->printState();

    if (startCoreEvent != nullptr){
        cancelEvent(startCoreEvent);
        delete startCoreEvent;
        startCoreEvent = nullptr;
    }

    if (HelloTimer != nullptr){
        cancelEvent(HelloTimer);
        delete HelloTimer;
        HelloTimer = nullptr;
    }

    EV << "<-IoToriiOperation::finish()" << endl;
}

} // namespace iotorii

