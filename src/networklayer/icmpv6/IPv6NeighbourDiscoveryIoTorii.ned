//
// Copyright (C) 2005 Andras Varga
//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    OMNeT++ 5.2.1 & INET 3.6.3
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package iotorii.src.networklayer.icmpv6;   //EXTRA

//
// Implements IPv6 Neighbour Discovery.
//
// An overview of the IPv6 implementation in the INET Framework is
// provided <a href="ipv6overview.html">here</a>.
//
// @see ~IPv6, ~IPv6RoutingTable, ~ICMPv6
//
simple IPv6NeighbourDiscoveryIoTorii
{
    parameters:
        @class(iotorii::IPv6NeighbourDiscoveryIoTorii);  //EXTRA
        string interfaceTableModule;   // The path to the InterfaceTable module
        string icmpv6Module;
        string xmipv6Module;
        string routingTableModule;
        double minIntervalBetweenRAs @unit(s) = default(30ms); //minRtrAdvInterval:  0.03 sec for MIPv6 , declared as parameter to facilitate testing without recompiling (Zarrar 15.07.07)
        double maxIntervalBetweenRAs @unit(s) = default(70ms);  //MaxrtrAdvInterval: 0.07 sec for MIPv6, declared as parameter to facilitate testing without recompiling (Zarrar 15.07.07)
        @display("i=block/network");
        @signal[startDAD](type=long); // emits value=1
        @statistic[startDAD](title="DAD started";record=count,vector);
    gates:
        input ipv6In;
        output ipv6Out;
}
