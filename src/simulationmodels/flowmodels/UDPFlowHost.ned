//
// Copyright (C) 2012 Elisa Rojas 
//
//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    INET 3.6.3 adaptation, also adapted for using in the wARP-PATH protocol
//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    INET 3.6.3 adaptation, also adapted for using in the IoTorii(WSN) protocol
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

package iotorii.src.simulationmodels.flowmodels;

// This module is used by the UDPFlowGenerator, so there should be a generator module in the topology.
// Once the generator creates a flow, it passes its parameters to the UDPFlowHost at the source host.
// This module starts and ends the flow and the traffic generated is based on UDP packets.

import inet.applications.contract.IUDPApp;

simple UDPFlowHost like IUDPApp
{
    parameters:
        @class(iotorii::UDPFlowHost);
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int typeOfService = default(-1); // if not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        string multicastInterface = default("");  // if not empty, set the multicast output interface option on the socket (interface name expected) 
 		bool receiveBroadcast = default(false); // if true, makes the socket receive broadcast packets
        bool joinLocalMulticastGroups = default(false); // if true, makes the socket receive packets from all multicast groups set on local interfaces
        @display("i=block/app");
    gates:
        input udpIn;
        output udpOut;
}
