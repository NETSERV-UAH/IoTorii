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

package iotorii.simulations.test3_IgnoreInterference;

import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
//import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

import iotorii.src.node.inet.WSNHostIoTorii;


network Net802154
{
    parameters:
        int numHosts;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=19,19");
        }
        configurator: FlatNetworkConfigurator6 {
            parameters:
                @display("p=108,19");
        }
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=57,19");
        }
        host[numHosts]: WSNHostIoTorii {
            parameters:
                @display("r=,,#707070;p=146,58");
        }
}
