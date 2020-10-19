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
#include "hlmac-table.h"
//#include <stdio.h> //For sprintf()
#include <stdlib.h> //For malloc()
#include "lib/list.h"


#include "sys/log.h"
#define LOG_MODULE "IoTorii-HLMAC-Table"
#define LOG_LEVEL LOG_LEVEL_MAC

LIST(hlmac_table_entery_list);

/*---------------------------------------------------------------------------*/
void
hlmac_table_init(void)
{
  number_of_hlmac_addresses = 0;
}

/*---------------------------------------------------------------------------*/
void
hlmac_table_remove_aged_entries(void)
{
  hlmac_table_entery_t *table_entry;
  for(table_entry=list_head(hlmac_table_entery_list); table_entry!=NULL; table_entry=table_entry->next){
    //If insertion time + aging time <= now, the table entry is expired.
    if(table_entry->insertion_time + IOTORII_TABLE_ENTRY_AGING_TIME * CLOCK_SECOND <= clock_time()){
      #if LOG_DBG_DEVELOPER == 1 || LOG_DBG_STATISTIC == 1
      char *addr_str = hlmac_addr_to_str(table_entry->address);
      LOG_DBG("Aged HLMAC address removed: address: %s, insertion time: %lu\n", addr_str, (unsigned long)(table_entry->insertion_time));
      free(addr_str);
      addr_str = NULL;
      #endif
      list_remove(hlmac_table_entery_list, table_entry);
    }
  }
}

/*---------------------------------------------------------------------------*/
hlmac_table_entery_t *
hlmac_table_is_in_table(const hlmacaddr_t address)
{
  hlmac_table_remove_aged_entries();

  if (list_length(hlmac_table_entery_list) == 0)
      return NULL; //Table is empty.

  /* Find address in the table */
  hlmac_table_entery_t *table_entry;
  for(table_entry=list_head(hlmac_table_entery_list); table_entry!=NULL; table_entry=table_entry->next){
    if (hlmac_cmp(table_entry->address, address) == 0){ //This condithion can be merged with the "for" condition.
      return table_entry;
    }
  }
  //Address is not in the table
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
hlmac_table_update_entry(hlmac_table_entery_t *entry, clock_time_t insertion_time)
{
  entry->insertion_time = insertion_time;
  #if LOG_DBG_DEVELOPER == 1 || LOG_DBG_STATISTIC == 1
  char *addr_str = hlmac_addr_to_str(entry->address);
  LOG_DBG("Aged HLMAC address updated: address: %s, insertion time: %lu\n", addr_str, (unsigned long)(entry->insertion_time));
  free(addr_str);
  addr_str = NULL;
  #endif
}

/*---------------------------------------------------------------------------*/
uint8_t
hlmactable_add(const hlmacaddr_t addr, const clock_time_t insertion_time)
{
  hlmac_table_remove_aged_entries();

  if (number_of_hlmac_addresses >= 255){ //1~255, 0 is not used in the simulation
    #if LOG_DBG_DEVELOPER == 1
    LOG_DBG("Number of HLMAC addresses: %d, table is full.\n", number_of_hlmac_addresses);
    #endif
    return 0;
  }

  if((HLMAC_MAX_HLMAC == -1) || ((HLMAC_MAX_HLMAC != -1) && (number_of_hlmac_addresses < HLMAC_MAX_HLMAC))){
    hlmac_table_entery_t *entry = (hlmac_table_entery_t *)malloc(sizeof(hlmac_table_entery_t));
    entry->address = addr;
    entry->insertion_time = insertion_time;
    list_add(hlmac_table_entery_list, entry);
    number_of_hlmac_addresses ++;

    #if LOG_DBG_DEVELOPER == 1 || LOG_DBG_STATISTIC == 1
    char *addr_str = hlmac_addr_to_str(addr);
    printf("Periodic Statistics: HLMAC address: %s saved to HLMAC table.\n", addr_str);
    free(addr_str);
    addr_str = NULL;
    #endif

    #if LOG_DBG_DEVELOPER == 1
    LOG_DBG("Number of HLMAC address: %d saved to HLMAC table.\n", number_of_hlmac_addresses);
    #endif

    return 1;
  }else{

    #if LOG_DBG_DEVELOPER == 1
    char *addr_str = hlmac_addr_to_str(addr);
    LOG_DBG("HLMAC address %s is not saved to HLMAC table, MAX_HLMAC: %d, number of entries: %d, \n", addr_str, HLMAC_MAX_HLMAC, number_of_hlmac_addresses);
    free(addr_str);
    addr_str = NULL;
    #endif

    #if LOG_DBG_DEVELOPER == 1
    LOG_DBG("Number of HLMAC address: %d saved to HLMAC table.\n", number_of_hlmac_addresses);
    #endif

    return 0;
  }
}

/*---------------------------------------------------------------------------*/
uint8_t
hlmactable_has_loop(const hlmacaddr_t addr)
{

  hlmacaddr_t *longest_prefix = hlmactable_get_longest_matchhed_prefix(addr);

  #if LOG_DBG_DEVELOPER == 1
  char *addr_str = hlmac_addr_to_str(addr);
  char *pref_str = hlmac_addr_to_str(*longest_prefix);
  LOG_DBG("Check Loop: HLMAC address: %s, Longest Prefix: %s \n", addr_str, pref_str);
  free(pref_str);
  pref_str = NULL;
  #endif


  if (hlmac_is_unspecified_addr(*longest_prefix)){
    #if LOG_DBG_DEVELOPER == 1
    LOG_DBG("HLMAC address %s doesn't create a loop\n", addr_str);
    free(addr_str);
    addr_str = NULL;
    #endif
    free(longest_prefix);
    longest_prefix = NULL;
    return 0;
  }else{
    #if LOG_DBG_DEVELOPER == 1
    LOG_DBG("HLMAC address %s creates a loop\n", addr_str);
    free(addr_str);
    addr_str = NULL;
    #endif
    free(longest_prefix->address);
    longest_prefix->address = NULL;
    free(longest_prefix);
    longest_prefix = NULL;
    return 1;
  }

}

/*---------------------------------------------------------------------------*/
hlmacaddr_t *
hlmactable_get_longest_matchhed_prefix(const hlmacaddr_t address)
{
  hlmac_table_remove_aged_entries();

  //if (list_length(hlmac_table_entery_list) == 0)
      //return UNSPECIFIED_HLMAC_ADDRESS;

  hlmacaddr_t *addr = (hlmacaddr_t *)malloc(sizeof(hlmacaddr_t));
  addr->address = (uint8_t *)malloc(sizeof(uint8_t) * address.len);
  //memcpy(addr->address, address.address, address.len);
  uint8_t i;
  for (i=0; i<address.len; i++){
    addr->address[i] = address.address[i];
  }
  addr->len = address.len;

  hlmac_remove_Last_id(addr); //each address is not prefix of itself

  hlmac_table_entery_t *table_entry;

  while(hlmac_get_len(*addr) > 0){
    /* Find address in the table */
    for(table_entry=list_head(hlmac_table_entery_list); table_entry!=NULL; table_entry=table_entry->next){
      if (hlmac_cmp(table_entry->address, *addr) == 0){ //This condithion can be merged with the "for" condition.
        return addr;
      }
    }
    hlmac_remove_Last_id(addr);
  }

  //return UNSPECIFIED_HLMAC_ADDRESS;
  return addr; //Here, addr = UNSPECIFIED_HLMAC_ADDRESS
}
/*---------------------------------------------------------------------------*/
#if LOG_DBG_STATISTIC == 1
int
hlmactable_calculate_sum_hop(void)
{
  hlmac_table_remove_aged_entries();
  int sum = 0;
  hlmac_table_entery_t *table_entry;
  for(table_entry=list_head(hlmac_table_entery_list); table_entry!=NULL; table_entry=table_entry->next){
    sum += table_entry->address.len;
  }
  return sum;
}
#endif
