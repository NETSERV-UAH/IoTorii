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

#include "src/linklayer/IoTorii/IoToriiOperation.h"
#include "src/linklayer/simpleidealmac/SimpleIdealWirelessMAC.h"

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "src/linklayer/common/eGA3Frame.h"
#include "src/linklayer/common/HLMACAddress.h"
#include <vector>

#include "HLMACAddressTable.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"


namespace iotorii {
using namespace inet;

void globalStatisticsMemoryAllocation(int numHosts){
    joiningTimeTotal = new simtime_t;  //default constructor returne 0
    firstGenerationTime = new simtime_t;
    lastReceivedTime = new simtime_t;

    numHLMACAssignedTotal = new int(0);
    numNeighborsTotal = new int(0);
    numAllowedNeighborsTotal = new int(0);
    numHelloSentTotal = new int(0);
    numHLMACSentTotal = new int(0);
    numNotJoinedTotal = new int(0);
    //listNotJoinedTotal = new IPv6Address(numHosts);
    listNotJoinedTotal = new MACAddress[numHosts];
    for (int i=0; i<numHosts; i++){
        //listNotJoinedTotal[i] = IPv6Address::UNSPECIFIED_ADDRESS;
        listNotJoinedTotal[i] = MACAddress::UNSPECIFIED_ADDRESS;
    }
    numWithoutNeighborTotal = new int(0);
}

void globalStatisticsMemoryDeallocation(){
    delete joiningTimeTotal;
    joiningTimeTotal = nullptr;  //for mutual exclusion, to prevent twice deleting
    delete firstGenerationTime;
    firstGenerationTime = nullptr;
    delete lastReceivedTime;
    lastReceivedTime = nullptr;

    delete numHLMACAssignedTotal;
    numHLMACAssignedTotal = nullptr;
    delete numNeighborsTotal;
    numNeighborsTotal = nullptr;
    delete numAllowedNeighborsTotal;
    numAllowedNeighborsTotal = nullptr;
    delete numHelloSentTotal;
    numHelloSentTotal = nullptr;
    delete numHLMACSentTotal;
    numHLMACSentTotal = nullptr;
    delete numNotJoinedTotal;
    numNotJoinedTotal = nullptr;
    delete[] listNotJoinedTotal;
    listNotJoinedTotal = nullptr;
    delete numWithoutNeighborTotal;
    numWithoutNeighborTotal = nullptr;

}

Define_Module(IoToriiOperation);

IoToriiOperation::IoToriiOperation() :
    headerLength(0),
    headerLengthPANID(0),
    broadcastType(0),
    upperLayerInGateId(-1),
    upperLayerOutGateId(-1),
    lowerLayerInGateId(-1),
    lowerLayerOutGateId(-1),
    corePrefix(-1),
    isCoreSwitch(false),
    startCoreEvent(nullptr),
    coreInterval(0),
    coreStartTime(0),
    numReceivedLowerPacket(0),
    numReceivedUpperPacket(0),
    numDiscardedFrames(0),
    numRoutedUnicastFrames(0),
    numRoutedBroadcastFrames(0),
    numDiscardedUnicastFrames(0),
    numDiscardedBroadcastFrames(0),
    numHelloRcvd(0),
    numHelloSent(0),
    numNeighbors(0),
    numDiscardedNoHLMAC(0),
    hlmacLenIsLow(0),
    hlmacWidthIsLow(0),
    numHLMACRcvd(0),
    numHLMACAssigned(0),
    numHLMACLoopable(0),
    numHLMACSent(0),
    maxNeighbors(0),
    maxHLMACs(-1),
    hlmacTable(nullptr),
    HelloTimer(nullptr),
    helloStartTime(0),
    helloInterval(0),
    isOperational(false),
    //host(nullptr),
    myMACAddress(MACAddress::UNSPECIFIED_ADDRESS),
    numHosts(0)
{
}

void IoToriiOperation::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        //host = getContainingNode(this);
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");

        numHosts = getParentModule()->getParentModule()->getParentModule()->getParentModule()->par( "numHosts" );

        headerLength = par("headerLength");
        headerLengthPANID = par("headerLengthPANID");

        broadcastType = par("broadcastType");

        helloStartTime = par("helloStartTime");
        helloInterval = par("helloInterval");

        maxHLMACs = par("maxHLMACs");

        maxNeighbors = pow(2, sizeof(unsigned int) * 8) - 1;  //Type of address width is Unsigned int in this simulation

        //jitterPar = &par("jitter");


        //WATCH(jitterPar->doubleValue());
        WATCH(maxHLMACs);
        WATCH(numHosts);
        WATCH(headerLength);
        WATCH(headerLengthPANID);
        WATCH(broadcastType);
        WATCH(helloStartTime);
        WATCH(helloInterval);
        WATCH(maxNeighbors);
        WATCH_VECTOR(neighborList);

        WATCH(hlmacLenIsLow);
        WATCH(hlmacWidthIsLow);
        WATCH(numHelloRcvd);
        WATCH(numHelloSent);
        WATCH(numNeighbors); //number of neighbors discovered by Hello message
        WATCH(numHLMACRcvd);
        WATCH(numHLMACAssigned);
        WATCH(numHLMACLoopable);
        WATCH(numHLMACSent);
        WATCH(numDiscardedNoHLMAC);

        WATCH(numReceivedUpperPacket);
        WATCH(numReceivedLowerPacket);
        WATCH(numDiscardedFrames);
        WATCH(numRoutedUnicastFrames);
        WATCH(numRoutedBroadcastFrames);
        WATCH(numDiscardedUnicastFrames);
        WATCH(numDiscardedBroadcastFrames);

        if (!numHelloSentTotal)
            globalStatisticsMemoryAllocation(numHosts);
        WATCH(*numHelloSentTotal);
        WATCH(*numHLMACSentTotal);
        WATCH(*numNeighborsTotal);
        WATCH(*numAllowedNeighborsTotal);
        WATCH(*numHLMACAssignedTotal);
        WATCH(*joiningTimeTotal);
        WATCH(*firstGenerationTime);
        WATCH(*lastReceivedTime);



    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        HelloTimer = new cMessage("HelloTimer");
        scheduleAt(helloStartTime, HelloTimer); //Next Hello broadcasting

        hlmacTable = check_and_cast<IHLMACAddressTable *>(getModuleByPath(par("hlmacTablePath")));
        myMACAddress = check_and_cast<SimpleIdealWirelessMAC *>(getParentModule()->getSubmodule("simpleidealwirelessMAC"))->getMACAddress();
        corePrefix = par("corePrefix");
        if (isCoreSwitch = par("isCoreSwitch"))
        {
            EV<< "This switch is a core switch and its prefix is  "<< corePrefix << "\n";
            startCoreEvent = new cMessage("startCoreEvent");
            coreStartTime = par("coreStartTime");
            coreInterval = par("coreInterval");
            scheduleAt(simTime() + coreStartTime, startCoreEvent);
            *firstGenerationTime = coreStartTime;

            WATCH(coreStartTime);
            WATCH(corePrefix);

        }
/*
       //Testing HLMACAddress and eGA3 frame data structure

        EV << "unspecified   " << HLMACAddress::UNSPECIFIED_ADDRESS << endl;
        EV << "broadcast     " << HLMACAddress::BROADCAST_ADDRESS << endl;
        int array[8] = {0,0,0,0,0,0,0,0};

        EV << "size of " << sizeof array << "   " << sizeof array[0] << endl;
        HLMACAddress a;
        EV << "hlmac test is  " << a << endl;
        a.setCore(1);
        EV << "1              " << a << endl;
        a.removeLastId();
        EV << "empty vector!  " << a << "size vector " << a.getHLMACLength() << endl;
        a.addNewId(2);
        EV << "2              " << a << endl;
        a.addNewId(250);
        a.addNewId(300);
        a.addNewId(400);
        a.addNewId(50);
        EV << "250 300 400 50 " << a << endl;
        a.removeLastId();
        EV << "delete last id " << a << endl;
        eGA3Frame b;
        EV << "eGA3 0         " << b << endl;
        eGA3Frame c(2,a);
        EV << "eGA3 hlmac     " << c << endl;
*/

    }
}

void IoToriiOperation::sendAndScheduleHello()
{
    EV << "->IoToriiOperation::sendAndScheduleHello()" << endl;

    //scheduling next Hello packet
    helloStartTime+= helloInterval;
    //scheduleAt(helloStartTime, HelloTimer); //Next Hello broadcasting

    MACFrameIoTorii *macPkt = new MACFrameIoTorii("Hello!");
    AddressStruct dst;
    dst.MAC = MACAddress::BROADCAST_ADDRESS;
    macPkt->setDestAddr(dst);
    macPkt->setBitLength(headerLength);
    EV << "Hello message from this node is broadcasted to all node in the range. " << endl;
    //double delay = jitterPar->doubleValue();
    //EV << "value of random jitter is " << delay << endl;
    send(macPkt, lowerLayerOutGateId);  //sendDown(macPkt, delay); //send(macPkt, "lowerLayerOut");
    numHelloSent++;
    (*numHelloSentTotal)++;

    EV << "<-IoToriiOperation::sendAndScheduleHello()" << endl;
}

void IoToriiOperation::startCore(int core)
{
    EV << "->IoToriiOperation::startCore()" << endl;

    //scheduling next Core event
    coreStartTime = coreStartTime + coreInterval;
    //scheduleAt(coreStartTime, startCoreEvent);

    //preparing SetHLMAC frame
    HLMACAddress coreAddress;             // create HLMAC
    coreAddress.setCore((unsigned int)core);  //insert core prefix in it
    EV << "Core address " << coreAddress << " is assigned to this node." << endl;
    saveHLMAC(coreAddress);     //assigns coreAddress to itself
    eGA3FrameType type = SetHLMAC;  //unsigned int type = 1;               // if assume that type of SetHLMAC frame is 1
    eGA3Frame eGA3(type,coreAddress);

    //preparing CSMA frame
    MACFrameIoTorii *SetHLMACFrame = new MACFrameIoTorii ("SetHLMAC");
    AddressStruct src;
    src.eGA3 = eGA3;
    SetHLMACFrame->setSrcAddr(src);
    SetHLMACFrame->setBitLength(headerLength);  //size of IEEE802.15.4 frame format without PANIDs
    EV << "SetHLMAC frame " << SetHLMACFrame->getSrcAddr().eGA3 << " is disseminated to the neighbors." << endl;
    sendToNeighbors(SetHLMACFrame);

    EV << "<-IoToriiOperation::startCore()" << endl;
}

void IoToriiOperation::sendToNeighbors(MACFrameIoTorii *frame)
{
    EV << "->IoToriiOperation::sendToNeighbors()" << endl;

    EV << "This node has " << numNeighbors << " neighbors, " << "maximum number of allowed neighbors is " << maxNeighbors << endl;
    if (numNeighbors > maxNeighbors){
        hlmacWidthIsLow++;
        EV << "hlmacWidthIsLow is " << hlmacWidthIsLow << endl;
    }


    for (int i = 1; i <= numNeighbors && i <= maxNeighbors; i++) {  //Second condition is related on the width of HLMACAddress, 3 bits allowed 3 neighbor, 00 is reserved for NOTSPECIFIED address
        eGA3Frame eGA3 = frame->getSrcAddr().eGA3;
        HLMACAddress hlmac = eGA3.getHLMACAddress();

        hlmac.addNewId(i);
        eGA3.setHLMACAddress(hlmac);

        MACFrameIoTorii *dupFrame = frame->dup();
        AddressStruct src, dst;
        src.eGA3 = eGA3;
        dupFrame->setSrcAddr(src);
        dst.MAC = neighborList[i-1];
        dupFrame->setDestAddr(dst);
        emit(LayeredProtocolBase::packetSentToLowerSignal, frame);
        EV << "SetHLMAC frame " << dupFrame->getSrcAddr().eGA3 << " is sent to the neighbor with suffix # (" << i << ") and dst MAC address (" << dupFrame->getDestAddr().MAC << ")" << endl;
        //double delay = jitterPar->doubleValue();
        //EV << "value of random jitter is " << delay << endl;
        send(dupFrame, lowerLayerOutGateId);  //sendDown(dupFrame, delay); // send(dupFrame, "lowerLayerOut");
        numHLMACSent++;
        (*numHLMACSentTotal)++;

    }

    delete frame;
    EV << "<-IoToriiOperation::sendToNeighbors()" << endl;
}

void IoToriiOperation::receiveSetHLMACMessage(MACFrameIoTorii *frame)
{
    EV << "->IoToriiOperation::receiveSetHLMACMessage()" << endl;

    numHLMACRcvd++;
    if ((*lastReceivedTime) < simTime())
        *lastReceivedTime = simTime();
    *joiningTimeTotal = *lastReceivedTime - *firstGenerationTime;


    HLMACAddress hlmac = frame->getSrcAddr().eGA3.getHLMACAddress();

    if (!hasLoop(hlmac)){
        bool isSaved = saveHLMAC(hlmac);
        if (isSaved){
            EV << "HLMAC address " << hlmac << " has assigned to this node." << endl;
            EV << "Frame " << frame->getName() << " (src address: " << frame->getSrcAddr().eGA3 << ", dst address" << frame->getDestAddr().MAC<< " ) is resent to neighbors by this node after updating dst mac address field." << endl;
            sendToNeighbors(frame);
        }
        else{
            EV << "Frame " << frame->getName() << " (received from src mac address: " << frame->getSrcAddr().eGA3 << ", dst address" << frame->getDestAddr().MAC << " ) frame is dropped! table is full!" << endl;
            delete frame;
        }
    }else{
        EV << "Because of loop creation, HLMAC address " << hlmac << " is not assigned to this node." << endl;
        EV << "Frame " << frame->getName() << " (received from src mac address: " << frame->getSrcAddr().eGA3 << ", dst address" << frame->getDestAddr().MAC << " ) frame is dropped! Because of loop." << endl;
        delete frame;
    }

    EV << "<-IoToriiOperation::receiveSetHLMACMessage()" << endl;
}

bool IoToriiOperation::hasLoop(HLMACAddress hlmac)
{
    EV << "->IoToriiOperation::hasLoop()" << endl;

    HLMACAddress longestPrefix = hlmacTable->getlongestMatchedPrefix(hlmac);
    if (longestPrefix == HLMACAddress::UNSPECIFIED_ADDRESS){
        EV << "HLMAC adress " << hlmac << " does not create a loop in this node. Longest Matched Prefix is UNSPECIFIED : " << longestPrefix << endl;
        return false;
    }
    else{
        numHLMACLoopable++;
        EV << "HLMAC adress " << hlmac << " creates a loop in this node. Longest Matched Prefix is " << longestPrefix << endl;
        return true;
    }

    EV << "<-IoToriiOperation::hasLoop()" << endl;
}

bool IoToriiOperation::saveHLMAC(HLMACAddress hlmac)
{
    EV << "->IoToriiOperation::saveHLMAC()" << endl;

    if ((maxHLMACs == -1) || ((maxHLMACs != -1) && (numHLMACAssigned < maxHLMACs))){
        hlmacTable->updateTableWithAddress(-1, hlmac);
        numHLMACAssigned++;
        (*numHLMACAssignedTotal)++;
        int numHLMACAssignedTemp = *numHLMACAssignedTotal;
        EV << "HLMAC adress " << hlmac << " was saved to this node, number of assigned HLMAC is " << numHLMACAssigned << ", number of total assigned HLMAC is " << numHLMACAssignedTemp << endl;
        return true;
    }
    else{
        EV << "HLMAC address is discarded! HLMAC table is full! Maximum number of allowed HLMAC is  " << maxHLMACs << "  number of assigned hlmac is " << numHLMACAssigned << ", so HLMAC table is full. " << endl;
        return false;
    }

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

    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else if (isUpperMessage(msg)) {
        if (msg->isPacket()){
            //emit(packetReceivedFromUpperSignal, msg);
            handleUpperPacket(PK(msg));
        }
/*        else
            handleUpperCommand(msg); */
    }
    else if (isLowerMessage(msg)) {
        if (msg->isPacket()){
            //emit(packetReceivedFromLowerSignal, msg);
            handleLowerPacket(PK(msg));
        }
/*        else
            handleLowerCommand(msg);*/
    }
    else
        throw cRuntimeError("Message '%s' received on unexpected gate '%s'", msg->getName(), msg->getArrivalGate()->getFullName());

    EV << "<-IoToriiOperation::handleMessage()" << endl;
}

void IoToriiOperation::handleSelfMessage(cMessage *msg)
{
    EV << "<-IoToriiOperation::handleSelfMessage()" << endl;
    if (msg == HelloTimer) {
        sendAndScheduleHello();
        return;
    }
    else if (msg == startCoreEvent) {
        startCore(corePrefix);
        return;
    }
    else
        EV << "IoToriiOperation Error: unknown SelfMessage:" << msg << endl;
    EV << "<-IoToriiOperation::handleSelfMessage()" << endl;
}

void IoToriiOperation::handleUpperPacket(cPacket *msg)
{
    EV << "->IoToriiOperation::handleUpperPacket()" << endl;

    throw cRuntimeError("->IoToriiOperation::handleUpperPacket(): The simulation doesn't process upper layer packets.");

    EV << "<-IoToriiOperation::handleUpperPacket()" << endl;
}

void IoToriiOperation::handleLowerPacket(cPacket *msg)
{
    EV << "->IoToriiOperation::handleLowerPacket()" << endl;

    numReceivedLowerPacket++;

    if ((strcmp(msg->getName(),"Hello!")==0))
    {
        MACFrameIoTorii *frame = check_and_cast<MACFrameIoTorii *>(msg);
        numHelloRcvd++;
        //to check duplicate hello
        bool isDuplicate = false;
        for(int i=0; i<numNeighbors; i++)
            if (frame->getSrcAddr().MAC == neighborList[i])
                isDuplicate = true;

        if (!isDuplicate){
            neighborList.push_back(frame->getSrcAddr().MAC);
            EV << "Hello message is received from : " << frame->getSrcAddr().MAC << "; Number of neighbors is " << neighborList.size() <<endl;
            numNeighbors++;
            (*numNeighborsTotal)++;
            if (numNeighbors <= maxNeighbors)
                (*numAllowedNeighborsTotal)++;
            return;
        }
        else {
            EV << "Hello message is received from : " << frame->getSrcAddr().MAC << "is duplicate." <<endl;
            return;
        }
    } // END Hello
    else if ((strcmp(msg->getName(),"SetHLMAC")==0)) {
        MACFrameIoTorii *frame = check_and_cast<MACFrameIoTorii *>(msg);
        receiveSetHLMACMessage(frame);
        return;
    } //END SetHLMAC
    EV << "<-IoToriiOperation::handleLowerPacket()" << endl;
}


/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject *IoToriiOperation::setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

void IoToriiOperation::sendUp(cMessage *message)
{
//    if (message->isPacket())
//        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

void IoToriiOperation::sendDown(cMessage *message, double delay)
{
//    if (message->isPacket())
//        emit(packetSentToLowerSignal, message);
    sendDelayed(message, delay, lowerLayerOutGateId);
}

bool IoToriiOperation::isUpperMessage(cMessage *message)
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool IoToriiOperation::isLowerMessage(cMessage *message)
{
    return message->getArrivalGateId() == lowerLayerInGateId;
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

    finishCounter++;

    if (numHLMACAssigned == 0){
        (*numNotJoinedTotal)++;
        int i = 0;
/*        while ((i < numHosts) && (listNotJoinedTotal[i++] != MACAddress::UNSPECIFIED_ADDRESS));
        if (--i < numHosts)
            listNotJoinedTotal[i] = myMACAddress;
        else
            throw cRuntimeError("IoToriiOperation::finish() : Array len is low!");
 */   }

    if (numNeighbors == 0){
        (*numWithoutNeighborTotal)++;
    }



    recordScalar("hlmacLenIsLow", hlmacLenIsLow);
    recordScalar("hlmacWidthIsLow", hlmacWidthIsLow);
    recordScalar("Received Hello", numHelloRcvd);
    recordScalar("numHelloSent", numHelloSent);
    recordScalar("numNeighbors", numNeighbors); //number of neighbors discovered by Hello message
    recordScalar("numHLMACRcvd", numHLMACRcvd);
    recordScalar("numHLMACAssigned", numHLMACAssigned);
    recordScalar("numHLMACLoopable", numHLMACLoopable);
    recordScalar("numHLMACSent", numHLMACSent);
    recordScalar("numDiscardedNoHLMAC", numDiscardedNoHLMAC);

    recordScalar("Received Upper Packets", numReceivedUpperPacket);
    recordScalar("Received Lower frames", numReceivedLowerPacket);
    recordScalar("discarded frames", numDiscardedFrames);
    recordScalar("numRoutedUnicastFrames", numRoutedUnicastFrames);
    recordScalar("numRoutedBroadcastFrames", numRoutedBroadcastFrames);
    recordScalar("numDiscardedUnicastFrames", numDiscardedUnicastFrames);
    recordScalar("numDiscardedBroadcastFrames", numDiscardedBroadcastFrames);


    //if(numHelloSentTotal){  //when first finish() is run
    if(finishCounter == numHosts){  //when last finish() is run. this is only because of numNotJoinedTotal
        statisticsCollector = fopen("./01_IoToriiGlobalStats.txt","a+t");
        fprintf(statisticsCollector," _____________________________________________________________________________________\n");
        fprintf(statisticsCollector,"|Results of new run                                                |\n");
        fprintf(statisticsCollector,"|------------------------------------------------------------------|------------------\n");
        fprintf(statisticsCollector,"| Total joining time (convergence time)                            | %f\n", joiningTimeTotal->dbl());
        fprintf(statisticsCollector,"| Number of total table entries (HLMAC table + Neighbor table)     | %d\n", *numAllowedNeighborsTotal + *numHLMACAssignedTotal);
        fprintf(statisticsCollector,"| Number of All total table entries (HLMAC table + Neighbor table) | %d\n", *numNeighborsTotal + *numHLMACAssignedTotal);
        fprintf(statisticsCollector,"| Number of total assigned HLMAC (or number of HLMAC table entries)| %d\n", *numHLMACAssignedTotal);
        fprintf(statisticsCollector,"| Number of tot limited neighbors(number of neighbor table entries)| %d\n", *numAllowedNeighborsTotal);
        fprintf(statisticsCollector,"| Number of ALL total neighbors (All neighbor table entries)       | %d\n", *numNeighborsTotal);
        fprintf(statisticsCollector,"| Number of total messages (Hello + HLMAC)                         | %d\n", *numHLMACSentTotal + *numHelloSentTotal);
        fprintf(statisticsCollector,"| Number of total sent HLMAC                                       | %d\n", *numHLMACSentTotal);
        fprintf(statisticsCollector,"| Number of total sent Hello                                       | %d\n", *numHelloSentTotal);
        fprintf(statisticsCollector,"| Number of total disjoint to DODAG                                | %d\n", *numNotJoinedTotal);
        fprintf(statisticsCollector,"| Number of total without any neighbor                             | %d\n", *numWithoutNeighborTotal);
        fprintf(statisticsCollector,"|__________________________________________________________________|__________________\n");
        fclose(statisticsCollector);

        statisticsCollector = fopen("./02_joiningTimeFinal.txt","a+t");
        fprintf(statisticsCollector,"%f\n", joiningTimeTotal->dbl());
        fclose(statisticsCollector);

        statisticsCollector = fopen("./03_LimitedEntriesTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numAllowedNeighborsTotal + *numHLMACAssignedTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./04_AllEntriesTotal_2.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numNeighborsTotal + *numHLMACAssignedTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./05_HLMACAssignedTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numHLMACAssignedTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./06_NeighborsTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numAllowedNeighborsTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./07_NeighborsTotal_2.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numNeighborsTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./08_MessagesTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numHLMACSentTotal + *numHelloSentTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./09_HLMACSentTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numHLMACAssignedTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./10_HelloSentTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numHelloSentTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./11_NumNotJoinedTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numNotJoinedTotal);
        fclose(statisticsCollector);

        statisticsCollector = fopen("./12_NumWithoutNeighborTotal.txt","a+t");
        fprintf(statisticsCollector,"%d\n", *numWithoutNeighborTotal);
        fclose(statisticsCollector);

        globalStatisticsMemoryDeallocation();
    }



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

IoToriiOperation::~IoToriiOperation()
{
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
}

} // namespace iotorii

