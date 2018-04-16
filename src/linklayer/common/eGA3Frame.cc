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
#include <ctype.h>

namespace iotorii {
using namespace inet;


//constructors
 eGA3Frame::eGA3Frame(unsigned char type, HLMACAddress address )
{
     setHLMACAddress(address);
     seteGA3FrameType(type);
}


//getter methods
unsigned char eGA3Frame::getDataByte(unsigned int k) const
{
    if (k >= FRAME_DATA_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (FRAME_DATA_SIZE - k - 1) * 8;
    return 0xff & (data >> offset);
}

void eGA3Frame::getDataBytes(unsigned char *addrbytes) const
{
    for (int i = 0; i < FRAME_DATA_SIZE; i++)
        addrbytes[i] = getDataByte(i);
}


HLMACAddress eGA3Frame::getHLMACAddress()
{
    return HLMACAddress(data & FRAME_HLMAC_MASK);
}

//setter methods
void eGA3Frame::setDataByte(unsigned int k, unsigned char addrbyte)
{
    if (k >= FRAME_DATA_SIZE)
        throw cRuntimeError("Array of size 6 indexed with %d", k);
    int offset = (FRAME_DATA_SIZE - k - 1) * 8;
    data = (data & (~(((uint64)0xff) << offset))) | (((uint64)addrbyte) << offset);
}

void eGA3Frame::setDataBytes(unsigned char *addrbytes)
{
    data = 0;    // clear top 16 bits too that setDataByte() calls skip
    for (int i = 0; i < FRAME_DATA_SIZE; i++)
        setDataByte(i, addrbytes[i]);
}


void eGA3Frame::seteGA3FrameType(unsigned char type)
{
    if (type > 15)
        throw cRuntimeError("eGA3FrameType is not in range, it must be at most 4 bits.", type);
    data = data | type; //setDataByte(5, type);
}

void eGA3Frame::setHLMACAddress(HLMACAddress addr)
{
    char addrbytes[6];
    addr.getAddressBytes(addrbytes);
    setDataBytes(addrbytes);
}

//other methods

/* add new port id to frame. note that this method cannot insert new core.
 * for insertion new core, we can use setDataByte(0, core) directly or setCore()
 */
void eGA3Frame::addNewId(unsigned char newPortId)
{
  HLMACAddress hlmac = getHLMACAddress();
  unsigned char type = geteGA3FrameType(); // data is reseted by setHLMACAddress()
  int index = hlmac.getHLMACHier();

  if ((index < FRAME_DATA_SIZE - 1) && (index >= 0))  // HLMACAddress class checks other error checking and handling
  {
      hlmac.setAddressByte(index + 1, newPortId);
      setHLMACAddress(hlmac);
      seteGA3FrameType(type);
  }else
      throw cRuntimeError("in eGA3Frame::addNewId, index is out of range");


  //setHLMACAddress(getHLMACAddress().setAddressByte(this->getHLMACAddress().getHier()+1, newPortId));
  return;
}

void eGA3Frame::setCore(unsigned char newCoreId)
{
    setDataByte(0, newCoreId);
}

bool eGA3Frame::tryParse(const char *hexstr)
{
    if (!hexstr)
        return false;

    // check syntax
    int numHexDigits = 0;
    for (const char *s = hexstr; *s; s++) {
        if (isxdigit(*s))
            numHexDigits++;
        else if (*s != ' ' && *s != ':' && *s != '-' && *s != '.' && *s != ',')
            return false; // wrong syntax
    }
    if (numHexDigits != 2 * FRAME_DATA_SIZE)
        return false;

    // Converts hex string into the data
    // if hext string is shorter, data is filled with zeros;
    // Non-hex characters are discarded before conversion.
    data = 0;    // clear top 16 bits too that setDataByte() calls skip
    int k = 0;
    const char *s = hexstr;
    for (int pos = 0; pos < FRAME_DATA_SIZE; pos++) {
        if (!s || !*s) {
            setDataByte(pos, 0);
        }
        else {
            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setDataByte(pos, 0);
                continue;
            }
            unsigned char d = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            d = d << 4;
            s++;

            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setDataByte(pos, 0);
                continue;
            }
            d += isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            s++;

            setDataByte(pos, d);
            k++;
        }
    }
    return true;
}


std::string eGA3Frame::str() const
{
    char buf[20];
    char *s = buf;
    for (int i = 0; i < FRAME_DATA_SIZE; i++, s += 3)
        sprintf(s, "%2.2X.", getDataByte(i));
    *(s - 1) = '\0';
    return std::string(buf);
}

int eGA3Frame::compareTo(const eGA3Frame& other) const
{
    return (data < other.data) ? -1 : (data == other.data) ? 0 : 1;    // note: "return data-other.data" is not OK because 64-bit result does not fit into the return type
}

} // namespace iottorii


