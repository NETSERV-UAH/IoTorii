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

#ifndef IOTORII_SRC_LINKLAYER_COMMON_EGA3FRAME_H_
#define IOTORII_SRC_LINKLAYER_COMMON_EGA3FRAME_H_

#include <string>
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "HLMACAddress.h"
#include <sys/types.h>

#define FRAME_DATA_SIZE     2 //2 bytes,
#define FRAME_DATA_MASK     0xffffULL
#define FRAME_HLMAC_MASK    0b1111111111111100ULL

namespace iotorii {
using namespace inet;

enum eGA3FrameType
{
    SetHLMAC = 1,      //SetHLMAC frame
    DataHLMAC = 2      //Data frame
};

class eGA3Frame // : public HLMACAddress
{
     private:
      uint64 data;    // 2*8=16 bit data, lowest 2 bytes are used, highest 6 bytes are always zero

     public:
//constructors
       /**
        * Default constructor initializes data bits to zero.
        */
       eGA3Frame() { data = 0; }

       /**
        * Initializes the data from the lower 16 bits of the 64-bit argument
        */
       explicit eGA3Frame(uint64 bits) { data = bits & FRAME_DATA_MASK; }

       /**
        * Constructor which accepts a hex string (8 hex digits, may also
        * contain dots, spaces, hyphens and colons)
        */
      // explicit eGA3Frame(const char *hexstr) { setData(hexstr); }

       /**
        * Constructor which accepts two hex string (1hex digits for eGA3FrameType and
        * 7 hex digits for HLMACAddress, may also
        * contain dots, spaces, hyphens and colons)
        */
       eGA3Frame(unsigned char typehexstr,HLMACAddress hlmacaddress );


       /**
        * Copy constructor.
        */
       eGA3Frame(const eGA3Frame& other) { data = other.data; }

       eGA3Frame(const MACAddress& other) { data = (other.getInt() & FRAME_DATA_MASK); }

//getter methods
       /**
        * Returns the data size in bytes, that is, 2.
        */
       unsigned int getDataSize() const { return FRAME_DATA_SIZE; }

       /**
        * Returns the kth byte of the data.
        */
       unsigned char getIndexValue(unsigned int k) const;

       /**
        * Returns the HLMAC Address of the data.
        */
       HLMACAddress getHLMACAddress();

       /**
        * Returns the eGA3FrameType of the data.
        */
       int geteGA3FrameType() const { return getIndexValue(7); }

       /**
        * Converts data to 48 bits integer.
        */
       uint64 getInt() const { return data; }

//setter methods
       /**
        * Sets the kth byte of the data.
        */
       void setIndexValue(unsigned int k, unsigned char addrbyte);

       /**
        * Sets the kth byte of the data.
        */
       void setHLMACAddress(HLMACAddress addr);

       /**
        * Sets the kth byte of the data.
        */
       void seteGA3FrameType(unsigned char type);

//other methods
       /**
        * Converts data to a hex string.
        */
       std::string str() const;

   };

inline std::ostream& operator<<(std::ostream& os, const eGA3Frame& hlmac)
{
   return os << hlmac.str();
}


}// namespace iotorii
#endif /* IOTORII_SRC_LINKLAYER_COMMON_EGA3FRAME_H_ */
