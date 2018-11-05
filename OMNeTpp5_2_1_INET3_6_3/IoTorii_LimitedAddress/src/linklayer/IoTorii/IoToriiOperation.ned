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

package iotorii.src.linklayer.IoTorii;

simple IoToriiOperation
{
    parameters:
        @class(iotorii::IoToriiOperation);
        string interfaceTableModule;
        int headerLength @unit(bit) = default(72 bit);  //used for Hello & SetHLMAC
        int headerLengthPANID @unit(bit) = default(88 bit);  //used for DATA frames (packets received from upper layer)
        int broadcastType = default(3); //select broadcast type. 1: only Upward by using counter, 2: only Upward by using transmitter address, 3: UP/Downward and P2P traffic
        bool isCoreSwitch = default(false);
        int corePrefix = default(-1);
        int maxHLMACs = default(10);  //maximum number of HLMAC table size. -1 means "unlimited" size
        double helloStartTime @unit("s") = default(1s);
        double helloInterval @unit("s") = default(10s); //"Hello" interval time, every helloInterval seconds a node broadcasts Hello messages
        double coreStartTime @unit("s") = default(2s);  // Core is the Sink node
        double coreInterval @unit("s") = default(10s); 
        
        // RFC 5148:
        double maxPeriodicJitter @unit("s") = default(helloInterval / 4); // it MUST NOT be negative; it MUST NOT be greater than MESSAGE_INTERVAL/2; it SHOULD NOT be greater than MESSAGE_INTERVAL/4.
        volatile double periodicJitter @unit("s") = default(uniform(0s, maxPeriodicJitter)); // jitter for externally triggered message generation and message forwarding
              
        string hlmacTablePath = default("^.^.^.hlmacTable"); // The path to the HLMACAddressTable module
        @display("i=block/cogwheel");
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);    
}