//
// Copyright (C) 2017 Elisa Rojas(1), SeyedHedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//
// Main paper:
// Rojas, Elisa, et al. "GA3: scalable, distributed address assignment
// for dynamic data center networks." Annals of Telecommunications (2017): 1-10.þ
// DOI: http://dx.doi.org/10.1007/s12243-017-0569-4
//
// Developed in OMNet++5.2, based on INET framework.
// LAST UPDATE OF THE INET FRAMEWORK: inet3.6.2 @ October 2017
//
//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    OMNeT++ 5.2.1 & INET 3.6.3
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

package iotorii.src.linklayer.GA3switch;

import inet.linklayer.contract.IMACRelayUnit;

simple MACRelayUnitGA3 like IMACRelayUnit
{
    parameters:
        @class(iotorii::MACRelayUnitGA3);
        //int broadcastSeed = default(0);
        bool isCoreSwitch = default(false);
        int corePrefix = default(-1);
        double timeBetweenPorts @unit("s") = default(isCoreSwitch ? 0.000001s : 0s);
        double timeBetweenCores @unit("s") = default(isCoreSwitch ? 0.000008s : 0s);   
        double helloInterval @unit("s") = default(1s); //"Hello" interval time, every helloInterval seconds a node broadcasts Hello messages
        // RFC 5148:
        double maxPeriodicJitter @unit("s") = default(helloInterval / 4); // it MUST NOT be negative; it MUST NOT be greater than MESSAGE_INTERVAL/2; it SHOULD NOT be greater than MESSAGE_INTERVAL/4.
        volatile double periodicJitter @unit("s") = default(uniform(0s, maxPeriodicJitter)); // jitter for externally triggered message generation and message forwarding
        
             
        //string macTablePath = default("^.macTable"); // The path to the MACAddressTable module
        string hlmacTablePath = default("^.hlmacTable"); // The path to the HLMACAddressTable module
        @display("i=block/switch");
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
    gates:
        input ifIn[] @labels(EtherFrame);
        output ifOut[] @labels(EtherFrame);
        input stpIn @loose;
        output stpOut @loose;
}