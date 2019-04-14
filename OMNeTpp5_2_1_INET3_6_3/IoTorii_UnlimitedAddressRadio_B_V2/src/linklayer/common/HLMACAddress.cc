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

//Used for hopCount metric
/* To find common ancestor between two Addresses*/
HLMACAddress HLMACAddress::getLongestCommonPrefix(const HLMACAddress &other)
{
    //EV << "-->HLMACAddress::getLongestCommonPrefix()" << endl;

  unsigned int minLen = (getHLMACLength() < other.getHLMACLength()) ? getHLMACLength() : other.getHLMACLength();
  unsigned int i = 0;
  //EV << "minLen = " << minLen << " getHLMACLength = " << getHLMACLength() << ", other.getHLMACLength() = " << other.getHLMACLength() << endl;

  /*
   *  We dont use "HLMACAddress commonPrexif = HLMACAddress::UNSPECIFIED_ADDRESS;"
   *  because HLMACLength is 8 for HLMACAddress::UNSPECIFIED_ADDRESS!
   */
  HLMACAddress commonPrexif;

  while((i < minLen) && (getIndexValue(i) == other.getIndexValue(i)))
  {
      if (i == 0)
          commonPrexif.setIndexValue(i, getIndexValue(i));
      else
          commonPrexif.addNewId(getIndexValue(i));
      i++;
  }
  //EV << " commonPrexif = " << commonPrexif << endl;
  if (i > 0){
      //EV << "-->HLMACAddress::getLongestCommonPrefix(1)" << endl;
      return commonPrexif;
  }else{
      //EV << "-->HLMACAddress::getLongestCommonPrefix(2)" << endl;
      return HLMACAddress::UNSPECIFIED_ADDRESS;
  }
}

//Used for hopCount metric
int HLMACAddress::numHopsBetweenAddresses(const HLMACAddress &other)
{
    //EV << "-->HLMACAddress::numHopsBetweenAddresses()" << endl;

    HLMACAddress ancestor = getLongestCommonPrefix(other);
    unsigned int ancestorLen = ancestor.getHLMACLength();
    if ((ancestorLen == 0) || (ancestor == HLMACAddress::UNSPECIFIED_ADDRESS))
        //return -1;
        throw cRuntimeError("HLMACAddress::numHopsBetweenAddresses(): (ancestorLen == 0) || (ancestor == HLMACAddress::UNSPECIFIED_ADDRESS)!");

    //EV << "ancestor = " << ancestor << ", ancestorLen = " << ancestorLen << endl;
    //return src.len - ancestor.len + dst.len - ancestor_len;
    return getHLMACLength() + other.getHLMACLength() - 2 * ancestorLen;
}

} // namespace iotorii


