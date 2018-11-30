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
unsigned char
is_unspecified_hlmac_addr(const hlmacaddr_t addr)
{
  if ((addr.len == 0) && (!addr.address))
    return 1;
  else
    return 0;
}
/*---------------------------------------------------------------------------*/
unsigned char
get_hlmac_len(const hlmacaddr_t addr)
{
  return addr.len;
}
/*---------------------------------------------------------------------------*/
unsigned char *
get_hlmac_address(const hlmacaddr_t addr)
{
  unsigned char *address = (unsigned char *) malloc(sizeof(unsigned char) * addr.len);
  for(unsigned char i=0; i<addr.len; i++){
    address[i] = addr.address[i];
  }
  return address;
}
/*---------------------------------------------------------------------------*/
hlmacaddr_t *
create_root_addr(const unsigned char root_id)
{
  hlmacaddr_t *root_addr = (hlmacaddr_t *) malloc(sizeof(hlmacaddr_t));
  root_addr->address = (unsigned char *) malloc(sizeof(unsigned char));
  root_addr->address[0] = root_id;
  root_addr->len = 0;
  return root_addr;
}
/*---------------------------------------------------------------------------*/
void
add_new_id(hlmacaddr_t *addr, const unsigned char new_id)
{
  unsigned char * temp = addr->address;
  addr->address = (unsigned char *) malloc(sizeof(unsigned char) * (addr->len + 1));
  unsigned char i;
  for(i=0; i<addr->len; i++){
    addr->address[i] = temp[i];
  }
  addr->address[i] = new_id;
  (addr->len) ++;  // \fixme parenteses are need?!
  free(temp);
  temp = NULL;
}
/*---------------------------------------------------------------------------*/
void
remove_Last_id(hlmacaddr_t *addr)
{
  if (addr->len == 0)
    return;
  if (addr->len == 1){
    free(addr->address);
    addr->address = NULL;
    addr->len = 0;
    return;
  }
  unsigned char * temp = addr->address;
  addr->address = (unsigned char *) malloc(sizeof(unsigned char) * (addr->len - 1));
  for(int i=0; i<addr->len-1; i++){
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
  unsigned char min_len = (addr1.len < addr2.len) ? addr1.len : addr2.len;
  unsigned char i;

  for (i=0; i<min_len; i++){
      if (addr1.address[i] < addr2.address[i])
          return +1;
      else if (addr1.address[i] > addr2.address[i])
          return -1;
  }
  if (addr1.len == addr2.len)
      return 0;
  if (addr1.len < addr2.len){
      for (; i< addr2.len; i++){ // for (unsigned char i=min_len; i< addr2->len; i++){

          if (addr2.address[i] > 0) // Check whether the other IDs are 0 or not.
              return +1; // when addr1 = 1.2.3 and addr2 = 1.2.3.4 (even, 1.2.3.0.4)
      }
      return 0; // when addr1 = 1.2.3, addr2 = 1.2.3.0 (or 1.2.3.0.0 ...)
  }
  if (addr1.len > addr2.len){
      for (; i< addr1.len; i++){ // for (unsigned char i=min_len; i< addr1->len; i++){
          if (addr1.address[i] > 0) // Check whether the other IDs are 0 or not.
              return -1; // when addr1 = 1.2.3.4 (even, 1.2.3.0.4) and addr2 = 1.2.3
      }
      return 0; // when addr1 = 1.2.3.0 (or 1.2.3.0.0 ...), addr2 = 1.2.3
  }
  return -2; //\fixme not happen, for compile error
}
/*---------------------------------------------------------------------------*/
/*hlmacaddr_t *
char_array_to_hlmac_addr(const unsigned char *ch_array, const unsigned char len)
{

}*/
/*---------------------------------------------------------------------------*/
/**
 * \brief      Get the kth ID in a given HLMAC address. Since this function is just used here, we don't declare it in the header file, i.e. same as the private methods in C++.
 * \param addr The given HLMAC address
 * \param k  The index of the ID in the given HLMAC address, k is in range of [0 255]
 * \return     Value of the kth ID
 *
 */
unsigned char
get_addr_index_value(const hlmacaddr_t addr, const unsigned char k)
{
  if (k < (addr.len))
    return addr.address[k-1];
  else{
    const unsigned char *addr_str = hlmac_addr_to_str(addr);
    LOG_ERR("get_addr_index_value(): index %d is out of range %s", k, addr_str);
    //free(addr_str); // \fexme const => err, leakage?
    return 0; //for ERR, x.0 is not valid addres
  }

}
/*---------------------------------------------------------------------------*/
const unsigned char *
hlmac_addr_to_str(hlmacaddr_t addr)
{
  unsigned char *address = (unsigned char *) malloc(sizeof(unsigned char) * (addr.len * (2 +1) + 1)); //first 1 for dot,e.g 01.0F, second 1 for "\0" at the end of address array.
  unsigned char *s = address;
  unsigned char i;
  for (i = 0; i < addr.len; i++, s += 3)
      //sprintf(s, "%2.2u.", get_addr_index_value(addr, i));
  *(s) = '\0';
  return address;
}
