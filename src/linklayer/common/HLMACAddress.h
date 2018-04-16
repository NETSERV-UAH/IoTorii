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

#ifndef IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_
#define IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_

#include <string>
#include "inet/common/INETDefs.h"


#define HLMAC_ADDRESS_SIZE    6   //note that exact size is 5.5
#define HLMAC_ADDRESS_MASK    0xffffffffffffULL
//#define HLMAC_PRIORITY_MASK   0x00fffffffffULL

namespace iotorii {
using namespace inet;

/**
 * Stores an GA3 HLMAC address (5 octets = 40 bits).
 */


class HLMACAddress {

      private:
        uint64 address;    // 5*8=40 bit address, lowest 5 bytes are used, highest 3 bytes are always zero

      public:

        /**
         * Default constructor initializes address bytes to zero.
         */
        HLMACAddress() { address = 0; }

        /**
         * Initializes the address from the lower 48 bits of the 64-bit argument
         */
        explicit HLMACAddress(uint64 bits) { address = bits & HLMAC_ADDRESS_MASK; }

        /**
         * Constructor which accepts a hex string (12 hex digits, may also
         * contain spaces, hyphens and colons)
         */
        explicit HLMACAddress(const char *hexstr) { setAddress(hexstr); }

        /**
         * Copy constructor.
         */
        HLMACAddress(const HLMACAddress& other) { address = other.address; }

        /**
         * Assignment.
         */
        HLMACAddress& operator=(const HLMACAddress& other) { address = other.address; return *this; }

        /**
         * Returns the address size in bytes, that is, 6.
         */
        unsigned int getAddressSize() const { return HLMAC_ADDRESS_SIZE; }

        /**
         * Returns the kth byte of the address.
         */
        unsigned char getAddressByte(unsigned int k) const;

        HLMACAddress getHLMACSuffix();
        HLMACAddress getHLMACPrio() { return HLMACAddress(address << 8);}//(address & HLMAC_PRIORITY_MASK); }
        unsigned short int getHLMACHier();
        HLMACAddress getPrioSuffix();
        unsigned short int getPrioHier();





        /**
         * Sets the kth byte of the address.
         */
        void setAddressByte(unsigned int k, unsigned char addrbyte);

        /* add new port id to frame. note that this method cannot insert new core.
         * for insertion new core, we can directly use setAddressByte(0, core) or setCore()
         */
        void addNewId(unsigned char newPortId) { setAddressByte(getHLMACHier()+1, newPortId); }

        void setCore(unsigned char newCoreId);


        /**
         * Sets the address and returns true if the syntax of the string
         * is correct. (See setAddress() for the syntax.)
         */
        bool tryParse(const char *hexstr);

        /**
         * Converts address value from hex string (12 hex digits, may also
         * contain spaces, hyphens and colons)
         */
        void setAddress(const char *hexstr);

        /**
         * Copies the address to the given pointer (array of 6 unsigned chars).
         */
        void getAddressBytes(unsigned char *addrbytes) const;
        void getAddressBytes(char *addrbytes) const { getAddressBytes((unsigned char *)addrbytes); }

        /**
         * Sets address bytes. The argument should point to an array of 6 unsigned chars.
         */
        void setAddressBytes(unsigned char *addrbytes);
        void setAddressBytes(char *addrbytes) { setAddressBytes((unsigned char *)addrbytes); }

        /**
         * Converts address to a hex string.
         */
        std::string str() const;

        /**
         * Converts address to 48 bits integer.
         */
        uint64 getInt() const { return address; }

        /**
         * Returns true if the two addresses are equal.
         */
        bool equals(const HLMACAddress& other) const { return address == other.address; }

        /**
         * Returns true if the two addresses are equal.
         */
        bool operator==(const HLMACAddress& other) const { return address == other.address; }

        /**
         * Returns true if the two addresses are not equal.
         */
        bool operator!=(const HLMACAddress& other) const { return address != other.address; }

        /**
         * Returns -1, 0 or 1 as result of comparison of 2 addresses.
         */
        int compareTo(const HLMACAddress& other) const;

        bool operator<(const HLMACAddress& other) const { return address < other.address; }

        bool operator>(const HLMACAddress& other) const { return address > other.address; }
    };

inline std::ostream& operator<<(std::ostream& os, const HLMACAddress& hlmac)
{
    return os << hlmac.str();
}


}// namespace iotorii
#endif /* IOTORII_SRC_LINKLAYER_COMMON_HLMACADDRESS_H_ */
