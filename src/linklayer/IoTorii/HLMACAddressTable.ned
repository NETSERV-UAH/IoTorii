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

import iotorii.src.linklayer.contract.IHLMACAddressTable;

//
// This module handles the mapping between ports and MAC addresses.
//
// Entries are deleted if their age exceeds a certain limit.
//
// If needed, address tables can be pre-loaded from text files at the beginning
// of the simulation; this controlled by the addressTableFile module parameter.
//
simple HLMACAddressTable like IHLMACAddressTable
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit("s") = default(120s);
        @class(iotorii::HLMACAddressTable);
        
}
