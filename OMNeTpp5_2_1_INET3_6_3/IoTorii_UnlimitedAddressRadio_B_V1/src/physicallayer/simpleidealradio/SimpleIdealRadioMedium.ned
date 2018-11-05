// this module is inspired from physical layer of INET framework
// Copyright (C) 2013 OpenSim Ltd.
//
//
// Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
//                    (1) GIST, University of Alcala, Spain.
//                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
//                    OMNeT++ 5.2.1 & INET 3.6.3
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

package iotorii.src.physicallayer.simpleidealradio;

//import inet.physicallayer.contract.packetlevel.IPropagation;

simple SimpleIdealRadioMedium
{
    parameters:

        // submodule type parameters
        //string propagationType;                   // NED type of the propagation model


        @class(iotorii::physicallayer::SimpleIdealRadioMedium);
        @display("i=misc/sun_s");

}
