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

#include "src/simulationmodels/statistic/StatisticCollector.h"
#include "src/simulationmodels/flowmodels/UDPFlowGenerator.h"


namespace iotorii {
using namespace inet;

StatisticCollector::StatisticCollector()
{
    startTime = 0.2; // FIXME: this is according to flow generator

    numSent = 0;  //packet
    numReceived = 0;  //packet

    numSentInbyte = 0;
    numReceivedInbyte = 0;
    averageEndToEndDelay = 0;
    sumDelay = 0;
    lastDelay = 0;
    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numSentInbyte);
    WATCH(numReceivedInbyte);
    WATCH(averageEndToEndDelay);
    WATCH(sumDelay);
    WATCH(lastDelay);

    endToEndDelayVector.setName("endToEnd (sec)");
    averageEndToEndDelayVector.setName("endToEnd-Avrg (sec)");
}

//update statistics when a packet is sent by UDPFlowHost
void StatisticCollector::updateSentStats(unsigned int frameSize)
{
    numSent++;

    numSentInbyte += (long)(frameSize);
    updateSentStatsFlowGenerator(1, frameSize);

}

//update statistics when a packet is received by UDPFlowHost
void StatisticCollector::updateReceivedStats(simtime_t now, cPacket *pk)
{
    numReceived++;

    lastDelay = now - pk->getCreationTime();
    sumDelay += lastDelay;
    averageEndToEndDelay = sumDelay.dbl() / numReceived;
    numReceivedInbyte += pk->getByteLength(); //Maybe all packets are not the same size

    averageEndToEndDelayVector.record(averageEndToEndDelay);
    endToEndDelayVector.record(lastDelay.dbl());

    updateReceivedStatsFlowGenerator(now, lastDelay, 1, pk->getByteLength());
}


void StatisticCollector::updateSentStatsFlowGenerator(unsigned int numPackets, unsigned int numBytes)
{
    UDPFlowGenerator *pUDPFlowGenerator = check_and_cast<UDPFlowGenerator *>(getSimulation()->getSystemModule()->getSubmodule("generator"));
    pUDPFlowGenerator->accumulateSentData(numPackets, numBytes);


}

void StatisticCollector::updateReceivedStatsFlowGenerator(simtime_t now, simtime_t lastAverageDelay, unsigned int numPackets, unsigned int numBytes)
{
    UDPFlowGenerator *pUDPFlowGenerator = check_and_cast<UDPFlowGenerator *>(getSimulation()->getSystemModule()->getSubmodule("generator"));
    pUDPFlowGenerator->accumulateReceivedData(now, lastAverageDelay, numPackets, numBytes);
}

void StatisticCollector::finish()
{
/*
    recordScalar("total sent packets", numSent);
    recordScalar("total received packets", numReceived);
    recordScalar("total sent bytes", numSentInbyte);
    recordScalar("numReceivedInbyte", numReceivedInbyte);
    recordScalar("average EndToEndDelay", averagEndToEndDelay.dbl());

    //recordScalar("sumDelay", sumDelay);
    //recordScalar("lastDelay", lastDelay);
*/
}

StatisticCollector::~StatisticCollector()
{

}

} // namespace iotorii

