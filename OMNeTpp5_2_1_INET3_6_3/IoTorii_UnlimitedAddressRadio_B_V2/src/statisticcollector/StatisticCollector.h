/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), Carles Gomez(3);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    (3) UPC, Castelldefels, Spain.
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

#ifndef IOTORII_SRC_STATISTIC_STATISTICCOLLECTOR_H
#define IOTORII_SRC_STATISTIC_STATISTICCOLLECTOR_H

#include "inet/common/INETDefs.h"
#include "src/linklayer/common/HLMACAddress.h"
#include "src/linklayer/IoTorii/IoToriiOperation.h"
#include "src/linklayer/IoTorii/IHLMACAddressTable.h"


namespace iotorii {
using namespace inet;

class StatisticCollector : public cSimpleModule
{

    simtime_t simulationTimeInterval;
    cMessage *simulationEndEvent;

    struct NodeState{
        std::string fullName;
        int moduleIndex;
        cModule *host;
        IoToriiOperation *ioToriiOperation;
        IHLMACAddressTable *hlmacAddressTable;
        MACAddress macAddress;
        //HLMACAddress hlmacAddress;

        long hlmacLenIsLow;
        long hlmacWidthIsLow; //if numNeighbors > maxNeighbors, hlmacWidthIsLow++.
        //long hlmacAffectedByWidthIsLow;
        long numHelloRcvd;                 //Number of Hello messages received
        long numHelloSent;
        long numAllowedNeighbors;  //saves neighbors as long as width of HLMAC address (i.e. 3 for 3-bit width)
        long numNeighbors; //number of neighbors discovered by Hello message
        long numHLMACRcvd;
        long numHLMACAssigned;
        long numHLMACLoopable;  //or num discarded HLMAC
        long numHLMACSent;
        long numDiscardedNoHLMAC; //if node has not HLMAC address, by dropping each packet, this variable is incremented.

        long numReceivedLowerPacket;
        long numReceivedUpperPacket;
        long numDiscardedFrames;
        long numRoutedUnicastFrames;
        long numRoutedBroadcastFrames;
        long numDiscardedUnicastFrames;
        long numDiscardedBroadcastFrames;

        bool isJoined;
        simtime_t joiningTime;

        NodeState()
            : host(nullptr)
            , moduleIndex(-1)
            , ioToriiOperation(nullptr)
            , hlmacAddressTable(nullptr)
            , macAddress(MACAddress::UNSPECIFIED_ADDRESS)
            //, hlmacAddress(HLMACAddress::UNSPECIFIED_ADDRESS)
            , hlmacLenIsLow(false)
            , hlmacWidthIsLow(false) //if numNeighbors > maxNeighbors, hlmacWidthIsLow++.
            //, hlmacAffectedByWidthIsLow;
            , numHelloRcvd(0)                 //Number of Hello messages received
            , numHelloSent(0)
            , numAllowedNeighbors(0)
            , numNeighbors(0) //number of neighbors discovered by Hello message
            , numHLMACRcvd(0)
            , numHLMACAssigned(0)
            , numHLMACLoopable(0)  //or num discarded HLMAC
            , numHLMACSent(0)
            , numDiscardedNoHLMAC(0) //if node has not HLMAC address, by dropping each packet, this variable is incremented.
            , numReceivedLowerPacket(0)
            , numReceivedUpperPacket(0)
            , numDiscardedFrames(0)
            , numRoutedUnicastFrames(0)
            , numRoutedBroadcastFrames(0)
            , numDiscardedUnicastFrames(0)
            , numDiscardedBroadcastFrames(0)
            , isJoined(false)
            , joiningTime(SIMTIME_ZERO)
            {};

    };

    typedef std::vector<struct NodeState> NodeStateList;
    NodeStateList nodeStateList;

    //int version;
    unsigned int sinkID;
    //IPv6Address sinkLLAddress;

    //Global statistics
    simtime_t convergenceTimeStart;  //this is set by core start time
    simtime_t convergenceTimeEnd;   //interval time between the first SetHLMAC generation time (or convergenceTimeStart) and the time of the last received (AND ASSIGNED) SetHLMAC in the network.
    int numHLMACAssignedTotal;
    int numNeighborsTotal;  //saves all available neighbors
    int numAllowedNeighborsTotal;  //saves neighbors as long as width of HLMAC address (i.e. 3 for 3-bit width)
    int numHelloSentTotal;
    int numHLMACSentTotal;
    int numNotJoinedTotal;   //The number of nodes which are not joined to tree
    int numWithoutNeighborTotal;

    //int **hopCount;
    std::vector<std::vector <int>> hopCount;
    float averageNumberofHopCount;

public:
    StatisticCollector()
        : simulationEndEvent(nullptr)
        , simulationTimeInterval(SIMTIME_ZERO)
        , sinkID(-1)
        , convergenceTimeStart(SIMTIME_ZERO)
        , convergenceTimeEnd(SIMTIME_ZERO)
        , numHLMACAssignedTotal(0)
        , numNeighborsTotal(0)  //saves all available neighbors
        , numAllowedNeighborsTotal(0)  //saves neighbors as long as width of HLMAC address (i.e. 3 for 3-bit width)
        , numHelloSentTotal(0)
        , numHLMACSentTotal(0)
        , numNotJoinedTotal(0)   //The number of nodes which are not joined to tree
        , numWithoutNeighborTotal(0)
        , averageNumberofHopCount(0)
            {};

    ~StatisticCollector();

protected:
    virtual int numInitStages() const  {return NUM_INIT_STAGES;}

    virtual void initialize(int stage);

    virtual void extractTopology();

    virtual void saveStatistics();

    virtual void handleMessage(cMessage* msg);

    virtual bool isConverged();

    virtual void collectOtherMetrics();

    virtual void calculateHopCount();

    virtual int findMinHopCount(unsigned int src_id, unsigned int dst_id);

public:
    virtual void startStatistics(const MACAddress &sinkAddress, simtime_t time);

    virtual unsigned int getIndexFromMACAddress(const MACAddress &address);

    virtual void nodeJoined(int nodeID, simtime_t time);

    virtual void nodeJoined(const MACAddress &address, simtime_t time);

    virtual void receiveSetHLMACMessage(simtime_t time);

};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_STATISTIC_STATISTICCOLLECTOR_H

