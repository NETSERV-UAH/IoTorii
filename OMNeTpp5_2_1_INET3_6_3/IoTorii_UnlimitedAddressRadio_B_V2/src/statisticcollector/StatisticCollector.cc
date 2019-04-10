/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *
 */
//
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

#include <vector>
#include "src/statisticcollector/StatisticCollector.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
//#include <algorithm>    // std::sort


namespace iotorii {
using namespace inet;

Define_Module(StatisticCollector);

    //if ((NodeCounter_Upward[Version]<NodesNumber)&&(!IsDODAGFormed_Upward)) NodeStateLast->DIO.Received++;  //if simulation is not end ...

void StatisticCollector::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL){
        simulationTimeInterval = par("simulationTimeInterval");
    }else if(stage == INITSTAGE_NETWORK_LAYER_3) // for preparing IP addresses of interface table
    {
        extractTopology();
    }
}

void StatisticCollector::extractTopology()
{
    EV << "->StatisticCollector::extractTopology()" << endl;

    // extract topology
    cTopology topo("topo");
    topo.extractByProperty("networkNode"); // topo.extractByProperty("node");
    //cModule *mod = topo.getNode(0)->getModule();

    // fill in nodeStateListVector (isHost and names) and HostInfoVector (IP and MAC addresses)
    nodeStateList.resize(topo.getNumNodes());
    unsigned int nWSN = 0;

    for (unsigned int i=0; i<topo.getNumNodes(); i++)
    {
        struct NodeState newWSN;
        newWSN.host = topo.getNode(i)->getModule();

        if (std::string(newWSN.host->getNedTypeName()).find("WSNHostIoTorii") != std::string::npos) //such as: WSNHostIoTorii, ...
        {
            newWSN.fullName = std::string(newWSN.host->getFullName());
            newWSN.ioToriiOperation = check_and_cast<IoToriiOperation *>(newWSN.host->getSubmodule("wlan[0]")->getSubmodule("mac")->getSubmodule("IoToriiMAC"));
            newWSN.hlmacAddressTable = check_and_cast<IHLMACAddressTable *>(newWSN.ioToriiOperation->getModuleByPath(par("hlmacTablePath")));
            IInterfaceTable *ift = L3AddressResolver().interfaceTableOf(newWSN.host);

            int nInterfaces = ift->getNumInterfaces();
            if(nInterfaces > 2) //If host has more than 2 interfaces...
                error("The host has more than 2 interfaces (one connected and loopback) and that's still not implemented!");
            for (unsigned int k=0; k<nInterfaces; k++)
            {
                InterfaceEntry *ie = ift->getInterface(k);
                //We add only the info about the entry which is not the loopback
                if (!ie->isLoopback())
                {
                    //newWSN.ipAddress = ie->ipv6Data()->getLinkLocalAddress();
                    newWSN.macAddress = ie->getMacAddress();
                    newWSN.moduleIndex = newWSN.host->getIndex();
                    EV << "        " << newWSN.fullName << "->"<< " MAC: " << newWSN.macAddress << "; Module Index: " << newWSN.moduleIndex << "; Vector index: " << i <<endl;
                }
            }
            if (newWSN.macAddress == MACAddress::UNSPECIFIED_ADDRESS){
                throw cRuntimeError("Host has not MAC address!");
            }
            nodeStateList.push_back(newWSN);
        }else{
            throw cRuntimeError("Host is not an IoTorii host!");
        }
    }

    EV << "<-StatisticCollector::extractTopology()" << endl;
}

void StatisticCollector::startStatistics(const MACAddress &sinkAddress, simtime_t time)
{
    Enter_Method("startStatistics()");

    simulationEndEvent = new cMessage("simulationEndEvent");
    scheduleAt(time + simulationTimeInterval, simulationEndEvent);
    sinkID = getIndexFromMACAddress(sinkAddress);
    convergenceTimeStart = time;
    nodeJoined(sinkAddress, time);

}

unsigned int StatisticCollector::getIndexFromMACAddress(const MACAddress &address)
{
    for (unsigned int i = 0; i < nodeStateList.size(); i++){
        if (nodeStateList.at(i).macAddress == address)
            return i;
    }
    return -1;
}

void StatisticCollector::nodeJoined(const MACAddress &address, simtime_t time)
{
    Enter_Method("nodeJoinedDownnward()");

    unsigned int vectorIndex = 0;
    bool found = false;
    while(vectorIndex < nodeStateList.size() && (!found)){
        if (nodeStateList.at(vectorIndex).macAddress == address){
            found = true;
            nodeJoined(vectorIndex, time);
        }
        vectorIndex++;
    }
}

void StatisticCollector::nodeJoined(int nodeID, simtime_t time)
{
    Enter_Method("nodeJoinedDownnward(nodeID)");

    if (!nodeStateList.at(nodeID).isJoined){
        nodeStateList.at(nodeID).isJoined = true;
        nodeStateList.at(nodeID).joiningTime = time;
        if (convergenceTimeEnd < time){
            convergenceTimeEnd = time;
        }
        EV << "Node" << nodeID << "(MAC Address: " << nodeStateList.at(nodeID).macAddress << ", HLMAC Address: " << nodeStateList.at(nodeID).hlmacAddress << " has a route to Root." << endl;
    }
}

void StatisticCollector::collectOtherMetrics()
{
    for (unsigned int i = 0; i < nodeStateList.size(); i++){
        //Collects message statistics
        nodeStateList.at(i).ioToriiOperation->getMessageStatistics(nodeStateList.at(i).numHelloSent, nodeStateList.at(i).numHLMACSent);
        numHelloSentTotal += nodeStateList.at(i).numHelloSent;
        numHLMACSentTotal += nodeStateList.at(i).numHLMACSent;

        //Collects tables statistics
        nodeStateList.at(i).ioToriiOperation->getTableStatistics(nodeStateList.at(i).numAllowedNeighbors, nodeStateList.at(i).numNeighbors);
        numAllowedNeighborsTotal += nodeStateList.at(i).numAllowedNeighbors;
        numNeighborsTotal += nodeStateList.at(i).numNeighbors;

        nodeStateList.at(i).numHLMACAssigned = nodeStateList.at(i).hlmacAddressTable->getNumberOfAddresses();
        numHLMACAssignedTotal += nodeStateList.at(i).numHLMACAssigned;

        //Other metrics
        if (nodeStateList.at(i).numHLMACAssigned == 0){
            (numNotJoinedTotal)++;  //Related to the convergence definition/implementation
        }
        if (nodeStateList.at(i).numNeighbors == 0){
            numWithoutNeighborTotal++;  //Related to the convergence definition/implementation
        }

    }

    calculateHopCount();
}

void StatisticCollector::handleMessage(cMessage* msg)
{
    if (strcmp(msg->getName(),"simulationEndEvent") == 0){
        collectOtherMetrics();
        saveStatistics();
        delete msg;
        endSimulation();
    }else{
        EV << "Unknown self message is deleted." << endl;
        delete msg;
    }

    return;
}

void StatisticCollector::calculateHopCount()
{
}

void StatisticCollector::saveStatistics()
{
    FILE *statisticsCollector;

    statisticsCollector = fopen("01_IoToriiGlobalStats.txt","a");
    fprintf(statisticsCollector," _____________________________________________________________________________________\n");
    fprintf(statisticsCollector,"|Results of new run                                                |\n");
    fprintf(statisticsCollector,"|------------------------------------------------------------------|------------------\n");
    fprintf(statisticsCollector,"| Convergence Time (Total joining time)                            | %f\n", convergenceTimeEnd.dbl() - convergenceTimeStart.dbl());
    fprintf(statisticsCollector,"| Number of total table entries (HLMAC table + Neighbor table)     | %d\n", numAllowedNeighborsTotal + numHLMACAssignedTotal);
    fprintf(statisticsCollector,"| Number of All total table entries (HLMAC table + Neighbor table) | %d\n", numNeighborsTotal + numHLMACAssignedTotal);
    fprintf(statisticsCollector,"| Number of total assigned HLMAC (or number of HLMAC table entries)| %d\n", numHLMACAssignedTotal);
    fprintf(statisticsCollector,"| Number of tot limited neighbors(number of neighbor table entries)| %d\n", numAllowedNeighborsTotal);
    fprintf(statisticsCollector,"| Number of ALL total neighbors (All neighbor table entries)       | %d\n", numNeighborsTotal);
    fprintf(statisticsCollector,"| Number of total messages (Hello + HLMAC)                         | %d\n", numHLMACSentTotal + numHelloSentTotal);
    fprintf(statisticsCollector,"| Number of total sent HLMAC                                       | %d\n", numHLMACSentTotal);
    fprintf(statisticsCollector,"| Number of total sent Hello                                       | %d\n", numHelloSentTotal);
    fprintf(statisticsCollector,"| Number of total disjoint to Tree                                 | %d\n", numNotJoinedTotal);
    fprintf(statisticsCollector,"| Number of total without any neighbor                             | %d\n", numWithoutNeighborTotal);
    fprintf(statisticsCollector,"|__________________________________________________________________|__________________\n");
    fclose(statisticsCollector);

    //Main metrics
    statisticsCollector = fopen("02_ConvergenceTime.txt","a");
    fprintf(statisticsCollector,"%f\n", convergenceTimeEnd.dbl() - convergenceTimeStart.dbl());
    fclose(statisticsCollector);

    statisticsCollector = fopen("03_LimitedEntriesTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numAllowedNeighborsTotal + numHLMACAssignedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("04_MessagesTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numHLMACSentTotal + numHelloSentTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("05_HopCount.txt","a+t");
    fprintf(statisticsCollector,"%d\n", 0);
    fclose(statisticsCollector);

    //Other metrics
    statisticsCollector = fopen("06_AllEntriesTotal_2.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numNeighborsTotal + numHLMACAssignedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("07_HLMACAssignedTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numHLMACAssignedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("08_NeighborsTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numAllowedNeighborsTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("09_NeighborsTotal_2.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numNeighborsTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("10_HLMACSentTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numHLMACSentTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("11_HelloSentTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numHelloSentTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("12_NumNotJoinedTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numNotJoinedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("13_NumWithoutNeighborTotal.txt","a+t");
    fprintf(statisticsCollector,"%d\n", numWithoutNeighborTotal);
    fclose(statisticsCollector);
}

StatisticCollector::~StatisticCollector()
{
}

} // namespace iotorii

