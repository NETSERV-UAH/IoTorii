// this module is inspired from physical layer of INET framework
// Copyright (C) 2013 OpenSim Ltd.
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

package iotorii.src.physicallayer.simpleidealradio;


import inet.physicallayer.contract.packetlevel.IRadio;

simple SimpleIdealRadio like IRadio
{
    parameters:
        string radioMediumModule = default("^.^.^.radioMedium"); // module path of the medium module where this radio communicates
                string mobilityModule = default(".mobility"); // the path to the mobility module relative to the network node
        
        double communicationRange @unit(m) = default(9.6m);  //default(9.6); //meters
        bool displayCommunicationRange = default(false); // if true communication range is displayed as a blue circle around the node      
        @class(iotorii::physicallayer::SimpleIdealRadio);
        @display("i=block/wrxtx, r=9.6"); 
        
        @signal[packetReceivedFromUpper](type=cPacket);
        //@signal[packetReceivedFromLower](type=cPacket);
        //@signal[packetFromUpperDropped](type=cPacket);
        
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input radioIn @labels(IRadioFrame);

   
}
