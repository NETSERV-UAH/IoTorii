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

namespace iotorii {
using namespace inet;

enum eGA3FrameType
{
    Unspecified = 0,   //Zero value is reserved for unspecified address
    DataHLMAC = 0,      //Data frame
    SetHLMAC = 1,      //SetHLMAC frame
    Broadcast = 255      //the bigest value is reserved for broadcast address
};

class eGA3Frame // : public HLMACAddress
{
     private:
      unsigned int type;
      HLMACAddress data;

     public:
//constructors
       /**
        * Default constructor initializes data bits to zero.
        */
       eGA3Frame() { type = Unspecified; data = HLMACAddress();}


       /**
        * Constructor which accepts two hex string (1hex digits for eGA3FrameType and
        * 7 hex digits for HLMACAddress, may also
        * contain dots, spaces, hyphens and colons)
        */
       eGA3Frame(unsigned int type, HLMACAddress hlmacaddress ) {this->type = type; data = hlmacaddress;}


       /**
        * Copy constructor.
        */
       eGA3Frame(const eGA3Frame& other) { data = other.data; type = other.type;}



//getter methods

       /**
        * Returns the kth byte of the data.
        */
       unsigned int getIndexValue(unsigned int k) const { return data.getIndexValue(k);}

       /**
        * Returns the HLMAC Address of the data.
        */
       HLMACAddress getHLMACAddress(){return data;}

       /**
        * Returns the eGA3FrameType of the data.
        */
       unsigned int geteGA3FrameType() const { return type; }


//setter methods
       /**
        * Sets the kth byte of the data.
        */
       void setIndexValue(unsigned int k, unsigned int addrbyte) { data.setIndexValue(k, addrbyte);}

       /**
        * Sets the kth byte of the data.
        */
       void setHLMACAddress(HLMACAddress addr) {data = addr;}

       /**
        * Sets the kth byte of the data.
        */
       void seteGA3FrameType(unsigned int type) { this->type = type;}

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
