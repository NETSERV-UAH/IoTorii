/*
 * The implementation of eGA3 frame is inspired by the MAC address implementation.
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
*/

/*
 * Copyright (C) 2017 Elisa Rojas(1), SeyedHedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *
 * Main paper:
 * Rojas, Elisa, et al. "GA3: scalable, distributed address assignment
 * for dynamic data center networks." Annals of Telecommunications (2017): 1-10.þ
 * DOI: http://dx.doi.org/10.1007/s12243-017-0569-4
 *
 * Developed in OMNet++5.2, based on INET framework.
 * LAST UPDATE OF THE INET FRAMEWORK: inet3.6.2 @ October 2017
*/

/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
*/

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

#include "eGA3Frame.h"
#include "HLMACAddress.h"
#include <ctype.h>

namespace iotorii {
using namespace inet;


std::string eGA3Frame::str() const
{
    char *buf = new char[(data.getHLMACLength() + 1) * (4 + 1) + 3];  //first 1 for type, second 1 for dot & dash, 3 for parentheses & \0
    char *s = buf;
    int i;
    sprintf(s, "(%4.4u-", type);
    s += 6;
    for (i = 0; i < data.getHLMACLength(); i++, s += 5)
        sprintf(s, "%4.4u.", getIndexValue(i));
    sprintf(s++, ")");
    *(s) = '\0';
    return std::string(buf);
}

} // namespace iottorii

