
/*
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

#include "HLMACAddress.h"

#include <ctype.h>
namespace iotorii {


unsigned char HLMACAddress::getIndex(unsigned int k) const
{
    if ((k < 0) || (k >= getHLMACLength()))
        throw cRuntimeError("HLMACAddress::index: index %d is not in range", k);
    else
    {
        int offset = ((HLMAC_ADDRESS_SIZE * 8) - (k * HLMACWidth) - (1 * HLMACWidth));
        uint64 mask =1;
        for (int i = 1; i < HLMACWidth; i++)
        {
            mask = mask<<1;
            mask = mask+1;
        }


        return mask & (address >> offset);
    }

}

void HLMACAddress::setIndex(unsigned int k, unsigned char indexValue)
{
    if ((k < 0) || (k >= getHLMACLength()))
        throw cRuntimeError("HLMACAddress::setAddressByte: Array of size 6 indexed with %d", k);
    else
    {
        int offset = ((HLMAC_ADDRESS_SIZE * 8) - (k * HLMACWidth) - (1 * HLMACWidth));
        uint64 mask =1;
        for (int i = 1; i < HLMACWidth; i++)
        {
            mask = mask<<1;
            mask = mask+1;
        }

        address = (address & (~(mask << offset))) | (((uint64)indexValue) << offset);
    }

}

std::string HLMACAddress::str() const
{
    char *buf = new char[getHLMACLength()*2];
    char *s = buf;
    int i;
    for (i = 0; i < getHLMACLength(); i++, s += 2)
        sprintf(s, "%1.1X.", getIndex(i));
    *(s-1) = '\0';
    return std::string(buf);
}

bool HLMACAddress::tryParse(const char *hexstr)
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
    if (numHexDigits != getHLMACLength())    // -1 because last byte
        return false;

    // Converts hex string into the address
    // if hext string is shorter, address is filled with zeros;
    // Non-hex characters are discarded before conversion.
    address = 0;    // clear top 16 bits too that setAddressByte() calls skip
    const char *s = hexstr;
    int pos;
    for (pos = 0; pos < getHLMACLength(); pos++) {
        if (!s || !*s) {
            setIndex(pos, 0);
        }
        else {
            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setIndex(pos, 0);
                continue;
            }
            unsigned char d = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            if (d >= pow(2,HLMACWidth))
                return false;
            s++;
            setIndex(pos, d);
        }
    }


    return true;
}



void HLMACAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("HLMACAddress: wrong address syntax '%s': %d hex digits expected, with optional embedded spaces, hyphens or colons", hexstr, getHLMACLength());
}

void HLMACAddress::setCore(unsigned char newCoreId)
{
    setIndex(0, newCoreId);
}

unsigned short int HLMACAddress::getHLMACHier()
{
    unsigned short int hier = HLMAC_ADDRESS_SIZE;
    while ((hier > 0) && (getIndex(--hier) == 0));
    return hier;
}

} // namespace iotorii


