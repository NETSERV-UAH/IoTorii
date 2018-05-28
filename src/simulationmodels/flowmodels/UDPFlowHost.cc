//
// Copyright (C) 2012 Elisa Rojas (inspired in inet/UDPBasicApp)
//
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    INET 3.6.3 adaptation, also adapted for using in the wARP-PATH protocol
*/
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    INET 3.6.3 adaptation, also adapted for using in the IoTorii(WSN) protocol
*/
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "src/simulationmodels/flowmodels/UDPFlowHost.h"
#include "inet/applications/base/ApplicationPacket_m.h"

#include <stdio.h>
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
//#include "InterfaceTableAccess.h"
#include "inet/common/ModuleAccess.h"  // findModuleFromPar()
#include "inet/networklayer/common/L3AddressResolver.h"
#include "src/simulationmodels/flowmodels/UDPFlowGenerator.h"


namespace iotorii {
using namespace inet;

Define_Module(UDPFlowHost);

int UDPFlowHost::counter;
simsignal_t UDPFlowHost::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPFlowHost::rcvdPkSignal = SIMSIGNAL_NULL;

//class constructor
UDPFlowHost::UDPFlowHost()
{

}

//class destructor
UDPFlowHost::~UDPFlowHost()
{
	for(unsigned int i=0; i<flowMessages.size(); i++)
		cancelAndDelete(flowMessages[i]);
}


void UDPFlowHost::initialize(int stage)
{
	//Initializes at the same stage that the UDPFlowGenerator module
	//if(stage == 3)
    if(stage == INITSTAGE_NETWORK_LAYER_3)
	{
	    counter = 0;
	    sentPkSignal = registerSignal("sentPk");
	    rcvdPkSignal = registerSignal("rcvdPk");
	}
}

void UDPFlowHost::startFlow(double transferRate, unsigned long long flowSize, unsigned int frameSize, const L3Address& destAddr, const L3Address& localAddr) //Kbps, B(KB*1024), B, address
//void UDPFlowHost::startFlow(unsigned int transferRate, unsigned long long flowSize, unsigned int frameSize, const L3Address& destAddr) //Kbps, B(KB*1000), B, address
{
	/*Since this module function is called from another - UDPFlowGenerator - we need to indicate it via Enter_Method
	 * or Enter_Method_Silent - with no animation -
	 * More about this at: http://www.omnetpp.org/doc/omnetpp/manual/usman.html#sec182 (4.12 Direct Method Calls Between Modules at the Omnet++ manual) */
	Enter_Method("UDPFlowHost::startFlow()");

	EV << "->UDPFlowHost::startFlow()" << endl;
	EV << "  Flow info received at host " << str() << "!" << endl;   //EV << "  Flow info received at host " << info() << "!" << endl;
	EV << "    [" << str() << " -> " << destAddr << "]" << endl;  //EV << "    [" << info() << " -> " << destAddr << "]" << endl;
	EV << "    Transfer Rate: " << transferRate << " (Kbps); Flow Size: " << flowSize << " (B); Frame Size: " << frameSize << " (B)" << endl;

	//Now we check in there's already a flow started for that destination and add only the size (not transfer rate or frame size)
	//or not, or if we need to start a new flow because that destination is still not registered at flowInfo
	int destIndex=-1, nextFreeIndex=-1, i=0;
	for(i=0; i<flowInfo.size(); i++)
	{
		//If we find the destination address, we save its index
		if(flowInfo[i].destAddress == destAddr)
			destIndex = i;

		//Meanwhile, we save the next free entry
		if(nextFreeIndex== -1 && flowInfo[i].localPort == 0) //The entry is free if localPort (and destPort) == 0
			nextFreeIndex = i;
	}

	//If we have already registered the destination address..
	i=0;
	if (destIndex!=-1)
	{
		EV << "  The destination was already registered!" << endl;
		i = destIndex;
		if(flowInfo[i].flowSize == 0) //If flow is inactive, we active it
		{
			//Register the flow
			flowInfo[i].transferRate = transferRate;
			flowInfo[i].flowSize = flowSize;
			flowInfo[i].frameSize = frameSize;

	    	//Start the handler to generate the flow traffic
	    	if(frameSize > flowSize) frameSize = flowSize; //Should never happen (with JAC's model), but just in case it happens //###
	    	//EXTRA BEGIN
	    	//double startTime = double(frameSize*8)/(transferRate*1000); //(B*8)/(Kbps*1000)
            double startTime = double(frameSize*8)/(transferRate*1024); //(B*8)/(Kbps*1024)
	    	//EXTRA END
	    	char indexName[6]; indexName[0] = 'G'; indexName[1] = 'e'; indexName[2] = 'n'; indexName[3] = '-';
	    	indexName[4] = i; indexName[5] = 0; //#i \x0i\x00 (cMessage copia la cadena y la hace 'const')
	    	cMessage *timer = new cMessage(indexName); //To distinguish events, we pass the index as the name of the timer
	    	if(simTime()+startTime <= stopTime)
	    	{
	    		scheduleAt(simTime()+startTime, timer);
	    		EV << "    A new flow starts at T=" << simTime()+startTime << " (now T=" << simTime() << ")" << endl;
	    		flowMessages.push_back(timer);
	    	}
	    	else
	    		EV << "    A new flow will not start at T=" << simTime()+startTime << " > stopTime = " << stopTime << endl;
		}
		else //Otherwise, we add more traffic to it
		{
			//Add more traffic to the existing flow, but the frame size or transfer rate will not change
			flowInfo[i].flowSize = flowInfo[i].flowSize + flowSize;
			//The handler is already active... no need to scheduleAt...
			EV << "    The flow was already running, we just added " << flowSize << "(B)"<< endl;
		}
	}
	//If not, we register it
	else
	{
		EV << "  The destination was not registered at this source!" << endl;
		i = nextFreeIndex;

		//Assign port numbers
		flowInfo[i].localPort = 1000+nextFreeIndex; //From 1000 to 1999
		//flowInfo[i].destPort = 2000+nextFreeIndex; //From 2000 to 2999

		//regist socket in dst node
		UDPFlowHost * pUdpFlowHost = check_and_cast<UDPFlowHost *>(L3AddressResolver().findHostWithAddress(destAddr)->getSubmodule("udpGen"));
		flowInfo[i].destPort = pUdpFlowHost->registDstSocket(localAddr);
		EV << "Socket setting : local address " << localAddr << " local port " << flowInfo[i].localPort << "-->" << "dest address " << destAddr << "dest port" << flowInfo[i].destPort << endl;


	    //Create socket and bind to localPort
		flowInfo[i].socket.setOutputGate(gate("udpOut"));
		//flowInfo[i].socket.bind(flowInfo[i].localPort);
		//flowInfo[i].socket.bind(localAddr, flowInfo[i].localPort);
		flowInfo[i].socket.connect(destAddr, flowInfo[i].destPort);
		EV <<"my address is :"<<localAddr<<endl;
		setSocketOptions(flowInfo[i].socket);

	    //Register the flow
	    flowInfo[i].destAddress = destAddr;
    	flowInfo[i].transferRate = transferRate;
    	flowInfo[i].flowSize = flowSize;
    	flowInfo[i].frameSize = frameSize;

    	//Start the handler to generate the flow traffic
    	if(frameSize > flowSize) frameSize = flowSize; //Should never happen, but just in case it happens //###
    	//EXTRA BEGIN
    	//double startTime = double(frameSize*8)/(transferRate*1000); //(B*8)/(Kbps*1000)
        double startTime = (frameSize*8)/(transferRate*1024); //(B*8)/(Kbps*1024)
    	//EXTRA END
    	char indexName[6]; indexName[0] = 'G'; indexName[1] = 'e'; indexName[2] = 'n'; indexName[3] = '-';
    	indexName[4] = i; indexName[5] = 0; //#i \x0i\x00 (cMessage copia la cadena y la hace 'const')
    	cMessage *timer = new cMessage(indexName); //To distinguish events, we pass the index as the name of the timer
    	if(simTime()+startTime <= stopTime)
    	{
    		scheduleAt(simTime()+startTime, timer);
    		EV << "    A new flow starts at T=" << simTime()+startTime << " (now T=" << simTime() << ")" << endl;
    		EV << "      #" << i << "-> SocketId: " << flowInfo[i].socket.getSocketId() << "; Local port: " << flowInfo[i].localPort << "; Destination port: " << flowInfo[i].destPort << endl;
    		flowMessages.push_back(timer);
    	}
    	else
    		EV << "    A new flow will not start at " << simTime()+startTime << " > stopTime = " << stopTime << endl;
	}

	EV << "<-UDPFlowHost::startFlow()" << endl;
}

cPacket *UDPFlowHost::createPacket(unsigned int frameSize)
{
    char msgName[32];
    sprintf(msgName,"UDPFlowHost-%d", counter++);
    cPacket *payload = new cPacket(msgName);
    //ApplicationPacket *payload = new ApplicationPacket(msgName); // because its seq# to ignore duplicate packets
    payload->setByteLength(frameSize);
    //payload->setSequenceNumber(counter++); // for ApplicationPacket. pay attention to ++ in above sprintf. remove ++ there
    return payload;
}

void UDPFlowHost::sendPacket(unsigned int frameSize, unsigned int nFlow)
{
	//Generate payload and send it via UDP
	cPacket *payload = createPacket(frameSize);

    emit(sentPkSignal, payload);
    //flowInfo[nFlow].socket.sendTo(payload, flowInfo[nFlow].destAddress, flowInfo[nFlow].destPort);
    flowInfo[nFlow].socket.send(payload);

    updateSentStats(frameSize);
}

void UDPFlowHost::handleMessage(cMessage *msg)
{
	EV << "->UDPFlowHost::handleMessage()" << endl;
	if (msg->isSelfMessage())
	{
		deleteMessageFromVector(msg); //Delete it from the pending messages vector

	    // send, then reschedule next sending
	    if (simTime()<=stopTime)
	    {
	    	std::string indexName = msg->getName(); //From const char * to string (not possible to char *)
	    	unsigned char c = indexName[4]; //The char #5 (index 4) of the chain indicates the flowInfo index
	    	unsigned int i = c; //First from char to unsigned char and then to unsigned int (otherwise, the 'i' could get a wrong value)
	    	EV << "  Generating a new UDP packet for flow #" << i+1 << endl;

	    	//EXTRA BEGIN
	    	//int transferRate = flowInfo[i].transferRate;
	    	double transferRate = flowInfo[i].transferRate;
	    	//EXTRA END
	    	long long flowSize = flowInfo[i].flowSize;
	    	int frameSize = flowInfo[i].frameSize;

	    	//Send next frameSize(B) UDP packet and substract it from the flow size (total)
	    	if(frameSize > flowSize) frameSize = flowSize; //Unlikely, but just in case it happens
	    	sendPacket(frameSize, i); //Send packet of size 'frameSize' and flow parameters at vector position 'i'
	    	flowInfo[i].flowSize = flowSize - frameSize;
	    	EV << "    " << frameSize << "/" << flowSize << "(B) at rate " << transferRate << ")" << endl;

	    	//If flow has not finished, reschedule
	    	if(flowInfo[i].flowSize > 0)
	    	{
	    	    //EXTRA BEGIN
	    		//double nextTime = double(frameSize*8)/(transferRate*1000); //(B*8)/(Kbps*1000)
	    	    double nextTime = (frameSize*8)/(transferRate*1024); //(B*8)/(Kbps*1024)
	    	    //EXTRA END
	    		EV <<"BBB next time:" << nextTime << "frame size:"<< frameSize <<"flow size:"<<flowInfo[i].flowSize<<"rate"<<transferRate<<endl;
	        	if(simTime()+nextTime <= stopTime)
	        	{
	        		scheduleAt(simTime()+nextTime, msg);
	        		EV << "    Next UDP packet at T=" << simTime()+nextTime << " (now T=" << simTime() << ")" << endl;
	        		flowMessages.push_back(msg);
	        	}
	        	else
	        	{
	        		EV << "    No next UDP packet at T=" << simTime()+nextTime << " > stopTime = " << stopTime << endl;
	        		delete msg; //Flow ended, delete associated message
	        	}
	    	}
	    	else
	    	{
	    		EV << "    Flow ended at T=" << simTime() << "!" << endl;
	    		delete msg; //Flow ended, delete associated message
	    	}
	    }
	    else
	    {
			EV << "  Host stopped because traffic generator ended! At " << simTime() << " with stop time T=" << stopTime << endl;
			delete msg; //Flow ended, delete associated message
	    }
	}
    else if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

	//if (ev.isGUI())
	if (hasGUI())
	{
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
	}
	EV << "<-UDPFlowHost::handleMessage()" << endl;
}

void UDPFlowHost::processPacket(cPacket *pk)
{
	EV << "->UDPFlowHost::processPacket()" << endl;

    emit(rcvdPkSignal, pk);
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

    updateReceivedStats(simTime(),  pk);


    delete pk;

}

void UDPFlowHost::setSocketOptions(UDPSocket socket)
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0])
    {
        //IInterfaceTable *ift = InterfaceTableAccess().get(this);
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);

        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);
/*
    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups)
        socket.joinLocalMulticastGroups();
        */
}

//This method is called from src node to setting up sockets in dst node
unsigned int UDPFlowHost::registDstSocket(L3Address src)
{
    Enter_Method("UDPFlowHost::reqistDstSocket");

    //Finding a free port in destination
    unsigned int dstPort = 2000 + dstSockets.size();

    auto iter= dstSockets.find(src);
    if (iter == dstSockets.end())
    {
        dstSockets[src] = UDPSocket();
        dstSockets[src].setOutputGate(gate("udpOut"));
        dstSockets[src].bind(dstPort);
    }else
        throw cRuntimeError("Port number %d is being used by `%s'",dstPort, src);
    return dstPort;

}

void UDPFlowHost::deleteMessageFromVector(cMessage *msg)
{
    unsigned int i=0;
    for(i=0; i<flowMessages.size(); i++)
        if(flowMessages[i] == msg)
            break;
    if(i<flowMessages.size())
        flowMessages.erase(flowMessages.begin()+i);
}

void UDPFlowHost::updateHostsInfo(unsigned int n, simtime_t stop)
{
    Enter_Method("UDPFlowHost::updateHostsInfo");

	EV << "->UDPFlowHost::updateHostsInfo()" << endl;
    EV << "  Number of active hosts in the topology = " << n << " and stop time T=" << stop << endl;

    //Updates the number of hosts
    //TODO: This method considers it is called just once (at the beginning of the simulation), since the topology cannot change in real time (right now March'12)

    nHosts = n; //Update number of hosts
    flowInfo.resize(n-1); //n hosts, so n-1 possible destinations

    //Update vector values
    for(unsigned int i=0; i<n-1; i++)
    {
        flowInfo[i].socket = UDPSocket(); //It is necessary to call the constructor so that it generates different socket ids (otherwise, all the sockets in the vector would have the same id and this would throw an error)
    	flowInfo[i].localPort = 0; //From 1000 to 1999 - initialized when the destination address is assigned
    	flowInfo[i].destPort = 0;  //From 2000 to 2999 - initialized when the destination address is assigned
    	flowInfo[i].transferRate = 0;
    	flowInfo[i].flowSize = 0;
    	flowInfo[i].frameSize = 0;
    }

    //Update the stop time (it is defined by the generator (global for the whole topology))
    stopTime = stop;
    EV << "<-UDPFlowHost::updateHostsInfo()" << endl;
}

void UDPFlowHost::finish()
{
	EV << "->UDPFlowHost::finish()" << endl;

    recordScalar("total sent packets", numSent);
    recordScalar("total received packets", numReceived);
    recordScalar("total sent bytes", numSentInbyte);
    recordScalar("total received bytes", numReceivedInbyte);
    recordScalar("total endToEndDelay average", averageEndToEndDelay);
    //recordScalar("sumDelay", sumDelay.dbl());
    recordScalar("lastDelay", lastDelay.dbl());

	//Print statistics...
	EV << "  Printing some statistics..." << endl;
	EV << "    Counter = " << counter << "; #Sent = " << numSent << " ; #Received = " << numReceived << endl;
	EV << "    Traffic offered to the network (as source) = " << (numSent*1500*8)/stopTime.dbl()/1000 << "(Kbps)" << endl; //###Está puesto fijo con tramas de 1500B (cambiar cuando el tamaño de trama sea variable)
	EV << "    Flow Info Vector:" << endl;
	for(unsigned int i=0; i<flowInfo.size(); i++)
	{
		if(!flowInfo[i].destAddress.isUnspecified()) //If destination address is not unspecified, do not print
		{
			EV << "      Flow #" << i+1 << ":" << endl;
			EV << "        Local port: " << flowInfo[i].localPort << "; Destination port: " << flowInfo[i].destPort << endl;
			EV << "        Destination address: " << flowInfo[i].destAddress << endl;
		}
	}

	//Print pending scheduled messages (if any)
	EV << "Pending messages: " << flowMessages.size() << endl;
	for(unsigned int i=0; i<flowMessages.size(); i++)
		EV << "  " << flowMessages[i] << endl;
}

} //namespace iotorii
