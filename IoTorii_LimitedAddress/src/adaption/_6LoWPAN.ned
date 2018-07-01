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

package iotorii.src.adaption;

simple _6LoWPAN
{
    parameters:
        @class(iotorii::_6LoWPAN);
        int udpHeaderLength @unit(B) = default(4 B); //UDP compressed header length in Byte
        int dispatchHeaderLength @unit(B) = default(1 B); //Dispatch header length in Byte
        int ipv6HeaderLength @unit(B) = default(2 B); //IPv6 (HC1) compressed header length in Byte
        int icmpHeaderLength @unit(B) = default(8 B); //ICMP compressed header length in Byte, only for ping traffic
        int nsHeaderLength @unit(B) = default(8 B);   //Neighbor Solicitation compressed header length in Byte
        int naHeaderLength @unit(B) = default(8 B);  //Neighbor Advertisement compressed header length in Byte
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        
    gates:
        input netIn @labels(INetworkDatagram);
        output netOut @labels(INetworkDatagram);
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);    
}