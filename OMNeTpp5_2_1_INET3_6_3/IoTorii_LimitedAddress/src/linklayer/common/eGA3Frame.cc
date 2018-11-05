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

eGA3Frame::eGA3Frame(unsigned char type, HLMACAddress address )
{
    data = 0;
    setHLMACAddress(address);
    seteGA3FrameType(type);
}

unsigned char eGA3Frame::getIndexValue(unsigned int k) const
{
    if ((k < 0) || (k >= 8))  //if ((k < 0) || (k >= (getHLMACLength() + 1))
        throw cRuntimeError("eGA3Frame::getIndexValue(): index %d is not in range", k);
    else
    {
        int offset = ((FRAME_DATA_SIZE * 8) - (k * 2) - (1 * 2));// offset = ((HLMACAddress::HLMAC_ADDRESS_SIZE * 8) - (k * HLMACAddress::HLMAC_WIDTH) - (1 * HLMACAddress::HLMAC_WIDTH));
        uint64 mask =1;
        for (int i = 1; i < 2; i++)  //for (int i = 1; i < HLMACAddress::HLMAC_WIDTH; i++)
        {
            mask = mask<<1;
            mask = mask+1;
        }


        return mask & (data >> offset);
    }

}

HLMACAddress eGA3Frame::getHLMACAddress()
{
    return HLMACAddress(data & FRAME_HLMAC_MASK);
}

void eGA3Frame::setIndexValue(unsigned int k, unsigned char indexValue)
{
    if ((k < 0) || (k >= 8))  //if ((k < 0) || (k >= HLMACAddress::getHLMACLength() + 1))
        throw cRuntimeError("eGA3Frame::setIndexvalue(): index %d is not in range", k);
    else
    {
        int offset = ((FRAME_DATA_SIZE * 8) - (k * 2) - (1 * 2));  //int offset = ((HLMACAddress::HLMAC_ADDRESS_SIZE * 8) - (k * HLMACAddress::HLMAC_WIDTH) - (1 * HLMACAddress::HLMAC_WIDTH));
        uint64 mask =1;
        for (int i = 1; i < 2; i++)  //for (int i = 1; i < HLMACAddress::HLMAC_WIDTH; i++)
        {
            mask = mask<<1;
            mask = mask+1;
        }

        data = (data & (~(mask << offset))) | (((uint64)indexValue) << offset);
    }
}

void eGA3Frame::seteGA3FrameType(unsigned char type)
{
    if (type > (pow(2, 2) - 1))  //if (type > (pow(2, HLMACAddress::HLMAC_WIDTH) - 1))
        throw cRuntimeError("eGA3Frame::seteGA3FrameType(): eGA3FrameType is not in range, it must be at most %d bits.", type, 2);  //throw cRuntimeError("eGA3Frame::seteGA3FrameType(): eGA3FrameType is not in range, it must be at most %d bits.", type,  HLMACAddress::HLMAC_WIDTH);
    setIndexValue(7, type);  //setIndexValue(HLMACAddress::getHLMACLength(), type);
}

void eGA3Frame::setHLMACAddress(HLMACAddress addr)
{
    unsigned char type = geteGA3FrameType();
    data = addr.getInt();
    seteGA3FrameType(type);
}

std::string eGA3Frame::str() const
{
    char *buf = new char[16];  //char *buf = new char[HLMACAddress::getHLMACLength()*2+HLMACAddress::getHLMACWidth()];
    char *s = buf;
    int i;
    for (i = 0; i < 8; i++, s += 2)  //for (i = 0; i < HLMACAddress::getHLMACLength() + 1; i++, s += 2)
        sprintf(s, "%1.1X.", getIndexValue(i));
    *(s-1) = '\0';
    return std::string(buf);
}

} // namespace iottorii


