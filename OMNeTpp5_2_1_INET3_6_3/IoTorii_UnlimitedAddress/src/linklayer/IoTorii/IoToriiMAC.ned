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

import iotorii.src.linklayer.IoTorii.IoToriiOperation;
import iotorii.src.linklayer.ieee802154.Ieee802154NarrowbandMacNew;

//
// TODO auto-generated type
//
module IoToriiMAC
{
    parameters:
        @display("i=block/layer");
    
   gates:
        input upperLayerIn @labels(INetworkDatagram/down);
        output upperLayerOut @labels(INetworkDatagram/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
    
    submodules:
        IoTorii: IoToriiOperation { //<relayUnitType> like IMACRelayUnit {
            parameters:
                @display("p=100,75");
        }
        mac802154: Ieee802154NarrowbandMacNew {  //EXTRA
            parameters:
                @display("p=100,150");
        }
        
 connections:
        IoTorii.lowerLayerOut --> mac802154.upperLayerIn;
        IoTorii.lowerLayerIn <-- mac802154.upperLayerOut;

        IoTorii.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
        IoTorii.upperLayerIn <-- { @display("m=n"); } <-- upperLayerIn;
        mac802154.lowerLayerIn <-- { @display("m=s"); } <-- lowerLayerIn;
        mac802154.lowerLayerOut --> { @display("m=s"); } --> lowerLayerOut;
         
}
