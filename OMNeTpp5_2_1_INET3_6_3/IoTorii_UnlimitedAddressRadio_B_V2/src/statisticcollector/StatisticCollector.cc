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
    }else if(stage == INITSTAGE_LINK_LAYER)
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
            newWSN.ioToriiOperation = check_and_cast<IoToriiOperation *>(newWSN.host->getSubmodule("wlan", 0)->getSubmodule("mac")->getSubmodule("IoTorii"));
            newWSN.hlmacAddressTable = check_and_cast<IHLMACAddressTable *>(newWSN.host->getSubmodule("hlmacTable"));
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
            nodeStateList.at(i) = (newWSN);
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
    scheduleAt(simulationTimeInterval, simulationEndEvent);
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
    throw cRuntimeError("MAC Address %s is not in the nodeStateList!", address);
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
/*      //Convergence time definition: interval time between the first SetHLMAC generation time (or convergenceTimeStart) and the time when the last node is joining to the tree.
        if (convergenceTimeEnd < time){
            convergenceTimeEnd = time;
        }
*/
        EV << "Node" << nodeID << "(MAC Address: " << nodeStateList.at(nodeID).macAddress << " has a route to Root." << endl;
    }

/*  //Convergence time definition: interval time between the first SetHLMAC generation time (or convergenceTimeStart) and the time of the last assigned SetHLMAC.
    if (convergenceTimeEnd < time){
        convergenceTimeEnd = time;
    }
*/
}

void StatisticCollector::receiveSetHLMACMessage(simtime_t time)
{
    //Convergence time definition: interval time between the first SetHLMAC generation time (or convergenceTimeStart) and the time of the last received SetHLMAC in the network.
    if (convergenceTimeEnd < time){
        convergenceTimeEnd = time;
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
        if (isConverged()){
            collectOtherMetrics();
            saveStatistics();
        }else
            EV << " The network didn't converge!" << endl;
        delete msg;
        endSimulation();
    }else{
        EV << "Unknown self message is deleted." << endl;
        delete msg;
    }

    return;
}

bool StatisticCollector::isConverged()
{
    for (unsigned int i = 0; i < nodeStateList.size(); i++){
        if (!nodeStateList.at(i).isJoined)
            return false;
    }
    return true;
}

void StatisticCollector::calculateHopCount()
{
    int numElements = 0;

    //hopCount = new int[nodeStateList.size()];
    hopCount.resize(nodeStateList.size());
    for (unsigned int i = 0; i < nodeStateList.size(); i++){
        //hopCount[i] = new int[nodeStateList.size()];
        hopCount.at(i).resize(nodeStateList.size());
        for (unsigned int j = 0; j < nodeStateList.size(); j++){
            if (i == j)
                //hopCount[i][j] = 0;
                hopCount.at(i).at(j) = 0;
            else{
                //hopCount[i][j] = findMinHopCount(i, j);
                hopCount.at(i).at(j) = findMinHopCount(i, j);
                if (hopCount.at(i).at(j) == -1)
                    throw cRuntimeError("There is not any route between node %d and %d!", i, j);
                averageNumberofHopCount += hopCount.at(i).at(j);
                numElements++;
            }
        }
    }
    averageNumberofHopCount /= numElements;

/*    for(unsigned int i = 0; i < hopCount.size(); i++){
        delete[] hopCount[i];
        hopCount[i] = nullptr;
    }
    delete[] hopCount;
    hopCount = nullptr;
*/
}

int StatisticCollector::findMinHopCount(unsigned int src_id, unsigned int dst_id)
{
    //EV << "-->StatisticCollector::findMinHopCount()" << endl;
    int minHopCount = -1;
    int numberOfAddressesSrc = nodeStateList.at(src_id).hlmacAddressTable->getNumberOfAddresses();
    int numberOfAddressesDst = nodeStateList.at(dst_id).hlmacAddressTable->getNumberOfAddresses();
    for (unsigned int i = 0; i < numberOfAddressesDst; i++){
        HLMACAddress dst = nodeStateList.at(dst_id).hlmacAddressTable->getAddress(i);
        int newHopCount = nodeStateList.at(src_id).hlmacAddressTable->getMinHopCount(dst);
        //EV << "newHopCount = " << newHopCount << endl;
        if (i == 0)
            minHopCount = newHopCount;
        else if (newHopCount < minHopCount) {
                minHopCount = newHopCount;
        }
    }
    return minHopCount;
}

void StatisticCollector::saveStatistics()
{
    FILE *statisticsCollector;

    //Main metrics
    statisticsCollector = fopen("02_ConvergenceTime.txt","a");
    fprintf(statisticsCollector,"%f\n", convergenceTimeEnd.dbl() - convergenceTimeStart.dbl());
    fclose(statisticsCollector);

    statisticsCollector = fopen("03_averageNumberOfEntries.txt","a");
    fprintf(statisticsCollector,"%f\n", (double) (numAllowedNeighborsTotal + numHLMACAssignedTotal) / nodeStateList.size());
    fclose(statisticsCollector);

    statisticsCollector = fopen("04_averageNumberOfMessages.txt","a");
    fprintf(statisticsCollector,"%f\n", (double)(numHLMACSentTotal + numHelloSentTotal) / nodeStateList.size());
    fclose(statisticsCollector);

    statisticsCollector = fopen("05_HopCount.txt","a");
    fprintf(statisticsCollector,"%f\n", averageNumberofHopCount);
    fclose(statisticsCollector);

    //Other metrics
    statisticsCollector = fopen("06_AllEntriesTotal_2.txt","a");
    fprintf(statisticsCollector,"%d\n", numNeighborsTotal + numHLMACAssignedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("07_HLMACAssignedTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numHLMACAssignedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("08_NeighborsTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numAllowedNeighborsTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("09_NeighborsTotal_2.txt","a");
    fprintf(statisticsCollector,"%d\n", numNeighborsTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("10_HLMACSentTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numHLMACSentTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("11_HelloSentTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numHelloSentTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("12_NumNotJoinedTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numNotJoinedTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("13_NumWithoutNeighborTotal.txt","a");
    fprintf(statisticsCollector,"%d\n", numWithoutNeighborTotal);
    fclose(statisticsCollector);

    statisticsCollector = fopen("14_HLMACAddresses.txt","a");
    for (unsigned int i = 0; i < nodeStateList.size(); i++){
        fprintf(statisticsCollector,"host[%d]: MAC Address = %s, HLMAC Addresses = \t", i, nodeStateList.at(i).macAddress.str().c_str());
        for (unsigned int j = 0; j < nodeStateList.at(i).hlmacAddressTable->getNumberOfAddresses(); j++){
            fprintf(statisticsCollector,"%s\t", nodeStateList.at(i).hlmacAddressTable->getAddress(j).str().c_str());
        }
        fprintf(statisticsCollector,"\n");
    }
    fprintf(statisticsCollector,"\n ---------------------------------------------------- \n");
    fclose(statisticsCollector);

    statisticsCollector = fopen("15_hopCountInfo.txt","a");
    for (unsigned int i = 0; i < hopCount.size(); i++){
        for (unsigned int j = 0; j < hopCount.size(); j++){
            //fprintf(statisticsCollector,"%3d\t", hopCount[i][j]);
            fprintf(statisticsCollector,"%3d\t", hopCount.at(i).at(j));
            if((i != j) && (hopCount.at(i).at(j) != -1)){
            }
        }
        fprintf(statisticsCollector,"\n");
    }
    fprintf(statisticsCollector,"\n ---------------------------------------------------- \n");
    fclose(statisticsCollector);

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
    fprintf(statisticsCollector,"| Number of average Hop Count                                      | %f\n", averageNumberofHopCount);
    fprintf(statisticsCollector,"| Number of total disjoint to Tree                                 | %d\n", numNotJoinedTotal);
    fprintf(statisticsCollector,"| Number of total without any neighbor                             | %d\n", numWithoutNeighborTotal);
    fprintf(statisticsCollector,"|__________________________________________________________________|__________________\n");
    fclose(statisticsCollector);
}

StatisticCollector::~StatisticCollector()
{
}

} // namespace iotorii

