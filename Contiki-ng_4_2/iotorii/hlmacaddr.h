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

#include "contiki.h"

typedef struct {
  unsigned char *address = NULL;  // The address width is in range of [0 255]
  unsigned char len = 0; // The address length is in range of [0 255]
}  helmacaddr_t;

//To define unspecified HLMAC address

extern const linkaddr_t UNSPECIFIED_HLMAC_ADDRESS;

/**
 * \brief      Check whether a HLMAC address is the unspecifed address or not
 * \param addr The HLMAC address
 * \return  0 if addr is not the unspecified address, +1 otherwise
 */

unsigned char is_unspecified_hlmac_addr(const helmacaddr_t addr);

/**
 * \brief      Get the len of a given HLMAC address
 * \param addr The given HLMAC address
 * \return  The length of the addr
 */

unsigned char get_hlmac_len(const helmacaddr_t addr);

/**
 * \brief      Create a new HLMAC address, and add a root ID
 * \param root_id The root ID to be added to the new HLMAC address
 * \return the root HLMAC address
 */

helmacaddr_t *create_root_id(const unsigned char root_id);

//void setIndexValue(unsigned int k, unsigned int addrbyte);

/**
 * \brief      Add an ID to a given HLMAC address
 * \param addr The given address
 * \param new_id The ID to be added to the end of addr
 * \return addr without its last id
 */

void add_new_id(helmacaddr_t *addr, const unsigned char new_id);

/**
 * \brief      Remove the last ID from a given HLMAC address
 * \param addr The given address
 */

void remove_Last_id(helmacaddr_t *addr);

/**
 * \brief      Compare two HLMAC addresses
 * \param addr1 The first address
 * \param addr2 The second address
 * \return     Zero if the addresses are the same,
 *             +1 if addr1 < addr2,
 *             -1 if addr1 > addr2
 */

int hlmac_cmp(const helmacaddr_t addr1, const helmacaddr_t addr2);



#endif /* HLMACADDR_H_ */
