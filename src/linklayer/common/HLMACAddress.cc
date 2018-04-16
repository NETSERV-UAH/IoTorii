
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

unsigned char HLMACAddress::getAddressByte(unsigned int k) const
{
    if ((k < 0) && (k >= HLMAC_ADDRESS_SIZE))
        throw cRuntimeError("HLMACAddress::setAddressByte: Array of size 6 indexed with %d", k);
    else
    {
        int offset = (HLMAC_ADDRESS_SIZE - k - 1) * 8;
        if (k == HLMAC_ADDRESS_SIZE - 1)    //if k indicates last byte of HLMACAddress
            return 0xf0 & (address >> offset); // only 4 bits belong to HLMACAddress, others belong to eGA3 frame type
        else
            return 0xff & (address >> offset);
    }

}

void HLMACAddress::setAddressByte(unsigned int k, unsigned char addrbyte)
{
    if ((k < 0) && (k >= HLMAC_ADDRESS_SIZE))
        throw cRuntimeError("HLMACAddress::setAddressByte: Array of size 6 indexed with %d", k);
    else
    {
        int offset = (HLMAC_ADDRESS_SIZE - k - 1) * 8;
        if (k == HLMAC_ADDRESS_SIZE - 1)    //if k indicates last byte of HLMACAddress
            if (addrbyte < 16)
                address = (address & (~(((uint64)0xff) << offset))) | (((uint64)addrbyte) << (offset+4));
            else
                throw cRuntimeError("HLMACAddress::setAddressByte: 4 bits are allowed to save %2.2X (hex) in last byte of HLMACAddress.", addrbyte);
        else
            address = (address & (~(((uint64)0xff) << offset))) | (((uint64)addrbyte) << offset);
    }

}

unsigned short int HLMACAddress::getHLMACHier()
{
    unsigned short int hier = HLMAC_ADDRESS_SIZE;
    while ((hier > 0) && (getAddressByte(--hier) == 0));
    return hier;
}

HLMACAddress HLMACAddress::getHLMACSuffix()   // first & last address bytes must be zero
{
    //return HLMACAddress(address >> ((HLMAC_ADDRESS_SIZE - getHier())*8));
    if (getHLMACHier()<1)
        return HLMACAddress();

    HLMACAddress suffix = getHLMACPrio();      //first address byte = 0
    suffix.setAddressByte(getHLMACHier()-1, 0);  // last address byte =0
    return suffix;

}

HLMACAddress HLMACAddress::getPrioSuffix()    // last address byte must be zero (here, "this" is a prio, not HLMAC)
{
    HLMACAddress suffix(address);
    suffix.setAddressByte(getHLMACHier(), 0);
    return suffix;

}

unsigned short int HLMACAddress::getPrioHier() // "this" is a prio, not HLMAC
{
    if (address == 0) // because of ambiguity when prio is 00.00.00.00.00.0. both xx.(00.00.00.00.00.0) & xx.(yy.00.00.00.00.0) priorities return 0 as hierarchy
        return 0;
    else
        return getHLMACHier() + 1;
}

void HLMACAddress::setCore(unsigned char newCoreId)
{
    setAddressByte(0, newCoreId);
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
    if (numHexDigits != 2 * HLMAC_ADDRESS_SIZE - 1)    // -1 because last byte
        return false;

    // Converts hex string into the address
    // if hext string is shorter, address is filled with zeros;
    // Non-hex characters are discarded before conversion.
    address = 0;    // clear top 16 bits too that setAddressByte() calls skip
    int k = 0;
    const char *s = hexstr;
    int pos;
    for (pos = 0; pos < HLMAC_ADDRESS_SIZE - 1; pos++) {
        if (!s || !*s) {
            setAddressByte(pos, 0);
        }
        else {
            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setAddressByte(pos, 0);
                continue;
            }
            unsigned char d = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            d = d << 4;
            s++;

            while (*s && !isxdigit(*s))
                s++;
            if (!*s) {
                setAddressByte(pos, 0);
                continue;
            }
            d += isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            s++;

            setAddressByte(pos, d);
            k++;
        }
    }

    // last data byte
    while (*s && !isxdigit(*s))
                    s++;
    unsigned char lastd = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
    s++;
    setAddressByte(pos, lastd);
    k++;

    return true;
}

void HLMACAddress::setAddress(const char *hexstr)
{
    if (!tryParse(hexstr))
        throw cRuntimeError("HLMACAddress: wrong address syntax '%s': 11 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
}

void HLMACAddress::getAddressBytes(unsigned char *addrbytes) const
{
    for (int i = 0; i < HLMAC_ADDRESS_SIZE; i++)
        addrbytes[i] = getAddressByte(i);
}

void HLMACAddress::setAddressBytes(unsigned char *addrbytes)
{
    address = 0;    // clear top 16 bits too that setAddressByte() calls skip
    for (int i = 0; i < HLMAC_ADDRESS_SIZE; i++)
        setAddressByte(i, addrbytes[i]);
}

std::string HLMACAddress::str() const
{
    char buf[20];
    char *s = buf;
    int i;
    for (i = 0; i < HLMAC_ADDRESS_SIZE-1; i++, s += 3)
        sprintf(s, "%2.2X.", getAddressByte(i));
    sprintf(s, "%1.1X.", getAddressByte(i));
    s+=2;
    *(s - 1) = '\0';
    return std::string(buf);
}

int HLMACAddress::compareTo(const HLMACAddress& other) const
{
    return (address < other.address) ? -1 : (address == other.address) ? 0 : 1;    // note: "return address-other.address" is not OK because 64-bit result does not fit into the return type
}

} // namespace iotorii


