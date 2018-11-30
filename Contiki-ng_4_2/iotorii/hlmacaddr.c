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

const linkaddr_t UNSPECIFIED_HLMAC_ADDRESS = {NULL, 0};

/*---------------------------------------------------------------------------*/
unsigned char
is_unspecified_hlmac_addr(const helmacaddr_t addr)
{
  if (addr.len == 0) && (!addr.address)
    return 1;
  else
    return 0;
}
/*---------------------------------------------------------------------------*/
unsigned char
get_hlmac_len(const helmacaddr_t addr)
{
  return addr.len;
}
/*---------------------------------------------------------------------------*/
helmacaddr_t *
create_root_id(const unsigned char root_id)
{
  helmacaddr_t *root_addr = (helmacaddr_t *) malloc(sixeof(helmacaddr_t));
  root_addr->address = (unsigned char *) malloc(sixeof(unsigned char));
  root_addr->*address = root_id;
  root_addr->len = 0;
}
/*---------------------------------------------------------------------------*/
//void setIndexValue(unsigned int k, unsigned int addrbyte);
/*---------------------------------------------------------------------------*/
void
add_new_id(helmacaddr_t *addr, const unsigned char new_id)
{
  unsigned char * temp = addr->address;
  addr->address = (unsigned char *) malloc(sizeof(unsigned char) * (addr->len + 1));
  for(int i=0; i<addr->len; i++){
    addr->address[i] = temp[i];
  }
  addr->address[i] = new_id;
  (addr->len) ++;  // \fixme parenteses are need?!
  free(temp);
  temp = NULL;
}
/*---------------------------------------------------------------------------*/
void
remove_Last_id(helmacaddr_t *addr)
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
int
hlmac_cmp(const helmacaddr_t addr1, const helmacaddr_t addr2)
{
  int minSize = (addr1->len < addr2->len) ? addr1->len : addr2->len;

  for (unsigned char i=0; i<minSize; i++){
      if (addr1->address[i] < addr2->address[i])
          return +1;
      else if (addr1->address[i] > addr2->address[i])
          return -1;
  }
  if (addr1->len == addr2->len)
      return 0;
  else if (addr1->len < addr2->len){
      for (int i=minSize; i< address.size(); i++){
          if (address.at(i) > 0)
              return +1;
      }
      return 0;
  }
  else if (address.size() < other.address.size()){
      for (int i=minSize; i< other.address.size(); i++){
          if (other.address.at(i) > 0)
              return -1;
      }
      return 0;
  }
}
