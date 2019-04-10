/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
*/

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "src/linklayer/common/HLMACAddress.h"

#include <ctype.h>

namespace iotorii {

unsigned int unspecifiedArray[8] = {0,0,0,0,0,0,0,0};
const HLMACAddress HLMACAddress::UNSPECIFIED_ADDRESS(unspecifiedArray);
unsigned int broadcastArray[8] = {255,255,255,255,255,255,255,255};
const HLMACAddress HLMACAddress::BROADCAST_ADDRESS(broadcastArray);


std::string HLMACAddress::str() const
{
    char *buf = new char[address.size() * (4 +1) + 1];  //first 1 for dot, second 1 for "\0"
    char *s = buf;
    int i;
    for (i = 0; i < address.size(); i++, s += 5)
        sprintf(s, "%4.4u.", getIndexValue(i));
    *(s) = '\0';
    return std::string(buf);
}

int HLMACAddress::compareTo(const HLMACAddress& other) const
{
    int minSize = (address.size() < other.address.size()) ? address.size() : other.address.size();

    for (int i=0; i<minSize; i++){
        if (address.at(i) > other.address.at(i))
            return +1;
        else if (address.at(i) < other.address.at(i))
            return -1;
    }
    if (address.size() == other.address.size())
        return 0;
    else if (address.size() > other.address.size()){
        for (int i=minSize; i< address.size(); i++){
            if (address.at(i) > 0)
                return +1;
        }
        return 0;
    }
    else if (address.size() < other.address.size()){
        for (int i=minSize; i< other.address.size(); i++){
            if (other.address.at(i) > 0)
                return -1;
        }
        return 0;
    }

}

/*
unsigned short int HLMACAddress::getHLMACHier()
{

}
*/

} // namespace iotorii


