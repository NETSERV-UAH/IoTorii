//
// Copyright (C) 03/2012 Elisa Rojas
//
/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    INET 3.6.3 adaptation adaptation, also adapted for using in the wARP-PATH protocol
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

#include "src/simulationmodels/flowmodels/UDPFlowGenerator.h"

#include <stdio.h>

namespace iotorii {
using namespace inet;

Define_Module(UDPFlowGenerator);

void UDPFlowGenerator::initialize(int stage)
{
    EV << "->FlowGenerator::initialize()" << endl;
    FlowGeneratorBase::initialize(stage);

    EV << "<-FlowGenerator::initialize()" << endl;

}

void UDPFlowGenerator::startRandomFlow()
{
    FlowGeneratorBase::startRandomFlow();

    EV << "->UDPFlowGenerator::startRandomFlow()" << endl;

    std::string flowInfo, smodel; //String to be saved for final statistics about the flow generator
    std::stringstream ss1, ss2, ss3, ss4;

    //We randomly choose a source and a destination for the new flow
    int iSource=0, iDestination=0;
    getRandomSrcDstIndex(iSource, iDestination);
    wSNInfo[iSource].nFlowSource++;
    wSNInfo[iDestination].nFlowDestination++;
    flowInfo = wSNInfo[iSource].fullName + "->" + wSNInfo[iDestination].fullName;

    //Now a transfer rate and flow size,

    double flowSize, transferRate, sessionLength;
    unsigned int frameSize = par("frameSize");
    unsigned long long numPackets = 0;
    if(strcmp(trafficType, "Upward") == 0)  //Upwardtraffic
    {
        /*
         Upward : packet size is 112 Bytes, interval time of data packets is 20 ms
         then 44.800 kbps
         */
        transferRate = frameSize * 8 / interval; //bps
        transferRate = transferRate / 1000; //Kbps
        ss1 << transferRate;
        flowInfo = flowInfo + "; " + ss1.str() + " Kbps";

        /*Upward: packet size is 112 Bytes, Interval time of data packets is 20 ms
         * then, 50 packets per secend
         * */
        smodel = "(IOT -> Upward!)";
        //sessionLength = stopTime - startTime;
        sessionLength = stopTime.dbl() - sessionStartTimeList.at(numSent).dbl();
        sessionLength = (long int) (sessionLength / interval.dbl()) * interval.dbl(); //for last packet, if last interval is not a complete interval.

        flowSize = sessionLength * transferRate / 8 ; //Kilo Bytes
        flowSize *= 1000; //Bytes
        //Finally, we decide the frames size
        numPackets = flowSize / frameSize;
    }
    else
        throw cRuntimeError("Type of traffic is undefined");

    wSNInfo[iSource].averageSizeSource = (wSNInfo[iSource].averageSizeSource*(wSNInfo[iSource].nFlowSource-1)+flowSize)/wSNInfo[iSource].nFlowSource;
    wSNInfo[iDestination].averageSizeDestination = (wSNInfo[iDestination].averageSizeDestination*(wSNInfo[iDestination].nFlowDestination-1)+flowSize)/wSNInfo[iDestination].nFlowDestination;
    ss2 << flowSize;
    flowInfo = flowInfo + "; " + ss2.str() + " B";

    EV << "  Flow info created! " << smodel << endl;
    EV << "    [" << wSNInfo[iSource].fullName << " (" << wSNInfo[iSource].ipAddress << ")" << " -> " << wSNInfo[iDestination].fullName << " (" << wSNInfo[iDestination].ipAddress << ")]" << endl;
    EV << "    Transfer Rate: " << transferRate << " (Kbps); Flow Size: " << flowSize << " (B); Frame Size: " << frameSize << " (B)" << endl;

    //The generator indicates the parameters to the source and it's this host that will start and stop the flow
    //wSNInfo[iSource].pUdpFlowHost->startFlow(transferRate, flowSize*1000, frameSize, wSNInfo[iDestination].ipAddress); //Kbps, B(KB*1000), B, address
    wSNInfo[iSource].pUdpFlowHost->startFlow(transferRate, flowSize, frameSize, wSNInfo[iDestination].ipAddress, wSNInfo[iSource].ipAddress); //Kbps, B, B, dst address, local address
    numSent++;

    int n = numSent; ss3 << n;
    simtime_t genTime = simTime();
    ss4 << sessionLength + genTime;
    flowInfo = ss3.str() + " - " + flowInfo + "; start at t = " + genTime.str() + " s; end at t = " + ss4.str();
    generatedFlows.push_back(flowInfo);

    EV << "<-UDPFlowGenerator::startRandomFlow()" << endl;
}

} //namespace wapb
