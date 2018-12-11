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
 *         Functions for managing the HLMAC addresses
 */


#include "contiki.h"
#include "hlmacaddr.h"
#include <stdio.h> //For sprintf()
#include <stdlib.h> //For malloc()

#include "sys/log.h"
#define LOG_MODULE "IoTorii"
#define LOG_LEVEL LOG_LEVEL_MAC

const hlmacaddr_t UNSPECIFIED_HLMAC_ADDRESS = {NULL, 0};

/*---------------------------------------------------------------------------*/
uint8_t
hlmac_is_unspecified_addr(const hlmacaddr_t addr)
{
  if ((addr.len == 0) && (!addr.address))
    return 1;
  else
    return 0;
}
/*---------------------------------------------------------------------------*/
uint8_t
hlmac_get_len(const hlmacaddr_t addr)
{
  return addr.len;
}
/*---------------------------------------------------------------------------*/
uint8_t *
hlmac_get_address(const hlmacaddr_t addr)
{
  uint8_t *address = (uint8_t *) malloc(sizeof(uint8_t) * addr.len);
  uint8_t i;
  for(i=0; i<addr.len; i++){ //for(uint8_t i=0; i<addr.len; i++){ : allowed for C99 mode, so use option -std=c99 or -std=gnu99 to compile this "for" loop
    address[i] = addr.address[i];
  }
  return address;
}
/*---------------------------------------------------------------------------*/
void
hlmac_create_root_addr(hlmacaddr_t * root_addr, const uint8_t root_id)
{
  root_addr->address = (uint8_t *) malloc(sizeof(uint8_t));
  root_addr->address[0] = root_id;
  root_addr->len = 1;
  //LOG_DBG("Root.address: %d, Root.len: %d", node_hlmac_address.address[0], node_hlmac_address.len);
}

/*---------------------------------------------------------------------------*/
void
hlmac_add_new_id(hlmacaddr_t *addr, const uint8_t new_id)
{
  uint8_t * temp = addr->address;
  addr->address = (uint8_t *) malloc(sizeof(uint8_t) * (addr->len + 1));
  uint8_t i;
  for(i=0; i<addr->len; i++){
    addr->address[i] = temp[i];
  }
  addr->address[i] = new_id;
  (addr->len) ++;
  free(temp);
  temp = NULL;
}
/*---------------------------------------------------------------------------*/
void
hlmac_remove_Last_id(hlmacaddr_t *addr)
{
  if (addr->len == 0)
    return;
  if (addr->len == 1){
    free(addr->address);
    addr->address = NULL;
    addr->len = 0;
    return;
  }
  uint8_t * temp = addr->address;
  addr->address = (uint8_t *) malloc(sizeof(uint8_t) * (addr->len - 1));
  uint8_t i;
  for(i=0; i<addr->len-1; i++){ //for(uint8_t i=0; i<addr->len-1; i++){ : allowed for C99 mode, so use option -std=c99 or -std=gnu99 to compile this "for" loop
    addr->address[i] = temp[i];
  }
  (addr->len) --;  // \fixme parenteses are need?!
  free(temp);
  temp = NULL;

}
/*---------------------------------------------------------------------------*/
char
hlmac_cmp(const hlmacaddr_t addr1, const hlmacaddr_t addr2)
{
  uint8_t min_len = (addr1.len < addr2.len) ? addr1.len : addr2.len;
  uint8_t i;

  for (i=0; i<min_len; i++){
      if (addr1.address[i] < addr2.address[i])
          return +1;
      else if (addr1.address[i] > addr2.address[i])
          return -1;
  }
  if (addr1.len == addr2.len)
      return 0;
  if (addr1.len < addr2.len){
      for (; i< addr2.len; i++){ // for (uint8_t i=min_len; i< addr2->len; i++){

          if (addr2.address[i] > 0) // Check whether the other IDs are 0 or not.
              return +1; // when addr1 = 1.2.3 and addr2 = 1.2.3.4 (even, 1.2.3.0.4)
      }
      return 0; // when addr1 = 1.2.3, addr2 = 1.2.3.0 (or 1.2.3.0.0 ...)
  }
  if (addr1.len > addr2.len){
      for (; i< addr1.len; i++){ // for (uint8_t i=min_len; i< addr1->len; i++){
          if (addr1.address[i] > 0) // Check whether the other IDs are 0 or not.
              return -1; // when addr1 = 1.2.3.4 (even, 1.2.3.0.4) and addr2 = 1.2.3
      }
      return 0; // when addr1 = 1.2.3.0 (or 1.2.3.0.0 ...), addr2 = 1.2.3
  }
  return -2; //\fixme not happen, for compile error only
}
/*---------------------------------------------------------------------------*/
hlmacaddr_t *
hlmac_char_array_to_addr(const uint8_t *ch_array, const uint8_t len)
{
  hlmacaddr_t *addr = (hlmacaddr_t *) malloc(sizeof(hlmacaddr_t));
  addr->address = (uint8_t *) malloc(sizeof(uint8_t) * (len));
  addr->len = len;
  return addr;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get the kth ID in a given HLMAC address.
 * \param addr The given HLMAC address
 * \param k  The index of the ID in the given HLMAC address, k is in range of [0 255]
 * \return     Value of the kth ID
 *
 */
uint8_t
get_addr_index_value(const hlmacaddr_t addr, const uint8_t k)
{
  if (k < (addr.len))
    return addr.address[k]; //k is in range [0 len-1]
  else{
    const char *addr_str = hlmac_addr_to_str(addr);
    LOG_ERR("get_addr_index_value(): index %d is out of range %s", k, addr_str);
    //free(addr_str); // \fexme const => err, leakage?
    return 0; //for ERR, x.0 is not valid addres
  }

}
/*---------------------------------------------------------------------------*/
char *
hlmac_addr_to_str(const hlmacaddr_t addr)
{
  char *address = (char *) malloc(sizeof(char) * (addr.len * (2 +1) + 1)); //first 1 for dot,e.g 01.0F, second 1 for "\0" at the end of address array.
  char *s = (char *)address;
  uint8_t i;
  for (i = 0; i < addr.len; i++, s += 3)
      sprintf(s, "%2.2u.", (char)get_addr_index_value(addr, i));
  *(s) = '\0';
  return address;
}
