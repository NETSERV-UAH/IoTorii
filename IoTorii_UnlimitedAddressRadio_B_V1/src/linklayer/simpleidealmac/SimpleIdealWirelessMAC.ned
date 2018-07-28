
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

package iotorii.src.linklayer.simpleidealmac;

simple SimpleIdealWirelessMAC //extends MACProtocolBase like IMACProtocol  //EXTRA
{
    parameters:
       string interfaceTableModule; //EXTRA this line is added becaus "extends MACProtocolBase like IMACProtocol" is disabled

       string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.

        string radioModule = default("^.^.radio");   // The path to the Radio module  //FIXME remove default value

        // length of MAC header
        int headerLength @unit(bit) = default(72 bit);
        int mtu @unit("B") = default(0B);
        // bit rate
        double bitrate @unit(bps) = default(250000 bps);

        @class(iotorii::SimpleIdealWirelessMAC);                                          // "auto". "auto" values will be replaced by
        
        @signal[packetSentToUpper](type=cPacket); //because of lack of "LayeredProtocolBase"
        @signal[packetReceivedFromUpper](type=cPacket); //because of lack of "LayeredProtocolBase"
        @signal[packetFromUpperDropped](type=cPacket); //because of lack of "LayeredProtocolBase"
        @signal[packetSentToLower](type=cPacket); //because of lack of "LayeredProtocolBase"
        @signal[packetReceivedFromLower](type=cPacket); //because of lack of "LayeredProtocolBase"
        @signal[packetFromLowerDropped](type=cPacket); //because of lack of "LayeredProtocolBase"
        
 //EXTRA BEGIN       
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);        
//EXTRA END          
}
