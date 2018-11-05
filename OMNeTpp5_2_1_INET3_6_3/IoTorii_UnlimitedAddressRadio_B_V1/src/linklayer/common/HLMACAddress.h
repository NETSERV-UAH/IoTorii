/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran Polytechnic), Iran.
 *                    OMNeT++ 5.2.1 & INET 3.6.3
 * This class implements unlimited HLMAC address in terms of Length. The range of Width is 0~255.
 *
 *
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

#ifndef IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_
#define IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_

#define HLMAC_WIDTH 8

#include <vector>
//#include <iterator>
#include "inet/common/INETDefs.h"

namespace iotorii {
using namespace inet;

class HLMACAddress {

private:
    std::vector<unsigned int> address;

public:

  /** The unspecified HLMAC address, 0.0.0.0.0.0.0.0 */
  static const HLMACAddress UNSPECIFIED_ADDRESS;

  /** The broadcast HLMAC address, 255.255.255.255.255.255.255.255 or FF.FF.FF.FF.FF.FF.FF.FF */
  static const HLMACAddress BROADCAST_ADDRESS;

  /**
   * Default constructor initializes address bytes to zero.
   */
  HLMACAddress() { address = {0}; }  //{ address = {0,0,0,0,0,0,0,0}; }

  /**
    * copy constructor
    */
  HLMACAddress(const HLMACAddress& other) { address = other.address; }

  explicit HLMACAddress(unsigned int array[])  {  address.insert( address.begin(), array, array + sizeof array);}  //{  address.insert( address.begin(), array, array + sizeof array / sizeof array[0]);} //e.g. HLMACAddress::BROADCAST_ADDRESS({255.255.255.255.255.255.255.255})

  /**
   * Returns the address width.
   */
  unsigned int getHLMACWidth() const { return HLMAC_WIDTH; }

  /**
   * Returns the address width.
   */
  unsigned int getHLMACLength() const { return address.size(); }

    /**
   * Returns the value of index.
   */
  unsigned int getIndexValue(unsigned int k) const {return address.at(k);}

  /**
   * Converts address to a string.
   */
  std::string str() const;

  /**
   * Sets the kth index of the address.
   */
  void setIndexValue(unsigned int k, unsigned int addrbyte){address.at(k) = addrbyte;}

  /* add new suffix to frame. note that this method cannot insert new core.
   * for insertion new core, we can directly use setIndexValue(0, core) or setCore()
   */
  void addNewId(unsigned int newPortId) {address.push_back(newPortId);}

  void removeLastId() {address.pop_back(); }

  void setCore(unsigned int newCoreId) {address.at(0) = newCoreId;}

  //unsigned short int getHLMACHier();
  /**
   * Assignment.
   */
  HLMACAddress& operator=(const HLMACAddress& other) { address = other.address; return *this; }

  /**
   * Returns -1, 0 or 1 as result of comparison of 2 addresses.
   */
  int compareTo(const HLMACAddress& other) const;

  /**
   * Returns true if the two addresses are equal.
   */
  bool equals(const HLMACAddress& other) const { return (compareTo(other) == 0) ? true : false;}

   /**
   * Returns true if the two addresses are equal.
   */
  bool operator==(const HLMACAddress& other) const { return (compareTo(other) == 0) ? true : false; }

  /**
   * Returns true if the two addresses are not equal.
   */
  bool operator!=(const HLMACAddress& other) const { return (compareTo(other) != 0) ? true : false; }

  bool operator<(const HLMACAddress& other) const { return (compareTo(other) < 0) ? true : false; }

  bool operator<=(const HLMACAddress& other) const { return (compareTo(other)  <= 0) ? true : false;}

  bool operator>(const HLMACAddress& other) const { return (compareTo(other)  > 0) ? true : false; }

  bool operator>=(const HLMACAddress& other) const {  return (compareTo(other)  >= 0) ? true : false; }


};
inline std::ostream& operator<<(std::ostream& os, const HLMACAddress& hlmac)
{
    return os << hlmac.str();
}

}

#endif /* IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_ */
