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

#include "src/adaption/_6LoWPAN.h"

#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

#include "inet/common/ModuleAccess.h"
#include <vector>
//#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"

#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"
#include "inet/networklayer/icmpv6/IPv6NDMessage_m.h"
#include "inet/transportlayer/udp/UDPPacket.h"

namespace iotorii {
using namespace inet;


Define_Module(_6LoWPAN);

_6LoWPAN::_6LoWPAN() :
    netIn(-1),
    netOut(-1),
    ifIn(-1),
    ifOut(-1),
    udpHeaderLength(4),
    dispatchHeaderLength(1),
    ipv6HeaderLength(2),
    icmpHeaderLength(8),
    nsHeaderLength(0),
    naHeaderLength(0)

{
}

void _6LoWPAN::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        netIn = findGate("netIn");
        netOut = findGate("netOut");
        ifIn = findGate("ifIn");
        ifOut = findGate("ifOut");

        udpHeaderLength = par("udpHeaderLength");
        dispatchHeaderLength = par("dispatchHeaderLength");
        ipv6HeaderLength = par("ipv6HeaderLength");
        //icmpHeaderLength = par("icmpHeaderLength");
        nsHeaderLength = par("nsHeaderLength");
        naHeaderLength = par("naHeaderLength");

        WATCH(udpHeaderLength);
        WATCH(dispatchHeaderLength);
        WATCH(ipv6HeaderLength);
        WATCH(icmpHeaderLength);
        WATCH(nsHeaderLength);
        WATCH(naHeaderLength);

    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

    }
}

void _6LoWPAN::handleMessage(cMessage *msg)
{
    EV << "->_6LoWPAN::handleMessage()" << endl;

    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

    if (isUpperMessage(msg)) {
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


    EV << "<-_6LoWPAN::handleMessage()" << endl;
}

void _6LoWPAN::handleUpperPacket(cMessage *msg)
{
    EV << "->_6LoWPAN::handleUpperPacket()" << endl;
    ipv6HeaderLength += dispatchHeaderLength;
    if (dynamic_cast<IPv6Datagram *>(msg)){    //if (dynamic_cast<IPv6Datagram *>(msg)){
        IPv6Datagram *ipv6dg = check_and_cast<IPv6Datagram *>(msg);
        EV << "IPv6 datagram is received from upper layer, datagram size is " << ipv6dg->getByteLength() << " (Bytes)." << endl;
        cPacket *payload = ipv6dg->decapsulate();
        EV << "header length is " << ipv6dg->getByteLength() << "(Bytes), payload size is " << payload->getByteLength() << "(Bytes), ";
        ipv6dg->setByteLength(ipv6HeaderLength);
        EV <<"new header length is " << ipv6dg->getByteLength() << "(Bytes)." << endl;

        if (dynamic_cast<UDPPacket *>(payload)){    //if(dynamic_cast<UDPPacket *>(ipv6dg)){
            UDPPacket *udpPacket = check_and_cast<UDPPacket *>(payload);
            cPacket *udpPayload = payload->decapsulate();
            EV << "Encapsulated packet is UDP, header length is " << udpPacket->getByteLength() << "UDP payload is " << udpPayload->getByteLength() << "(Bytes), ";
            udpPacket->setByteLength(udpHeaderLength);
            EV <<"new header length is " << udpPacket->getByteLength() << "(Bytes)." << endl;
            udpPacket->encapsulate(udpPayload);
            EV <<"new UDP length is " << udpPacket->getByteLength() << "(Bytes)." << endl;
            ipv6dg->encapsulate(udpPacket);
            EV << "UDP Packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
        }
        else if (dynamic_cast<ICMPv6Message *>(payload)){
            if (dynamic_cast<IPv6NeighbourSolicitation *>(payload)){
                IPv6NeighbourSolicitation *icmpPacket = check_and_cast<IPv6NeighbourSolicitation *>(payload);
                EV << "Encapsulated packet is Neighbor Solicitation packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                icmpPacket->setByteLength(nsHeaderLength);
                EV <<"new compressed length according to 6LoWPAN is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                ipv6dg->encapsulate(icmpPacket);
                EV << "ICMP packet is encapsulated again. new size of datagram is " << ipv6dg->getByteLength() << endl;
            }
            else if (dynamic_cast<IPv6NeighbourAdvertisement *>(payload)){
                IPv6NeighbourAdvertisement *icmpPacket = check_and_cast<IPv6NeighbourAdvertisement *>(payload);
                EV << "Encapsulated packet is Neighbor Advertisement packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                icmpPacket->setByteLength(naHeaderLength);
                EV <<"new compressed length according to 6LoWPAN is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                ipv6dg->encapsulate(icmpPacket);
                EV << "ICMP packet is encapsulated again. new size of datagram is " << ipv6dg->getByteLength() << endl;
            }
            else{  //other icmp messages. note that RS and RA is not used in this simulation. FIXME : icmp header size in ping is 0
                ICMPv6Message *icmpPacket = check_and_cast<ICMPv6Message *>(payload);
                if (icmpPacket->hasEncapsulatedPacket()){ //for ping
                    cPacket *payloadicmp = icmpPacket->decapsulate(); //PingPayload *payload = check_and_cast<PingPayload *>(icmpPacket);
                    EV << "Encapsulated packet is ICMP packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                    EV << "ICMP has an encapsulated packet (maybe ping), payload length is " << payloadicmp->getByteLength() << "(Bytes), ";
                    icmpPacket->setByteLength(icmpHeaderLength);
                    EV <<"new icmp header length is " << icmpPacket->getByteLength() << "(Bytes)";
                    icmpPacket->encapsulate(payloadicmp);
                    EV <<"new icmp length is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                    ipv6dg->encapsulate(icmpPacket);
                    EV << "ICMP packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
                }
               /* else{  //icmp without payload, other ICMPs
                    EV << "Encapsulated packet is ICMP packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                    icmpPacket->setByteLength(icmpHeaderLength);
                    EV <<"new icmp header length is " << icmpPacket->getByteLength() << "(Bytes)";
                    ipv6dg->encapsulate(icmpPacket);
                    EV << "ICMP packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
                }*/
            }
        } //end icmp
        else
            throw cRuntimeError("6LoWPAN::handleUpperPacket(): Unknown payload is arrived.");
    } //end ipv6
    else
        throw cRuntimeError("6LoWPAN::handleUpperPacket(): Unknown datagram is arrived.");
    sendDown(msg);

    EV << "<-_6LoWPAN::handleUpperPacket()" << endl;
}

void _6LoWPAN::handleLowerPacket(cMessage *msg)
{
    EV << "->_6LoWPAN::handleLowerPacket()" << endl;
    if (dynamic_cast<IPv6Datagram *>(msg)){    //if (dynamic_cast<IPv6Datagram *>(msg)){
        IPv6Datagram *ipv6dg = check_and_cast<IPv6Datagram *>(msg);
        EV << "IPv6 datagram is received from lower layer, datagram size is " << ipv6dg->getByteLength() << " (Bytes)." << endl;
        cPacket *payload = ipv6dg->decapsulate();
        EV << "header length is " << ipv6dg->getByteLength() << "(Bytes), payload size is " << payload->getByteLength() << "(Bytes), ";
        ipv6dg->setByteLength(IPv6_HEADER_BYTES);
        EV <<"new header length is " << ipv6dg->getByteLength() << "(Bytes)." << endl;

        if (dynamic_cast<UDPPacket *>(payload)){    //if(dynamic_cast<UDPPacket *>(ipv6dg)){
            UDPPacket *udpPacket = check_and_cast<UDPPacket *>(payload);
            cPacket *udpPayload = payload->decapsulate();
            EV << "Encapsulated packet is UDP, header length is " << udpPacket->getByteLength() << "UDP payload is " << udpPayload->getByteLength() << "(Bytes), ";
            udpPacket->setByteLength(UDP_HEADER_BYTES);
            EV <<"new header length is " << udpPacket->getByteLength() << "(Bytes)." << endl;
            udpPacket->encapsulate(udpPayload);
            EV <<"new UDP length is " << udpPacket->getByteLength() << "(Bytes)." << endl;
            ipv6dg->encapsulate(udpPacket);
            EV << "UDP Packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
        }
        else if (dynamic_cast<ICMPv6Message *>(payload)){
            if (dynamic_cast<IPv6NeighbourSolicitation *>(payload)){
                IPv6NeighbourSolicitation *icmpPacket = check_and_cast<IPv6NeighbourSolicitation *>(payload);
                EV << "Encapsulated packet is Neighbor Solicitation packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                icmpPacket->setByteLength(ICMPv6_HEADER_BYTES + IPv6_ADDRESS_SIZE + IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
                EV <<"new header length is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                ipv6dg->encapsulate(icmpPacket);
                EV << "ICMP packet is encapsulated again. new size of datagram is " << ipv6dg->getByteLength() << endl;
            }
            else if (dynamic_cast<IPv6NeighbourAdvertisement *>(payload)){
                IPv6NeighbourAdvertisement *icmpPacket = check_and_cast<IPv6NeighbourAdvertisement *>(payload);
                EV << "Encapsulated packet is Neighbor Advertisement packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                icmpPacket->setByteLength(ICMPv6_HEADER_BYTES + IPv6_ADDRESS_SIZE + IPv6ND_LINK_LAYER_ADDRESS_OPTION_LENGTH);
                EV <<"new header length is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                ipv6dg->encapsulate(icmpPacket);
                EV << "ICMP packet is encapsulated again. new size of datagram is " << ipv6dg->getByteLength() << endl;
            }
            else{  //other icmp messages. note that RS and RA is not used in this simulation.
                     ICMPv6Message *icmpPacket = check_and_cast<ICMPv6Message *>(payload);
                     if (icmpPacket->hasEncapsulatedPacket()){  //for ping
                         cPacket *payloadicmp = icmpPacket->decapsulate(); //PingPayload *payload = check_and_cast<PingPayload *>(icmpPacket);
                         EV << "Encapsulated packet is ICMP packet, header length is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                         EV << "ICMP has an encapsulated packet (maybe ping), payload length is " << payloadicmp->getByteLength() << "(Bytes)" << endl;
                         icmpPacket->setByteLength(ICMPv6_HEADER_BYTES);
                         EV <<"new icmp header length is " << icmpPacket->getByteLength() << "(Bytes)";
                         icmpPacket->encapsulate(payloadicmp);
                         EV <<"new icmp length is " << icmpPacket->getByteLength() << "(Bytes)" << endl;
                         ipv6dg->encapsulate(icmpPacket);
                         EV << "ICMP packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
                     }
                   /*  else{  //icmp without payload
                         EV << "Encapsulated packet is ICMP packet, header length is " << icmpPacket->getByteLength() << "(Bytes), ";
                         icmpPacket->setByteLength(ICMPv6_HEADER_BYTES);
                         EV <<"new icmp header length is " << icmpPacket->getByteLength() << "(Bytes)";
                         ipv6dg->encapsulate(icmpPacket);
                         EV << "ICMP packet is encapsulated to IPv6 datagram again. new size of datagram is " << ipv6dg->getByteLength() << endl;
                     }*/
            }//end ping and other ICMPs
        } //end icmp
        else
            throw cRuntimeError("6LoWPAN::handleUpperPacket(): Unknown payload is arrived.");
    } //end ipv6
    else
        throw cRuntimeError("6LoWPAN::handleUpperPacket(): Unknown datagram is arrived.");
    sendUp(msg);

    EV << "<-_6LoWPAN::handleLowerPacket()" << endl;
}

void _6LoWPAN::sendUp(cMessage *message)
{
//    if (message->isPacket())
//        emit(packetSentToUpperSignal, message);
    send(message, netOut);
}

void _6LoWPAN::sendDown(cMessage *message)
{
//    if (message->isPacket())
//        emit(packetSentToLowerSignal, message);
    send(message, ifOut);
}

bool _6LoWPAN::isUpperMessage(cMessage *message)
{
    return message->getArrivalGateId() == netIn;
}

bool _6LoWPAN::isLowerMessage(cMessage *message)
{
    return message->getArrivalGateId() == ifIn;
}

void _6LoWPAN::start()
{
    EV << "->_6LoWPAN::start()" << endl;

    isOperational = true;

    EV << "<-_6LoWPAN::start()" << endl;
}

void _6LoWPAN::stop()
{
    EV << "->_6LoWPAN::stop()" << endl;

    isOperational = false;

    EV << "<-_6LoWPAN::stop()" << endl;
}

bool _6LoWPAN::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

void _6LoWPAN::finish()
{
    EV << "->_6LoWPAN::finish()" << endl;

    //recordScalar("Received Upper Packets", numReceivedUpperPacket);
    //recordScalar("Received Lower frames", numReceivedLowerPacket);
    //recordScalar("discarded frames", numDiscardedFrames);
    //recordScalar("Received Hello", numHelloRcvd);

    EV << "<-_6LoWPAN::finish()" << endl;
}

_6LoWPAN::~_6LoWPAN()
{

}

} // namespace iotorii

