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
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"


namespace iotorii {
using namespace inet;


Define_Module(IoToriiOperation);

IoToriiOperation::IoToriiOperation() :
    headerLength(0),
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
    maxNeighbors(3),
    hlmacTable(nullptr),
    HelloTimer(nullptr),
    helloStartTime(0),
    helloInterval(0),
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
        broadcastType = par("broadcastType");

        helloStartTime = par("helloStartTime");
        helloInterval = par("helloInterval");

        WATCH(headerLength);
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


    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        HelloTimer = new cMessage("HelloTimer");
        scheduleAt(helloStartTime, HelloTimer); //Next Hello broadcasting

        hlmacTable = check_and_cast<IHLMACAddressTable *>(getModuleByPath(par("hlmacTablePath")));

        corePrefix = par("corePrefix");
        if (isCoreSwitch = par("isCoreSwitch"))
        {
            EV<< "This switch is a core switch and its prefix is  "<< corePrefix << "\n";
            startCoreEvent = new cMessage("startCoreEvent");
            coreStartTime = par("coreStartTime");
            coreInterval = par("coreInterval");
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
    helloStartTime+= helloInterval;
    //scheduleAt(helloStartTime, HelloTimer); //Next Hello broadcasting

    CSMAFrame *macPkt = new CSMAFrame("Hello!");
    macPkt->setDestAddr(MACAddress::BROADCAST_ADDRESS);
    EV << "Hello message from this node is broadcasted to all node in the range. " << endl;
    sendDown(macPkt); //send(macPkt, lowerLayerOutGateId); //send(macPkt, "lowerLayerOut");
    numHelloSent++;

    EV << "<-IoToriiOperation::sendAndScheduleHello()" << endl;
}

void IoToriiOperation::sendToNeighbors(CSMAFrame *frame)
{
    EV << "->IoToriiOperation::sendToNeighbors()" << endl;

    EV << "This node has " << numNeighbors << " neighbors, " << "maximum number of allowed neighbors is " << maxNeighbors << endl;
    if (numNeighbors > maxNeighbors){
        hlmacWidthIsLow++;
        EV << "hlmacWidthIsLow is " << hlmacWidthIsLow << endl;
    }

    eGA3Frame eGA3(frame->getSrcAddr());
    HLMACAddress hlmac = eGA3.getHLMACAddress();
    if (hlmac.getHLMACHier() >= hlmac.getHLMACLength() - 1){
        delete frame;
        hlmacLenIsLow++;
        EV << "HLMAC address is " << hlmac << ", Max allowed len is " << hlmac.getHLMACLength() << ", there is no enough space to continue. frame is deleted." << endl;
        return;
    }

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
        numHLMACSent++;

    }

    delete frame;
    EV << "<-IoToriiOperation::sendToNeighbors()" << endl;
}

void IoToriiOperation::startCore(int core)
{
    EV << "->IoToriiOperation::startCore()" << endl;

    //scheduling next Core event
    coreStartTime = coreStartTime + coreInterval;
    //scheduleAt(coreStartTime, startCoreEvent);

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

    numHLMACRcvd++;
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
        numHLMACLoopable++;
        EV << "HLMAC adress " << hlmac << " creates a loop in this node. Longest Matched Prefix is " << longestPrefix << endl;
        return true;
    }

    EV << "<-IoToriiOperation::hasLoop()" << endl;
}

void IoToriiOperation::saveHLMAC(HLMACAddress hlmac)
{
    EV << "->IoToriiOperation::saveHLMAC()" << endl;

    hlmacTable->updateTableWithAddress(-1, hlmac);
    numHLMACAssigned++;
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
    delete cInfo;

    eGA3Frame eGA3dst(macdest);
    HLMACAddress dest = eGA3dst.getHLMACAddress();
    unsigned char type = eGA3dst.geteGA3FrameType();
    EV_DETAIL << "A message has been received from upper layer, name is " << msg->getName() << ", CInfo removed, dst MAC addr=" << macdest <<", dst HLMAC addr=" << dest << ", dst HLMAC type=" << (unsigned short int) type << endl;
    CSMAFramePANID *frame = new CSMAFramePANID(msg->getName());
    frame->setBitLength(headerLength);
    EV << "frame has been created. frame header length is set to " <<  frame->getBitLength() << " (bits)." << endl;
    frame->setDestAddr(macdest);
    MetricType metric = HopCount;
    HLMACAddress bestSrcAddr = hlmacTable -> getSrcAddress(dest, metric);
    if (bestSrcAddr == HLMACAddress::UNSPECIFIED_ADDRESS){ //if this node has not any HLMAC address
        EV << "This node has not any HLMAC Address for this destination (or at all), frame is deleted." << endl;
        numDiscardedNoHLMAC++;
        delete frame;
        delete msg;
        return;
    }

    eGA3Frame eGA3src;
    eGA3src.setHLMACAddress(bestSrcAddr);
    MACAddress macsrc(eGA3src.getInt());
    frame->setSrcAddr(macsrc);

    if (dest != HLMACAddress::BROADCAST_ADDRESS){  //unicast transmission
        unsigned int lenCommonAncestor = 0;
        unsigned int counter = 0;
        HLMACAddress commonAncestor;
        if ((commonAncestor = bestSrcAddr.getLongestCommonPrefix(dest)) != HLMACAddress::UNSPECIFIED_ADDRESS){
            lenCommonAncestor = (unsigned int)(commonAncestor.getHLMACHier()) + 1;
            counter = ((unsigned int)bestSrcAddr.getHLMACHier() + 1) + ((unsigned int)dest.getHLMACHier() + 1) - 2 * lenCommonAncestor;
            EV << "Best selected src HLMAC address is " << bestSrcAddr << "src length is " << bestSrcAddr.getHLMACHier() + 1 << ", dst HLMAC address is " << dest << ", dst length is " << dest.getHLMACHier() + 1 << ", longest common ancestor HLMAC address is " << commonAncestor << ", longest common ancestor length is " << lenCommonAncestor << ", counter is " << counter << endl;
            frame->setSrcPANID(MACAddress(counter)); //src PAN ID is reused for saving counter in unicast transmission
            EV << "Unicast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (counter) is " << counter << endl;
        }
        else
            throw cRuntimeError("src and dst has no common ancestor!");
    }
    else{   //broadcast transmission
        if (broadcastType == 1){  //Only upward broadcast by using counter
            unsigned int lenCommonAncestor = 0;
            unsigned int counter = 0;
            HLMACAddress commonAncestor;
            HLMACAddress dest;
            dest.setCore(bestSrcAddr.getIndexValue(0));  //HLMACAddress dest(bestSrcAddr.getHLMACHier(0));
            EV << "Broadcast type = " << broadcastType << ", selected (best) Src Addr is " << bestSrcAddr << ", Sink address is " << dest <<  endl;
            if ((commonAncestor = bestSrcAddr.getLongestCommonPrefix(dest)) != HLMACAddress::UNSPECIFIED_ADDRESS){
                lenCommonAncestor = (unsigned int)(commonAncestor.getHLMACHier()) + 1;
                counter = ((unsigned int)bestSrcAddr.getHLMACHier() + 1) + ((unsigned int)dest.getHLMACHier() + 1) - 2 * lenCommonAncestor;
                EV << "Best selected src HLMAC address is " << bestSrcAddr << "src length is " << bestSrcAddr.getHLMACHier() + 1 << ", dst HLMAC address is " << dest << ", dst length is " << dest.getHLMACHier() + 1 << ", longest common ancestor HLMAC address is " << commonAncestor << ", longest common ancestor length is " << lenCommonAncestor << ", counter is " << counter << endl;
                frame->setSrcPANID(MACAddress(counter)); //src PAN ID is reused for saving counter in unicast transmission
                EV << "Unicast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (counter) is " << counter << endl;
            }
        }
        else if (broadcastType == 2){  //Only upward broadcast by using transmitter address
            frame->setSrcPANID(macsrc); //src PAN ID is reused for saving transmitter in broadcast transmission
            EV << "Broadcast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (Transmitter address) is " << eGA3src << endl;
        }
        else if (broadcastType == 3){  //Broadcast for Up/Downward and P2P traffic
            frame->setSrcPANID(macsrc); //src PAN ID is reused for saving transmitter in broadcast transmission
            EV << "Broadcast frame is prepared to phy layer, src is  " << eGA3src << ", dst is " << eGA3dst << ", src PAN ID (Transmitter address) is " << eGA3src << endl;
        }
        else
            throw cRuntimeError("Broadcast type %d in not defined.", broadcastType);

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

    numReceivedLowerPacket++;

    if ((strcmp(msg->getName(),"Hello!")==0))
    {
        CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);
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
    else if ((strcmp(msg->getName(),"SetHLMAC")==0)) {
        CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);
        receiveSetHLMACMessage(frame);
        return;
    } //END SetHLMAC
    else  // Data frame
    {
        CSMAFramePANID *frame = check_and_cast<CSMAFramePANID *>(msg);

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
            EV << "Data frame is a broadcast frame. it is sent to broadcast proccess." << endl;
            //sendUp(decapsMsg(frame->dup()));
            //nbRxFrames++;
            if (broadcastType == 1){  //Only upward broadcast by using counter
                EV << "broadcastType is " << broadcastType << " : Only upward broadcast by using counter"<< endl;
                broadcastProccessUpwardTraffic1(frame);
            }
            else if (broadcastType == 2){  //Only upward broadcast by using transmitter address
                EV << "broadcastType is " << broadcastType << " : Only upward broadcast by using transmitter address"<< endl;
                broadcastProccessUpwardTraffic2(frame);
            }
            else if (broadcastType == 3){  //Broadcast for Up/Downward and P2P traffic
                EV << "broadcastType is " << broadcastType << " : Broadcast for Up/Downward and P2P traffic"<< endl;
                broadcastProccessAllwardTraffic(frame);
            }
            else
                throw cRuntimeError("Broadcast type %d in not defined.", broadcastType);
        }
        else {     //Data frame is not mine. Send it to routing proccess
            EV << "Data frame is not mine and is not broadcast. it need to routing, so it is sent to routing proccess." << endl;
            //nbRxFrames++;
            routingProccess(frame);
        }
    }
    EV << "<-IoToriiOperation::handleLowerPacket()" << endl;
}

void IoToriiOperation::routingProccess(CSMAFramePANID *frame)
{
    EV << "->IoToriiOperation::routingProccess()" << endl;
    //CSMAFramePANID *framePANID = check_and_cast<CSMAFramePANID *>(frame);
    HLMACAddress src(frame->getSrcAddr().getInt());
    HLMACAddress dst(frame->getDestAddr().getInt());
    HLMACAddress myHLMACAddress, commonAncestor = src.getLongestCommonPrefix(dst);
    EV << "src is " << src << ", dst is " << dst << ", commonAncestor is " << commonAncestor << endl;

    unsigned int lenCommonAncestor = 0;
    unsigned int counter = (unsigned int)frame->getSrcPANID().getInt();
    unsigned int lenSrc = (unsigned int)src.getHLMACHier() + 1;
    unsigned int lenDst = (unsigned int)dst.getHLMACHier() + 1;
    if (commonAncestor != HLMACAddress::UNSPECIFIED_ADDRESS)
        lenCommonAncestor = (unsigned int)(commonAncestor.getHLMACHier()) + 1;
    EV << "lenSrc is " << lenSrc << ", lenDst " << lenDst << ", counter is " << counter << ", lenCommonAncestor is " << lenCommonAncestor << endl;

    //Routing algorithm
    if (((myHLMACAddress = hlmacTable->getlongestMatchedPrefix(src)) != HLMACAddress::UNSPECIFIED_ADDRESS) &&
        (myHLMACAddress >= commonAncestor) &&
        (((unsigned int)myHLMACAddress.getHLMACHier() + 1) == counter)){
        EV << "1: myHLMACAddress is " << myHLMACAddress << endl;
        counter--;
        EV << "1: new counter is " << counter << endl;
        frame->setSrcPANID(MACAddress(counter));
        EV << "1: frame is forwarded." << endl;
        sendDown(frame);

    }
    else if (((myHLMACAddress = hlmacTable->getlongestMatchedPrefix(dst)) != HLMACAddress::UNSPECIFIED_ADDRESS) &&
            ((lenDst - ((unsigned int)myHLMACAddress.getHLMACHier() + 1) + 1) == counter)){
        EV << "2: myHLMACAddress is " << myHLMACAddress << endl;
        counter--;
        EV << "2: new counter is " << counter << endl;
        frame->setSrcPANID(MACAddress(counter));
        EV << "2: frame is forwarded." << endl;
        sendDown(frame);
    }
    else{
        EV << "3: frame is deleted." << endl;
        delete frame;
    }
    EV << "<-IoToriiOperation::routingProccess()" << endl;
}

void IoToriiOperation::broadcastProccessUpwardTraffic1(CSMAFramePANID *frame)
{
    EV << "->IoToriiOperation::broadcastProccessUpwardTraffic1()" << endl;
    HLMACAddress src(frame->getSrcAddr().getInt());
    HLMACAddress dst;
    dst.setCore(src.getIndexValue(0));  //  dst.setCore(src.getCore());

    HLMACAddress myHLMACAddress, commonAncestor = src.getLongestCommonPrefix(dst);

    unsigned int lenCommonAncestor = 0;
    unsigned int counter = (unsigned int)frame->getSrcPANID().getInt();
    unsigned int lenSrc = (unsigned int)src.getHLMACHier() + 1;
    unsigned int lenDst = (unsigned int)dst.getHLMACHier() + 1;
    if (commonAncestor != HLMACAddress::UNSPECIFIED_ADDRESS)
        lenCommonAncestor = (unsigned int)(commonAncestor.getHLMACHier()) + 1;

    //Routing algorithm
    if (((myHLMACAddress = hlmacTable->getlongestMatchedPrefix(src)) != HLMACAddress::UNSPECIFIED_ADDRESS) &&
        (myHLMACAddress >= commonAncestor) &&
        (((unsigned int)myHLMACAddress.getHLMACHier() + 1) == counter)){
        sendUp(decapsMsg(frame->dup()));
        counter--;
        frame->setSrcPANID(MACAddress(counter));
        sendDown(frame);

    }
    else
        delete frame;

    EV << "<-IoToriiOperation::broadcastProccessUpwardTraffic1()" << endl;
}


void IoToriiOperation::broadcastProccessUpwardTraffic2(CSMAFramePANID *frame)
{
    EV << "->IoToriiOperation::broadcastProccessUpwardTraffic2()" << endl;

    EV << "<-IoToriiOperation::broadcastProccessUpwardTraffic2()" << endl;
}

void IoToriiOperation::broadcastProccessAllwardTraffic(CSMAFramePANID *frame)
{
    EV << "->IoToriiOperation::broadcastProccessAllwardTraffic()" << endl;
    HLMACAddress src(frame->getSrcAddr().getInt());
    HLMACAddress transmitter(frame->getSrcPANID().getInt());
    HLMACAddress myHLMACAddress = hlmacTable->getlongestMatchedPrefix(transmitter);
    eGA3Frame eGA3NewTransmitter;
    eGA3NewTransmitter.setHLMACAddress(myHLMACAddress);
    MACAddress newTransmitter(eGA3NewTransmitter.getInt());

    unsigned int lenTransmitter = transmitter.getHLMACHier() + 1;
    unsigned int lenMyHLMACAddress;
    if (myHLMACAddress != HLMACAddress::UNSPECIFIED_ADDRESS)
        lenMyHLMACAddress = myHLMACAddress.getHLMACHier() + 1;
    else
        lenMyHLMACAddress = 0;

    //Broadcast algorithm
    if (myHLMACAddress != HLMACAddress::UNSPECIFIED_ADDRESS)
        if ((lenTransmitter - lenMyHLMACAddress == 1) && transmitter.isPrefixOf(src) && hlmacTable->isNearest(myHLMACAddress, src)){
            EV << "1: myHLMACAddress is " << myHLMACAddress << ", lenMyHLMACAddress is " << lenMyHLMACAddress << ", getlongestMatchedPrefix(src: " << src << ") is " << hlmacTable->getlongestMatchedPrefix(src) << endl;
            EV << "src is " << src << ", transmitter is " << transmitter << ", len transmitter is " << lenTransmitter << endl;
            EV << "1: this broadcast frame is not duplicate, a copy of it is sent to upper layer."<< endl;
            sendUp(decapsMsg(frame->dup()));
            EV << "1: transmitter address is updated to (in form of mac address: " << newTransmitter << ", in form of HLMAC address: " << eGA3NewTransmitter << ")."<< endl;
            frame->setSrcPANID(newTransmitter);
            EV << "1: the broadcast frame is not duplicate, it is sent to lower layer to broadcast to the other nodes."<< endl;
            sendDown(frame);
        }
        else{
            delete frame;
            EV << "1: myHLMACAddress is " << myHLMACAddress << ", getlongestMatchedPrefix(src: " << src << ") is " << hlmacTable->getlongestMatchedPrefix(src) << endl;
            EV << "1: myHLMACAddress is " << myHLMACAddress << ", lenMyHLMACAddress is " << lenMyHLMACAddress << endl;
            EV << "1: the broadcast frame is dublicate, it is deleted." << endl;
        }
    else if (transmitter == (myHLMACAddress = hlmacTable->getShortestAddressForPrefix(transmitter)).getWithoutLastId() && myHLMACAddress != hlmacTable->getlongestMatchedPrefix(src) && hlmacTable->isNearest(myHLMACAddress, src)){
        EV << "2: myHLMACAddress is " << myHLMACAddress << ", getlongestMatchedPrefix(src: " << src << ") is " << hlmacTable->getlongestMatchedPrefix(src) << endl;
        EV << "2: the hlmac address of " << hlmacTable->getNearestTo(src) << " is the nearest address to src(" << src << ")."<< endl;
        EV << "2: the broadcast frame is not duplicate, a copy of it is sent to upper layer."<< endl;
        sendUp(decapsMsg(frame->dup()));
        eGA3NewTransmitter.setHLMACAddress(myHLMACAddress);
        MACAddress newTransmitter(eGA3NewTransmitter.getInt());
        EV << "2: transmitter address is updated to (in form of mac address: " << newTransmitter << ", in form of HLMAC address: " << eGA3NewTransmitter << ")."<< endl;
        frame->setSrcPANID(newTransmitter);
        sendDown(frame);
        EV << "2: the broadcast frame is not duplicate, it is sent to lower layer to broadcast to the other nodes."<< endl;
    }
    else{
        delete frame;
        EV << "2: myHLMACAddress is " << myHLMACAddress << ", getlongestMatchedPrefix(src: " << src << ") is " << hlmacTable->getlongestMatchedPrefix(src) << endl;
        EV << "2: the hlmac address of " << hlmacTable->getNearestTo(src) << " is the nearest address to src(" << src << ")."<< endl;
        EV << "2: the broadcast frame is dublicate, it is deleted." << endl;
    }
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

