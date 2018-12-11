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
 *         Header file for the HLMAC table representation
 */


#ifndef HLMACTABLE_H_
#define HLMACTABLE_H_

#include "contiki.h"
#include "hlmacaddr.h"

struct hlmac_table_entery{
  struct hlmac_table_entery *next;
  hlmacaddr_t addr;
};

typedef struct hlmac_table_entery hlmac_table_entery_t;
uint8_t number_of_hlmac_addresses;


/**
 * \brief      Initialize the HLMAC address
 */

void hlmac_table_init(void);

/**
 * \brief      Add a HLMAC address to the end of the table
 * \param addr The HLMAC address
 */

uint8_t hlmactable_add(const hlmacaddr_t addr);

/**
 * \brief      Check whether a given HLMAC address creats a loop or not
 * \param addr The given HLMAC address
 * \return  return 1 if there is a loop, 0 otherwise
 */

uint8_t hlmactable_has_loop(const hlmacaddr_t addr);

/**
 * \brief      Find the longest matched prefix for a given HLMAC address
 * \param addr The given HLMAC address
 * \return  return the longest matched prefix if there is, UNSPECIFIED address otherwise.
 */

hlmacaddr_t hlmactable_get_longest_matchhed_prefix(const hlmacaddr_t addr);


#endif /* HLMACTABLE_H_ */
