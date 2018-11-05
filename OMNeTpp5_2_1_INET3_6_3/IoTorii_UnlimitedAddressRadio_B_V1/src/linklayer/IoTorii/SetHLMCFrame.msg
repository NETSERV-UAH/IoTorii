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

//
// TODO generated message class
//
cplusplus {{
    #include "inet/linklayer/common/MACAddress.h"
    #include "src/linklayer/common/HLMACAddress.h"

   typedef struct {
       inet::MACAddress MAC;
       unsigned int suffix;
   } TargetOption;
    
    
}}

namespace iotorii;
class noncobject TargetOption;
class noncobject HLMACAddress;

packet SetHLMACFrame
{
    HLMACAddress prefix;
    TargetOption targetOptions[];
}