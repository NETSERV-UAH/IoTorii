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

#ifndef IOTORII_SRC_SIMULATIONMODELS_STATISTIC_STATISTICCOLLECTOR_H_
#define IOTORII_SRC_SIMULATIONMODELS_STATISTIC_STATISTICCOLLECTOR_H_

#include "inet/common/INETDefs.h"

namespace iotorii {
using namespace inet;

// Measures and records network goodput, end to end delay

class StatisticCollector
{
  protected:
    simtime_t startTime;    // start time

    // statistic parameters
    unsigned long long numSent; //Packets
    unsigned long long numReceived;  //Packets
    unsigned long long numSentInbyte;
    unsigned long long numReceivedInbyte;

    double averageEndToEndDelay;
    simtime_t sumDelay;
    simtime_t lastDelay;


    // statistic vectors
    cOutVector endToEndDelayVector; // this vector saves end to end delay of each packet. it records last delayes.
    cOutVector averageEndToEndDelayVector; // this vector saves average of end to end delay for all packets

  protected:
    StatisticCollector();
    virtual void updateSentStats(unsigned int frameSize);
    virtual void updateReceivedStats(simtime_t now, cPacket *pk);
    virtual void updateSentStatsFlowGenerator(unsigned int numPackets, unsigned int numBytes);
    virtual void updateReceivedStatsFlowGenerator(simtime_t now, simtime_t lastAverageDelay, unsigned int numPackets, unsigned int numBytes);
    virtual void finish();

    ~StatisticCollector();
};

} // namespace iotorii

#endif // ifndef IOTORII_SRC_SIMULATIONMODELS_STATISTIC_STATISTICCOLLECTOR_H_

