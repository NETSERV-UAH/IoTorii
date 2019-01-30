/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), and David Carrascal(1);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran
 *                        Polytechnic), Iran.
 * Adapted to use IoTorii, a link layer protocol for Low pawer and Lossy Network
 * (LLN), over the IEEE 802.15.4 none beacon mode.
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
*/


/**
 * \file
 *         Header file for the HLMAC address representation
 */


#ifndef HLMACADDR_H_
#define HLMACADDR_H_

typedef unsigned char uint8_t;

typedef struct {
  uint8_t *address;  // The address width is in range of [0 255]
  uint8_t len; // The address length is in range of [0 255]
}  hlmacaddr_t;


//To define unspecified HLMAC address
extern const hlmacaddr_t UNSPECIFIED_HLMAC_ADDRESS;

/**
 * \brief      Check whether a HLMAC address is the unspecifed address or not
 * \param addr The HLMAC address
 * \return  0 if addr is not the unspecified address, +1 otherwise
 */

uint8_t hlmac_is_unspecified_addr(const hlmacaddr_t addr);

/**
 * \brief      Get the len of a given HLMAC address
 * \param addr The given HLMAC address
 * \return  The length of the addr
 */

uint8_t hlmac_get_len(const hlmacaddr_t addr);

/**
 * \brief      Get the address fiel of a given HLMAC address. This function doesn't add '\0' at the end of address field.
 * \param addr The given HLMAC address
 * \return  The address field of the addr
 */

uint8_t *hlmac_get_address(const hlmacaddr_t addr);

/**
 * \brief      Create a new HLMAC address, and add a root ID
 * \param root_id The root ID to be added to the new HLMAC address
 * \return the root HLMAC address
 */

void hlmac_create_root_addr(hlmacaddr_t * root_addr, const uint8_t root_id);

//void setIndexValue(unsigned uint8_t k, unsigned uint8_t addrbyte);

/**
 * \brief      Add an ID to a given HLMAC address
 * \param addr The given address
 * \param new_id The ID to be added to the end of addr
 * \return addr without its last id
 */

void hlmac_add_new_id(hlmacaddr_t *addr, const uint8_t new_id);

/**
 * \brief      Remove the last ID from a given HLMAC address
 * \param addr The given address
 */

void hlmac_remove_Last_id(hlmacaddr_t *addr);

/**
 * \brief      Compare two HLMAC addresses
 * \param addr1 The first address
 * \param addr2 The second address
 * \return     Zero if the addresses are the same,
 *             +1 if addr1 < addr2,
 *             -1 if addr1 > addr2
 */

char hlmac_cmp(const hlmacaddr_t addr1, const hlmacaddr_t addr2);

/**
 * \brief      Get the value of an kth-ID in a HLMAC address
 * \param addr The HLMAC address
 * \param k The index of the ID
 * \return     The value of the kth-ID
 */

uint8_t get_addr_index_value(const hlmacaddr_t addr, const uint8_t k);

/**
 * \brief      Get the address fiel of a given HLMAC address as a string value, so this function add '\0' at the end of address field.
 * \param addr The given HLMAC address
 * \return  The address field of the addr
 */

char *hlmac_addr_to_str(hlmacaddr_t addr);

hlmacaddr_t hlmac_str_to_addr(const char *str);

#endif /* HLMACADDR_H_ */
