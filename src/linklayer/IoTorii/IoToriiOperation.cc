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
#include "src/linklayer/csma/CSMAFramePANID_m.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"


namespace iotorii {
using namespace inet;


Define_Module(IoToriiOperation);

IoToriiOperation::IoToriiOperation() :
    headerLength(0),
    upperLayerInGateId(-1),
    upperLayerOutGateId(-1),
    lowerLayerInGateId(-1),
    lowerLayerOutGateId(-1),
    corePrefix(-1),
    numReceivedLowerPacket(0),
    numReceivedUpperPacket(0),
    numDiscardedFrames(0),
    numHelloRcvd(0),
    numNeighbors(0),
    maxNeighbors(3),
    isCoreSwitch(false),
    hlmacTable(nullptr),
    HelloTimer(nullptr),
    startCoreEvent(nullptr),
    isOperational(false)

{
}

void IoToriiOperation::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");

        headerLength = par("headerLength");
        helloInterval = par("helloInterval");

        WATCH(headerLength);
        WATCH(numReceivedUpperPacket);
        WATCH(numDiscardedFrames);
        WATCH(numHelloRcvd);
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
    sendDown(macPkt); //send(macPkt, lowerLayerOutGateId); //send(macPkt, "lowerLayerOut");

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
        EV << "SetHLMAC frame " << eGA3 << " is sent to the neighbor with suffix # (" << i << ") and dst MAC address (" << dupFrame->getDestAddr() << ")" << endl;
        sendDown(dupFrame); //send(dupFrame, lowerLayerOutGateId); // send(dupFrame, "lowerLayerOut");

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
    eGA3FrameType type = SetHLMAC;  //unsigned char type = 1;               // if assume that type of SetHLMAC frame is 1
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
        EV << "HLMAC address " << hlmac << " has assigned to this node." << endl;
        EV << "Frame " << frame->getName() << " (received from dst mac address: " << frame->getDestAddr() << " )" << " is resent to neighbors by this node after updating dst mac address field." << endl;
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
        EV << "HLMAC adress " << hlmac << " does not create a loop in this node. Longest Matched Prefix is UNSPECIFIED : " << longestPrefix << endl;
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
    EV << "HLMAC adress " << hlmac << " was saved to this node." << endl;

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
        EV << "CSMAIoTorii Error: unknown SelfMessage:" << msg << endl;
    EV << "<-IoToriiOperation::handleSelfMessage()" << endl;
}

void IoToriiOperation::handleUpperPacket(cPacket *msg)
{
    EV << "->IoToriiOperation::handleUpperPacket()" << endl;

    //MacPkt *macPkt = encapsMsg(msg);

    IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
    MACAddress macdest = cInfo->getDestinationAddress();
    eGA3Frame eGA3dst(macdest);
    HLMACAddress dest = eGA3dst.getHLMACAddress();
    unsigned char type = eGA3dst.geteGA3FrameType();
    EV_DETAIL << "A message has received from upper layer, name is " << msg->getName() << ", CInfo removed, dst HLMAC addr=" << dest << ", dst HLMAC type=" << (unsigned short int) type << endl;
    CSMAFramePANID *frame = new CSMAFramePANID(msg->getName());
    frame->setBitLength(headerLength);
    EV << "frame header length is set to " <<  frame->getBitLength() << " (bits)." << endl;
    frame->setDestAddr(macdest);
    delete cInfo;
    MetricType metric = HopCount;
    HLMACAddress bestSrcAddr = hlmacTable -> getSrcAddress(dest, metric);
    eGA3Frame eGA3src;
    eGA3src.setHLMACAddress(bestSrcAddr);
    eGA3src.seteGA3FrameType(type);
    MACAddress macsrc(eGA3src.getInt());
    frame->setSrcAddr(macsrc);

    if (dest != HLMACAddress::BROADCAST_ADDRESS){  //unicast transmission
        unsigned int lenCommonAncestor = 0;
        unsigned int counter = 0;
        HLMACAddress commonAncestor;
        if ((commonAncestor = bestSrcAddr.getLongestCommonPrefix(dest)) != HLMACAddress::UNSPECIFIED_ADDRESS){
            lenCommonAncestor = (unsigned int)(commonAncestor.getHLMACHier()) + 1;
            counter = ((unsigned int)bestSrcAddr.getHLMACHier() + 1) + ((unsigned int)dest.getHLMACHier() + 1) - 2 * lenCommonAncestor;
            EV << "Best selected src HLMAC address is " << bestSrcAddr << "src length is" << bestSrcAddr.getHLMACHier() + 1 << "dst HLMAC address is " << dest << "dst length is " << dest.getHLMACHier() + 1 << "longest common ancestor HLMAC address is " << commonAncestor << "longest common ancestor length is " << lenCommonAncestor << "counter is " << counter << endl;
            frame->setSrcPANID(MACAddress(counter)); //src PAN ID is reused for saving counter in unicast transmission
            EV << "Unicast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (counter) is " << counter << endl;
        }
        else
            throw cRuntimeError("src and dst has no common ancestor!");
    }
    else{   //broadcast transmission
        frame->setSrcPANID(macsrc); //src PAN ID is reused for saving transmitter in broadcast transmission
        EV << "Broadcast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (Transmitter address) is " << eGA3src << endl;
    }

    //RadioAccNoise3PhyControlInfo *pco = new RadioAccNoise3PhyControlInfo(bitrate);
    //macPkt->setControlInfo(pco);
    assert(static_cast<cPacket *>(msg));
    frame->encapsulate(static_cast<cPacket *>(msg));
    EV_DETAIL << "pkt encapsulated, length: " <<  frame->getBitLength() << "\n";

    sendDown(frame);


    EV << "<-IoToriiOperation::handleUpperPacket()" << endl;
}

void IoToriiOperation::handleLowerPacket(cPacket *msg)
{
    EV << "->IoToriiOperation::handleLowerPacket()" << endl;
    CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);

    numReceivedLowerPacket++;

    if ((strcmp(frame->getName(),"Hello!")==0))
    {
        numHelloRcvd++;
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
        else {
            EV << "Hello message is received from : " << frame->getSrcAddr() << "is duplicate." <<endl;
            return;
        }
    } // END Hello
    else if ((strcmp(frame->getName(),"SetHLMAC")==0)) {
        receiveSetHLMACMessage(frame);
        return;
    } //END SetHLMAC
    else  // Data frame
    {
        const HLMACAddress& src = HLMACAddress(frame->getSrcAddr().getInt());
        const HLMACAddress& dst = HLMACAddress(frame->getDestAddr().getInt());
        //const int counter = frame->

        EV << "Received frame name= " << frame->getName() << " srcHLMAC=" << src << " dstHLMAC=" << dst << endl;
        if (hlmacTable->isMyAddress(dst)){ //if (myAddress != HLMACAddress::UNSPECIFIED_ADDRESS){     //Data frame is mine. Send its payload to upper layer
            EV << "Data frame is mine. its payload is sent to upper layer. " << endl;
            sendUp(decapsMsg(frame));
            //nbRxFrames++;
            delete msg;
        }
        else if (dst == HLMACAddress::BROADCAST_ADDRESS){     //Data frame is broadcast, a copye of it is mine. Send the copy to upper layer
            EV << "Data frame is a broadcast frame. its payload is sent to upper layer and  a copy of it is sent to broadcast proccess." << endl;
            sendUp(decapsMsg(frame));
            //nbRxFrames++;
            broadcastProccessUpwardTraffic(frame);
            //broadcastProccessAllwardTraffic(frame);
        }
        else {     //Data frame is not mine. Send it to routing proccess
            EV << "Data frame is not mine. it need to routing, so it is sent to routing proccess." << endl;
            //nbRxFrames++;
            routingProccess(frame);
        }
    }
    EV << "<-IoToriiOperation::handleLowerPacket()" << endl;
}

void IoToriiOperation::routingProccess(CSMAFrame *frame)
{
    EV << "->IoToriiOperation::routingProccess()" << endl;
    CSMAFramePANID *framePANID = check_and_cast<CSMAFramePANID *>(frame);
    HLMACAddress commonAncestor;
    const HLMACAddress& src = HLMACAddress(framePANID->getSrcAddr().getInt());
    const HLMACAddress& dst = HLMACAddress(frame->getDestAddr().getInt());






    EV << "<-IoToriiOperation::routingProccess()" << endl;
}

void IoToriiOperation::broadcastProccessUpwardTraffic(CSMAFrame *macPkt)
{
    EV << "->IoToriiOperation::broadcastProccessUpwardTraffic()" << endl;

    EV << "<-IoToriiOperation::broadcastProccessUpwardTraffic()" << endl;
}

void IoToriiOperation::broadcastProccessAllwardTraffic(CSMAFrame *macPkt)
{
    EV << "->IoToriiOperation::broadcastProccessAllwardTraffic()" << endl;

    EV << "<-IoToriiOperation::broadcastProccessAllwardTraffic()" << endl;
}

cPacket *IoToriiOperation::decapsMsg(CSMAFrame *macPkt)
{
    cPacket *msg = macPkt->decapsulate();
    //convert HLMAC broadcast address to the MAC broadcast address because network layer doesn't recognize HLMAC broadcast address
    if (HLMACAddress(macPkt->getSrcAddr().getInt()) == HLMACAddress::BROADCAST_ADDRESS)
        setUpControlInfo(msg, MACAddress::BROADCAST_ADDRESS);
    else
        setUpControlInfo(msg, macPkt->getSrcAddr());
    return msg;
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

void IoToriiOperation::sendDown(cMessage *message)
{
//    if (message->isPacket())
//        emit(packetSentToLowerSignal, message);
    send(message, lowerLayerOutGateId);
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

    recordScalar("Received Upper Packets", numReceivedUpperPacket);
    recordScalar("Received Lower frames", numReceivedLowerPacket);
    recordScalar("discarded frames", numDiscardedFrames);
    recordScalar("Received Hello", numHelloRcvd);

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

