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

#include "src/linklayer/IoTorii/HLMACAddressTable.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"

#include "src/statisticcollector/StatisticCollector.h"


namespace iotorii {
using namespace inet;

Define_Module(IoToriiOperation);

IoToriiOperation::IoToriiOperation() :
    statisticCollector(nullptr),
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
    numAllowedNeighbors(0),
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
    myMACAddress(MACAddress::UNSPECIFIED_ADDRESS)
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

        headerLength = par("headerLength");
        headerLengthPANID = par("headerLengthPANID");

        broadcastType = par("broadcastType");

        helloStartTime = par("helloStartTime");
        helloInterval = par("helloInterval");

        maxHLMACs = par("maxHLMACs");

        maxNeighbors = pow(2, sizeof(unsigned int) * 8) - 1;  //Type of address width is Unsigned int in this simulation

        jitterPar = &par("jitter");

        //WATCH(jitterPar->doubleValue());
        WATCH(maxHLMACs);
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
        WATCH(numAllowedNeighbors);
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
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        hlmacTable = check_and_cast<IHLMACAddressTable *>(getModuleByPath(par("hlmacTablePath")));
        myMACAddress = check_and_cast<SimpleIdealWirelessMAC *>(getParentModule()->getSubmodule("simpleidealwirelessMAC"))->getMACAddress();
        corePrefix = par("corePrefix");

    }else if (stage == NUM_INIT_STAGES){
        statisticCollector = check_and_cast<StatisticCollector *>(getSimulation()->getSystemModule()->getSubmodule("statisticCollector"));

        HelloTimer = new cMessage("HelloTimer");
        scheduleAt(helloStartTime, HelloTimer); //Next Hello broadcasting

        if (isCoreSwitch = par("isCoreSwitch"))
        {
            EV<< "This switch is a core switch and its prefix is  "<< corePrefix << "\n";
            startCoreEvent = new cMessage("startCoreEvent");
            coreStartTime = par("coreStartTime");
            coreInterval = par("coreInterval");
            simtime_t startTime = simTime() + coreStartTime;
            scheduleAt(startTime, startCoreEvent);
            statisticCollector->startStatistics(myMACAddress, startTime);

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

    MACFrameBase *macPkt = new MACFrameBase("Hello!");
    macPkt->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    macPkt->setBitLength(headerLength);
    EV << "Hello message from this node is broadcasted to all node in the range. " << endl;
    double delay = jitterPar->doubleValue();
    EV << "value of random jitter is " << delay << endl;
    sendDown(macPkt, delay); //send(macPkt, lowerLayerOutGateId);  //send(macPkt, "lowerLayerOut");
    numHelloSent++;

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

    sendToNeighbors(coreAddress);

    EV << "<-IoToriiOperation::startCore()" << endl;
}

void IoToriiOperation::sendToNeighbors(HLMACAddress prefix)
{
    EV << "->IoToriiOperation::sendToNeighbors()" << endl;

    unsigned int numSuffixes = 0;

    EV << "This node has " << numNeighbors << " neighbors, " << "maximum number of allowed neighbors is " << maxNeighbors << endl;
    if (numNeighbors > maxNeighbors){
        hlmacWidthIsLow++;
        numSuffixes = maxNeighbors;
        EV << "hlmacWidthIsLow is " << hlmacWidthIsLow << endl;
    }else
        numSuffixes = numNeighbors;

    //preparing frame payload
    SetHLMACFrame *SetHLMACFramePayload = new SetHLMACFrame ("SetHLMAC");
    SetHLMACFramePayload->setPrefix(prefix);
    SetHLMACFramePayload->setTargetOptionsArraySize(numSuffixes);
    //SetHLMACFrame->setBitLength(numSuffixes * 8 + 8);  //numSuffixes * sizeof suffix + size of prefix
    TargetOption targetOption;

    for (int i = 0; i < numSuffixes; i++) {  //Second condition is related on the width of HLMACAddress, 3 bits allowed 3 neighbor, 00 is reserved for NOTSPECIFIED address
        targetOption.MAC = neighborList.at(i);
        targetOption.suffix = i+1;
        EV << "Prefix " << SetHLMACFramePayload->getPrefix() << " and Suffix " << targetOption.suffix << " for neighbor with MAC address " << targetOption.MAC << " is added to the target option"<< endl;
        SetHLMACFramePayload->setTargetOptions(i, targetOption);
    }

    //preparing MAC frame
    MACFrameBase *SetHLMACFrame = new MACFrameBase ("SetHLMAC");
    SetHLMACFrame->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    SetHLMACFrame->encapsulate(SetHLMACFramePayload);

    double delay = jitterPar->doubleValue();
    EV << "value of random jitter is " << delay << endl;
    EV << "SetHLMAC frame " << SetHLMACFrame->getName() << " is sent to the MAC layer to send to the neighbors" << endl;
    emit(LayeredProtocolBase::packetSentToLowerSignal, SetHLMACFrame);
    sendDown(SetHLMACFrame, delay);  //send(dupFrame, lowerLayerOutGateId);  // send(dupFrame, "lowerLayerOut");
    numHLMACSent++;

    EV << "<-IoToriiOperation::sendToNeighbors()" << endl;
}

HLMACAddress IoToriiOperation::extractMyAddress(cPacket *frame){
    EV << "->IoToriiOperation::extractMyAddress()" << endl;

    SetHLMACFrame *SetHLMACFramePayload = check_and_cast<SetHLMACFrame *>(frame->decapsulate());
    HLMACAddress hlmac;

    for(unsigned int i=0; i<SetHLMACFramePayload->getTargetOptionsArraySize(); i++){
        if (SetHLMACFramePayload->getTargetOptions(i).MAC == myMACAddress){
            hlmac = SetHLMACFramePayload->getPrefix();
            hlmac.addNewId(SetHLMACFramePayload->getTargetOptions(i).suffix);
            delete frame;
            return hlmac;
        }
    }

    delete frame;
    return HLMACAddress::UNSPECIFIED_ADDRESS;

    EV << "<-IoToriiOperation::extractMyAddress()" << endl;
}
void IoToriiOperation::receiveSetHLMACMessage(HLMACAddress hlmac, simtime_t arrivalTime)
{
    if (!hasLoop(hlmac)){
        bool isSaved = saveHLMAC(hlmac);
        if (isSaved){
            EV << "HLMAC address " << hlmac << " has assigned to this node." << endl;
            EV << "Prefix " << hlmac << " is sent to neighbors by this node after creating broadcast SetHLMAC frame." << endl;
            statisticCollector->nodeJoined(myMACAddress, arrivalTime);
            sendToNeighbors(hlmac);
        }
        else{
            EV << "HLMAC address " << hlmac << " is not assigned to this node! table is full!" << endl;
        }
    }else{
        EV << "Because of loop creation, HLMAC address " << hlmac << " is not assigned to this node." << endl;
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
        EV << "HLMAC adress " << hlmac << " was saved to this node, number of assigned HLMAC is " << numHLMACAssigned << "." << endl;
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
        MACFrameBase *frame = check_and_cast<MACFrameBase *>(msg);
        numHelloRcvd++;
        //to check duplicate hello
        bool isDuplicate = false;
        for(int i=0; i<numNeighbors; i++)
            if (frame->getSrcAddr() == neighborList.at(i))
                isDuplicate = true;

        if (!isDuplicate){
            neighborList.push_back(frame->getSrcAddr());
            EV << "Hello message is received from : " << frame->getSrcAddr() << "; Number of neighbors is " << neighborList.size() <<endl;
            numNeighbors++;
            if (numNeighbors <= maxNeighbors)
                numAllowedNeighbors++;
            return;
        }
        else {
            EV << "Hello message is received from : " << frame->getSrcAddr() << "is duplicate." <<endl;
            return;
        }
    } // END Hello
    else if ((strcmp(msg->getName(),"SetHLMAC")==0)) {
        numHLMACRcvd++;
        MACFrameBase *frame = check_and_cast<MACFrameBase *>(msg);
        HLMACAddress hlmac = extractMyAddress(frame);
        if (hlmac != HLMACAddress::UNSPECIFIED_ADDRESS){
            receiveSetHLMACMessage(hlmac, frame->getArrivalTime());
        }
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

void IoToriiOperation::getMessageStatistics(long &numHelloSent, long &numHLMACSent)
{
    numHelloSent = this->numHelloSent;
    numHLMACSent = this->numHLMACSent;
}

void IoToriiOperation::getTableStatistics(long &numAllowedNeighbors, long &numNeighbors)
{
    numAllowedNeighbors = this->numAllowedNeighbors;
    numNeighbors = this->numNeighbors;
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


    if (startCoreEvent){
        cancelEvent(startCoreEvent);
        delete startCoreEvent;
        startCoreEvent = nullptr;
    }

    if (HelloTimer){
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

