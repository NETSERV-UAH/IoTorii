/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), and David Carrascal(1);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran
 *                        Polytechnic), Iran.
 * Adapted to use IoTorii, a link layer protocol for Low pawer and Lossy Network
 * (LLN), over the IEEE 802.15.4 standard CSMA protocol (nonbeacon-enabled).
 *
 */

/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
*         The 802.15.4 standard CSMA protocol (nonbeacon-enabled)
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Simon Duquennoy <simon.duquennoy@inria.fr>
 */

//EXTRA BEGIN
//#include "net/mac/csma/csma.h"
//#include "net/mac/csma/csma-output.h"
#include "csma-output.h"
#include "iotorii-csma.h"
#include "sys/ctimer.h"
#include "lib/list.h"
#include <stdlib.h> //For malloc()
#include "lib/random.h"
#include "hlmac-table.h"

#if LOG_DBG_STATISTIC == 1
#include "sys/node-id.h" //For node_id
#include "sys/rtimer.h" //For rtimer_clock_t, RTIMER_NOW()
#endif
//EXTRA END
#include "net/mac/mac-sequence.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

/* Log configuration */
//EXTRA BEGIN
#include "sys/log.h"
#if IOTORII_NODE_TYPE == 0 //The traditional MAC operation
#define LOG_MODULE "CSMA"
#elif IOTORII_NODE_TYPE > 0 // 1 => the root node, 2 => the common nodes
#define LOG_MODULE "IoTorii-CSMA"
#endif
#define LOG_LEVEL LOG_LEVEL_MAC
//EXTRA END

//EXTRA BEGIN
/* Time to send Hello messages
 * Unit : second
 * IoTorii starts to randomly send a hello message in range
 * [IOTORII_CONF_HELLO_START_TIME/2 IOTORII_CONF_HELLO_START_TIME]
 * after initializing a node.
 */
#ifdef IOTORII_CONF_HELLO_START_TIME
#define IOTORII_HELLO_START_TIME IOTORII_CONF_HELLO_START_TIME
#else
#define IOTORII_HELLO_START_TIME 2 //Default Delay is 2 s
#endif

/* Time to send the first SetHLMAC message by a root node
 * Unit : second
 * IoTorii starts to send a SetHLMAC message
 * at t = IOTORII_CONF_SETHLMAC_START_TIME
 * after initializing the root node.
 */
#ifdef IOTORII_CONF_SETHLMAC_START_TIME
#define IOTORII_SETHLMAC_START_TIME IOTORII_CONF_SETHLMAC_START_TIME
#else
#define IOTORII_SETHLMAC_START_TIME 10 //Default Delay is 10 s
#endif

/* Delay before sending a SetHLMAC message
 * Unit : tick
 * 1 second is 128 ticks
 * IoTorii uniformly waits for random delay in range
 * [IOTORII_CONF_SETHLMAC_DELAY/2 IOTORII_CONF_SETHLMAC_DELAY]
 * before sending a SetHLMAC in a common node.
 */
#ifdef IOTORII_CONF_SETHLMAC_DELAY
#define IOTORII_SETHLMAC_DELAY IOTORII_CONF_SETHLMAC_DELAY
#else
#define IOTORII_SETHLMAC_DELAY 0 //Default Delay is zero
#endif

/* Time to print statistics
 * Unit : second
 * IoTorii nodes start to print statistic logs on the output
 * at t = IOTORII_CONF_STATISTICS_TIME
 * after initializing a node.
 */
#ifdef IOTORII_CONF_STATISTICS_TIME
#define IOTORII_STATISTICS_TIME IOTORII_CONF_STATISTICS_TIME
#else
#define IOTORII_STATISTICS_TIME 20 //Default Delay is 20 s
#endif


#if IOTORII_NODE_TYPE == 1 //only root
static struct ctimer sethlmac_timer;
#endif
#if IOTORII_NODE_TYPE > 0 //root and common node
static struct ctimer hello_timer;
static struct ctimer send_sethlmac_timer;
#if LOG_DBG_STATISTIC == 1
static struct ctimer statistic_timer;
int number_of_hello_messages = 0;
int number_of_sethlmac_messages = 0;
#endif
#endif

#if IOTORII_NODE_TYPE > 0 //root and common node
//Create Neighbour table
LIST(neighbour_table_entry_list);
//number_of_neighbours = 0; //This variable is initialized in the init function.
//EXTRA END

//A list/queue to hold messages (Payloads and receiver addresses)
struct payload_entry{
  struct payload_entry *next;
  uint8_t *payload;
  int data_len;
  //linkaddr_t receiver_addr; //Since all receiver_addrs are Broadcast, it is eliminated.
};
typedef struct payload_entry payload_entry_t;
LIST(payload_entry_list);
#endif
//EXTRA End

static void
init_sec(void)
{
#if LLSEC802154_USES_AUX_HEADER
  if(packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL) ==
     PACKETBUF_ATTR_SECURITY_LEVEL_DEFAULT) {
    packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL,
                       CSMA_LLSEC_SECURITY_LEVEL);
  }
#endif
}
/*---------------------------------------------------------------------------*/
static void
send_packet(mac_callback_t sent, void *ptr)
{

  init_sec();

  csma_output_packet(sent, ptr);
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return NETSTACK_RADIO.on();
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  return NETSTACK_RADIO.off();
}
/*---------------------------------------------------------------------------*/
static int
max_payload(void)
{
  int framer_hdrlen;

  init_sec();

  framer_hdrlen = NETSTACK_FRAMER.length();

  if(framer_hdrlen < 0) {
    /* Framing failed, we assume the maximum header length */
    framer_hdrlen = CSMA_MAC_MAX_HEADER;
  }

  return CSMA_MAC_LEN - framer_hdrlen;
}
/*---------------------------------------------------------------------------*/
//EXTRA BEGIN
#if IOTORII_NODE_TYPE > 0 //The root or common nodes
static void
iotorii_handle_hello_timer()
{
  int mac_max_payload = max_payload();
  if(mac_max_payload <= 0) {
   /* Framing failed, Hello can not be created. */
     LOG_WARN("output: failed to calculate payload size - Hello can not be created\n");
  }else{
    packetbuf_clear(); //Hello has no payload.
    /*
    * The destination address, the broadcast address, is tagged to the outbound
    * packet.
    */
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
    LOG_DBG("Hello prepared to send\n");

    #if LOG_DBG_STATISTIC == 1
    number_of_hello_messages ++;
    LOG_DBG("Number of Hello messages: %d\n", number_of_hello_messages);
    #endif

    send_packet(NULL,NULL);
    //ctimer_reset(&hello_timer); //Restart the timer from the previous expire time.
    //ctimer_restart(&hello_timer); //Restart the timer from current time.
    //ctimer_stop(&hello_timer); //Stop the timer.
  }
}
/*---------------------------------------------------------------------------*/
void
iotorii_handle_send_sethlmac_timer(){
  payload_entry_t *payload_entry;
  //Pop packetbuf from the queue
  payload_entry = list_pop(payload_entry_list);
  if (payload_entry){
    //Preparing the packet buffer
    /* reset packetbuf buffer */
    packetbuf_clear();
    /* copy "payload" */
    memcpy(packetbuf_dataptr(), payload_entry->payload, payload_entry->data_len);
    packetbuf_set_datalen(payload_entry->data_len);

    //Release memory
    free(payload_entry->payload);
    payload_entry->payload = NULL;
    free(payload_entry);
    payload_entry = NULL;

    /*
    * Control info: the destination address, the broadcast address, is tagged to the outbound
    * packet.
    */
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
    LOG_DBG("Queue: SetHLMAC prepared to send\n");

    #if LOG_DBG_STATISTIC == 1
    number_of_sethlmac_messages ++;
    LOG_DBG("Number of SetHLMAC messages: %d\n", number_of_sethlmac_messages);
    #endif
    //If list has an entry, schedule next packet
    if (list_head(payload_entry_list)){
      //Schedule next packet
      clock_time_t sethlmac_delay_time = IOTORII_SETHLMAC_DELAY/2 * (CLOCK_SECOND / 128);
      sethlmac_delay_time = sethlmac_delay_time + (random_rand() % sethlmac_delay_time);
      #if LOG_DBG_DEVELOPER == 1
      LOG_DBG("Scheduling a SetHLMAC message after %u ticks in the future\n", (unsigned)sethlmac_delay_time);
      #endif
      ctimer_set(&send_sethlmac_timer, sethlmac_delay_time, iotorii_handle_send_sethlmac_timer, NULL);
    }

    send_packet(NULL, NULL);
  }
}
/*---------------------------------------------------------------------------*/
void
iotorii_send_sethlmac(hlmacaddr_t addr, linkaddr_t sender_link_address)
{
  int mac_max_payload = max_payload();
  if(mac_max_payload <= 0) {
   /* Framing failed, SetHLMAC can not be created */
     LOG_WARN("output: failed to calculate payload size - SetHLMAC can not be created\n");
  }else{

    neighbour_table_entry_t *neighbour_entry;

     #if LOG_DBG_DEVELOPER == 1// || LOG_DBG_STATISTIC == 1
     LOG_DBG("Info before sending SetHLMAC: ");
     LOG_DBG("Number of neighbours: %d, mac_max_payload: %d, LINKADDR_SIZE: %d.\n", number_of_neighbours, mac_max_payload, LINKADDR_SIZE);
     #endif

     /*
      * Eliminating the sender of the resceived SetHLMAC message from the new payload.
      */
     uint8_t number_of_neighbours_new = number_of_neighbours;

     for(neighbour_entry = list_head(neighbour_table_entry_list); neighbour_entry!= NULL; neighbour_entry=list_item_next(neighbour_entry)){
       if(linkaddr_cmp(&neighbour_entry->addr, &sender_link_address)){
         number_of_neighbours_new = number_of_neighbours - 1;
         #if LOG_DBG_DEVELOPER == 1
         LOG_DBG("Sender node is in neighbor list, number_of_neighbours: %u, number_of_neighbours_new: %u.\n", number_of_neighbours, number_of_neighbours_new);
         #endif

       }
     }

     /*
      * creating a random sequence for neighbors. it is useful when payload
      * can not accommodate all neighbors, so every neighbor has a same chance
      * to be select
      */
     if (number_of_neighbours_new > 0){
       neighbour_table_entry_t **random_list = (neighbour_table_entry_t **) malloc(sizeof(neighbour_table_entry_t *) * number_of_neighbours_new);
       uint8_t j;
       unsigned short r;
       for(j=0; j<number_of_neighbours_new; j++){
         random_list[j] = NULL;
       }
       for(neighbour_entry = list_head(neighbour_table_entry_list); neighbour_entry!= NULL; neighbour_entry=list_item_next(neighbour_entry)){
         if(!linkaddr_cmp(&neighbour_entry->addr, &sender_link_address)){
           //r=random_rand()%(number_of_neighbours);
           while(random_list[r=random_rand()%(number_of_neighbours_new)] != NULL); //Each r per each neighbor must be unic
           random_list[r] = neighbour_entry;
           #if LOG_DBG_DEVELOPER == 1
           LOG_DBG("number_of_neighbours_new = %u Neighbor (ID = %u) gets random priority %u\n", number_of_neighbours_new, random_list[r]->id, r);
           #endif
         }else{
           #if LOG_DBG_DEVELOPER == 1
           LOG_DBG("Sender node is eliminated in random list!\n");
           #endif
         }
       }

       //Accomodate SetHLMAC addresses in the payload.
       /* Is packetbuf a common repository? By using
        * "packetbuf_ptr = packetbuf_dataptr();", when we apply a process
        * on the packetbuf, if node want to send another SetHLMAC message (or
        * receive a new packet), packetbuf is overwrote. To avoid this, we create
        * a copy from packetbuf before starting a process on the packetbuf.
        */
       //Creating a payload
       uint8_t *packetbuf_ptr_head;
       /* A pointer to walk on the payload array/pointer */
       uint8_t *packetbuf_ptr;

       int datalen_counter = 0;
       uint8_t i = 1;

       /* The ayload structure:
        * +------------+--------+-----+------+-----+-----+------+
        * | Prefix len | Prefix | ID1 | MAC1 | ... | IDn | MACn |
        * +------------+--------+-----+------+-----+-----+------+
        */

       /* Add first neighbour
        * 2=(1 is for adding the length of the HLMAC address prefix)+(1 for adding id).
        */

       if (random_list[i-1] && (mac_max_payload >=  (addr.len + 2 + LINKADDR_SIZE))){
          //Preparing the payload
          packetbuf_ptr_head = (uint8_t *) malloc(sizeof(uint8_t) * mac_max_payload);
          packetbuf_ptr = packetbuf_ptr_head;
          //packetbuf_ptr = packetbuf_dataptr();
          /* Create the payload */
          /* Prefix lenght */
          memcpy(packetbuf_ptr, &(addr.len), 1);
          packetbuf_ptr ++;
          datalen_counter ++;
          /* Neighbour prefix */
          memcpy(packetbuf_ptr, addr.address, addr.len);
          packetbuf_ptr += addr.len;
          datalen_counter += addr.len;

        do{
          /* Neighbour ID */
          memcpy(packetbuf_ptr, &(random_list[i-1]->id), 1);
          packetbuf_ptr ++;
          datalen_counter ++;
          /* Neighbour MAC address */
          memcpy(packetbuf_ptr, &(random_list[i-1]->addr), LINKADDR_SIZE);
          packetbuf_ptr += LINKADDR_SIZE;
          datalen_counter += LINKADDR_SIZE;

          i++;
        }while (mac_max_payload >=  (datalen_counter + LINKADDR_SIZE + 1) && i<=number_of_neighbours_new);

        //Preparing a payload entry
        payload_entry_t * payload_entry = (payload_entry_t *) malloc(sizeof(payload_entry_t));
        payload_entry->next = NULL;
        payload_entry->payload = packetbuf_ptr_head;
        payload_entry->data_len = datalen_counter;

        /*
         * Before adding the entry to the queue/list,
         * check if the list is empty or not.
         * if the list is empty, set a timer.
         */
        if (!list_head(payload_entry_list)){
           /* Scheduling a delay before sending a SetHLMAC messages.
            * Delay before the first SetHLMAC, transmitted by the root node,
            * is zero.
            */
           clock_time_t sethlmac_delay_time = 0;
           #if IOTORII_NODE_TYPE > 1
             sethlmac_delay_time = IOTORII_SETHLMAC_DELAY/2 * (CLOCK_SECOND / 128);
             sethlmac_delay_time = sethlmac_delay_time + (random_rand() % sethlmac_delay_time);
             #if LOG_DBG_DEVELOPER == 1
             LOG_DBG("Scheduling a SetHLMAC message after %u ticks in the future\n", (unsigned)sethlmac_delay_time);
             #endif
          #elif IOTORII_NODE_TYPE == 1
             //sethlmac_delay_time = 0;
             #if LOG_DBG_DEVELOPER == 1
             LOG_DBG("Scheduling a SetHLMAC message by the root node after %u ticks in the future\n", (unsigned)sethlmac_delay_time);
             #endif
          #endif
          //First, add the payload to the end of the queue/list, then set the timer
          list_add(payload_entry_list, payload_entry);
          ctimer_set(&send_sethlmac_timer, sethlmac_delay_time, iotorii_handle_send_sethlmac_timer, NULL);
        }else{
          list_add(payload_entry_list, payload_entry);
        }

        #if LOG_DBG_DEVELOPER == 1
        char *neighbour_hlmac_addr_str = hlmac_addr_to_str(addr);
        LOG_DBG("SetHLMAC prefix (addr:%s) added to queue to advertise to %d nodes.\n", neighbour_hlmac_addr_str, i-1);
        free(neighbour_hlmac_addr_str);
        #endif

      }else{ //End if random_list[i-1]
        LOG_DBG("Node hos not any neighbour (or payload is low) to send SetHLMAC.\n");
      }

      /* Release memory
       * free(random_list[j]) releases the neighbor table.
       */
      if (number_of_neighbours_new > 0){
        for(j=0; j<number_of_neighbours_new; j++){
          random_list[j] = NULL;
        }
        free(random_list);
        random_list = NULL;
      }

     }//END if number_of_neighbours_new

  }// END else
}
#endif
/*---------------------------------------------------------------------------*/
#if IOTORII_NODE_TYPE == 1 //For root
static void
iotorii_handle_sethlmac_timer()
{
  //uint8_t id = 1;
  hlmacaddr_t root_addr;
  hlmac_create_root_addr(&root_addr, 1);
  hlmactable_add(root_addr);
  #if LOG_DBG_STATISTIC == 1
  LOG_DBG("Periodic Statistics: node_id: %u, convergence_time_start\n", node_id);
  #endif
  iotorii_send_sethlmac(root_addr, linkaddr_node_addr);
  free(root_addr.address); //malloc() in hlmac_create_root_addr()
  root_addr.address = NULL;
  //ctimer_reset(&sethlmac_timer); //Restart the timer from the previous expire time.
  //ctimer_restart(&sethlmac_timer); //Restart the timer from current time.
  //ctimer_stop(&sethlmac_timer); //Stop the timer.
}
#endif
/*---------------------------------------------------------------------------*/
#if IOTORII_NODE_TYPE > 0 //For root and common nodes
#if LOG_DBG_STATISTIC == 1
static void
iotorii_handle_statistic_timer()
{
  LOG_DBG("Periodic Statistics: node_id: %u, number_of_hello_messages: %d, number_of_sethlmac_messages: %d, number_of_neighbours: %d, number_of_hlmac_addresses: %d, sum_hop: %d\n", node_id, number_of_hello_messages, number_of_sethlmac_messages, number_of_neighbours, number_of_hlmac_addresses, hlmactable_calculate_sum_hop());

  //ctimer_reset(&sethlmac_timer); //Restart the timer from the previous expire time.
  //ctimer_restart(&sethlmac_timer); //Restart the timer from current time.
  //ctimer_stop(&sethlmac_timer); //Stop the timer.
}
#endif
#endif
//EXTRA END
/*---------------------------------------------------------------------------*/
static void
init(void)
{
#if LLSEC802154_USES_AUX_HEADER
#ifdef CSMA_LLSEC_DEFAULT_KEY0
  uint8_t key[16] = CSMA_LLSEC_DEFAULT_KEY0;
  csma_security_set_key(0, key);
#endif
#endif /* LLSEC802154_USES_AUX_HEADER */
  csma_output_init();
  on();
  //EXTRA BEGIN
#if IOTORII_NODE_TYPE == 1 //Root node
  LOG_INFO("This node operates as a root.\n");
#endif
#if IOTORII_NODE_TYPE > 1 //Common node
  LOG_INFO("This node operates as a common node.\n ");
#endif
#if IOTORII_NODE_TYPE > 0 //Root or Common node, we set a timer to send a Hello message.

/*
 * dev/ds2411/ds2411.c : ds2411_id[0] = 0x00;
 * arch/platform/sky/platform.c : random_init(ds2411_id[0]);
 * Since all nodes apply same random nuumbers,
 * we initialize a different seed number to each node.
 * we just checked this for the sky mote.
 * If the number of nodes is hier than 255, the seed must be checked is whether
 * unic or not for all nodes.
 */

//#ifdef CONTIKI_TARGET_SKY
  unsigned short seed_number;
  uint8_t min_len_seed = sizeof(unsigned short) < LINKADDR_SIZE ?  sizeof(unsigned short) : LINKADDR_SIZE;
  memcpy(&seed_number, &linkaddr_node_addr, min_len_seed);
  random_init(seed_number);
  #if LOG_DBG_DEVELOPER == 1
  LOG_DBG("Seed is %2.2X (%d), sizeof(Seed) is %u\n", seed_number, seed_number, min_len_seed);
  #endif
//#endif /* CONTIKI_TARGET_SKY */

  clock_time_t hello_start_time = IOTORII_HELLO_START_TIME * CLOCK_SECOND;
  hello_start_time = hello_start_time / 2 + (random_rand() % (hello_start_time / 2));
  LOG_DBG("Scheduling a Hello message after %u ticks in the future\n", (unsigned)hello_start_time);
  ctimer_set(&hello_timer, hello_start_time, iotorii_handle_hello_timer, NULL);

  //Create Neighbour table
  number_of_neighbours = 0;
  hlmac_table_init();
  //Timer to collect statistics
  #if LOG_DBG_STATISTIC == 1
  clock_time_t statistic_start_time = IOTORII_STATISTICS_TIME * CLOCK_SECOND;
  LOG_DBG("Scheduling a statistic timer after %u ticks in the future\n", (unsigned)statistic_start_time);
  ctimer_set(&statistic_timer, statistic_start_time, iotorii_handle_statistic_timer, NULL);
  #endif
#endif

#if IOTORII_NODE_TYPE == 1 //Root node, we set a timer to send a SetHLMAC address.
  clock_time_t sethlmac_start_time;
  sethlmac_start_time = IOTORII_SETHLMAC_START_TIME * CLOCK_SECOND;
  LOG_DBG("Scheduling a SetHLMAC message after %u ticks in the future\n", (unsigned)sethlmac_start_time);
  ctimer_set(&sethlmac_timer, sethlmac_start_time, iotorii_handle_sethlmac_timer, NULL);
#endif
//EXTRA END
}
/*---------------------------------------------------------------------------*/
//EXTRA BEGIN
#if IOTORII_NODE_TYPE > 0 //Root or Common node
void
iotorii_handle_incoming_hello() //To process an IoTorii Hello control broadcast packet received from other nodes
{
  const linkaddr_t *sender_addr = packetbuf_addr(PACKETBUF_ADDR_SENDER);

  LOG_DBG("A Hello message received from ");
  LOG_DBG_LLADDR(sender_addr);
  LOG_DBG("\n");

  if (number_of_neighbours < 256){
    uint8_t address_is_in_table = 0;
    neighbour_table_entry_t *new_nb;
    //Check if the address is in the list.
    for(new_nb=list_head(neighbour_table_entry_list); new_nb!=NULL; new_nb=new_nb->next){
      if (linkaddr_cmp(&(new_nb->addr), sender_addr)){ //This condithion can be merged with the "for" condition.
        address_is_in_table = 1;
      }
    }
    if (!address_is_in_table){
      new_nb = (neighbour_table_entry_t *)malloc(sizeof(neighbour_table_entry_t));
      new_nb->id = ++number_of_neighbours;
      new_nb->addr = *sender_addr;
      list_add(neighbour_table_entry_list, new_nb);
      LOG_DBG("A new neighbour added to IoTorii neighbour table, address: ");
      LOG_DBG_LLADDR(sender_addr);
      LOG_DBG(", ID: %d\n", new_nb->id);
    }else{
      LOG_DBG("Address of hello (");
      LOG_DBG_LLADDR(sender_addr);
      LOG_DBG(") message received already!\n");
    }
  }else{
    LOG_WARN("The IoTorii neighbour table is full! \n");
  }

  #if LOG_DBG_DEVELOPER == 1// || LOG_DBG_STATISTIC == 1
  //Write neighbour table
  neighbour_table_entry_t *nb;
  LOG_DBG("Naighbour Table:");
  for(nb=list_head(neighbour_table_entry_list); nb!=NULL; nb=list_item_next(nb)){
    LOG_DBG("Id: %d, MAC addr:",nb->id);
    LOG_DBG_LLADDR(&nb->addr);
    LOG_DBG(" - ");
  }
  LOG_DBG("\n");
  #endif
}
/*---------------------------------------------------------------------------*/
/* Extract the address of this node from a received packet */
hlmacaddr_t *iotorii_extract_address(void){
//hlmacaddr_t iotorii_extract_address(void){ //For this comment see mentioned solution

  /* Is packetbuf a common repository? By using
   * "packetbuf_ptr = packetbuf_dataptr();", when we apply a process
   * on the packetbuf, if node wants to send a SetHLMAC message (or receive a
   * new packet), packetbuf is overwrote. To avoid this, we create a copy from
   * packetbuf before starting a process on the packetbuf.
   */
  int packetbuf_data_len = packetbuf_datalen();
  uint8_t * packetbuf_ptr_head = (uint8_t *) malloc(sizeof(uint8_t) * packetbuf_data_len);
  memcpy(packetbuf_ptr_head, packetbuf_dataptr(), packetbuf_data_len);
  uint8_t * packetbuf_ptr = packetbuf_ptr_head;
  int datalen_counter = 0;
  //uint8_t pref_len;
  uint8_t id = 0;
  hlmacaddr_t *prefix = NULL;
  linkaddr_t link_address = linkaddr_null;
  uint8_t is_first_record = 1; // The first record of each payload includes a prefix lenght and prefix.

  /* Read "payload" */
  while(datalen_counter < packetbuf_data_len && !linkaddr_cmp(&link_address, &linkaddr_node_addr)){
    if (is_first_record){
      is_first_record = 0;
      /* Prefix lenght */
      prefix = (hlmacaddr_t *) malloc(sizeof(hlmacaddr_t));
      memcpy(&prefix->len, packetbuf_ptr, 1);
      packetbuf_ptr ++;
      datalen_counter ++;
      /* Neighbour prefix */
      prefix->address = (uint8_t *) malloc(sizeof(uint8_t) * prefix->len);
      memcpy(prefix->address, packetbuf_ptr, prefix->len);
      packetbuf_ptr += prefix->len;
      datalen_counter += prefix->len;
    }
    /* Neighbour ID */
    memcpy(&id, packetbuf_ptr, 1);
    packetbuf_ptr ++;
    datalen_counter ++;
    /* MAC address */
    memcpy(&link_address, packetbuf_ptr, LINKADDR_SIZE);
    packetbuf_ptr += LINKADDR_SIZE;
    datalen_counter += LINKADDR_SIZE;
  } // End while

  //Release memory: packetbuf_ptr_head
  free(packetbuf_ptr_head);
  packetbuf_ptr_head = NULL;

/* the follow approach doesn't work correctly because return_value.address is
 * a pointer, and when we use return_value = *prefix; ,
 * return_value.address = prefix->address. By using free(prefix->address),
 * return_value.address refers to a unreliable place in memory.
 *
 * SOLUTION 1:
 * Eliminating free(prefix->address);, so we can free return_value.address
 * at the end (whenever we want to delete  it from the HLMAC table, or if it
 * create a loop)
 *
 * SOLUTION 2:
 * We can return the pointer of prefix instead of the return_value variable,so
 * we can free the allocated memory based on the SOLUTION 1.
 *
 * SOLUTION 3:
 * We can alloacle new memory to the return_value.address,
 * then free(prefix->address); , and delete memory based on SOLUTION 1.
 *
 * SELECTETED SOLUTION: SOLUTION 2
 */

/*  hlmacaddr_t return_value = UNSPECIFIED_HLMAC_ADDRESS;
  if(linkaddr_cmp(&link_address, &linkaddr_node_addr)){
    hlmac_add_new_id(prefix, id);
    return_value = *prefix;
  }
  if(is_first_record == 0){
    free(prefix->address);
    free(prefix);
    prefix = NULL;
  }
*/
  //return return_value;
  if(linkaddr_cmp(&link_address, &linkaddr_node_addr)){
    hlmac_add_new_id(prefix, id);
    return prefix;
  }
  *prefix = UNSPECIFIED_HLMAC_ADDRESS;
  return prefix;
}
/*---------------------------------------------------------------------------*/
/*
 *To process an IoTorii SetHLMAC control broadcast packet received from other
 *nodes
 */
void
iotorii_handle_incoming_sethlamc()
{
  //#if LOG_DBG_DEVELOPER == 1
  LOG_DBG("A SetHLMAC message received from ");
  LOG_DBG_LLADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
  LOG_DBG("\n");
  //#endif

  hlmacaddr_t *received_hlmac_addr;
  linkaddr_t sender_link_address = *packetbuf_addr(PACKETBUF_ADDR_SENDER);
  received_hlmac_addr = iotorii_extract_address();

  if(hlmac_is_unspecified_addr(*received_hlmac_addr)){
    //#if LOG_DBG_DEVELOPER == 1
    LOG_DBG("Packet dosn't any address for me!\n");
    //#endif
  }else{
    //#if LOG_DBG_DEVELOPER == 1
    char *new_hlmac_addr_str = hlmac_addr_to_str(*received_hlmac_addr);
    LOG_DBG("New HLMAC is: %s\n", new_hlmac_addr_str);
    free(new_hlmac_addr_str);
    //#endif

    if(!hlmactable_has_loop(*received_hlmac_addr)){
      uint8_t is_added = hlmactable_add(*received_hlmac_addr);
      if (is_added){
        //#if LOG_DBG_DEVELOPER == 1
        LOG_DBG("New HLMAC address is assigned to the node.\n");
        #if LOG_DBG_DEVELOPER == 1
        LOG_DBG("New HLMAC address is sent to the neighbours.\n");
        #endif
        iotorii_send_sethlmac(*received_hlmac_addr, sender_link_address); //To advertise the prefix
      }else{
        //#if LOG_DBG_DEVELOPER == 1
        LOG_DBG("New HLMAC address not added to the HLMAC table, and memory is free.\n");
        //#endif
        free(received_hlmac_addr->address);
        received_hlmac_addr->address = NULL;
        free(received_hlmac_addr);
        received_hlmac_addr = NULL;
      }
    }else{
      //#if LOG_DBG_DEVELOPER == 1
      LOG_DBG("New HLMAC address not assigned to the node (loop), and memory is free.\n");
      //#endif
      free(received_hlmac_addr->address);
      received_hlmac_addr->address = NULL;
      free(received_hlmac_addr);
      received_hlmac_addr = NULL;
    }
  }
  #if LOG_DBG_STATISTIC == 1
  LOG_DBG("Periodic Statistics: node_id: %u, convergence_time_end\n", node_id);
  #endif

}
/*---------------------------------------------------------------------------*/
void
iotorii_operation(void)
{
  if (packetbuf_holds_broadcast()){
    //if (hello identification) //
    if (packetbuf_datalen() == 0)
      iotorii_handle_incoming_hello();
    //else if (SetHLMAC identification)
    else
      iotorii_handle_incoming_sethlamc();
    //else
      //handle_data_broadcast(); //To send a data broadcast packet, received from another node, to the upper layer
  }//else
    //handle_handle_unicast();  //For handling an unicast packet received from another node
}
#endif //#if IOTORII_NODE_TYPE == 0
//EXTRA END
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
#if CSMA_SEND_SOFT_ACK
  uint8_t ackdata[CSMA_ACK_LEN];
#endif

  if(packetbuf_datalen() == CSMA_ACK_LEN) {
    /* Ignore ack packets */
    LOG_DBG("ignored ack\n");
  } else if(csma_security_parse_frame() < 0) {
    LOG_ERR("failed to parse %u\n", packetbuf_datalen());
  } else if(!linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                         &linkaddr_node_addr) &&
            !packetbuf_holds_broadcast()) {
    LOG_WARN("not for us\n");
  } else if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER), &linkaddr_node_addr)) {
    LOG_WARN("frame from ourselves\n");
  } else {
    int duplicate = 0;

    /* Check for duplicate packet. */
    duplicate = mac_sequence_is_duplicate();
    if(duplicate) {
      /* Drop the packet. */
      LOG_WARN("drop duplicate link layer packet from ");
      LOG_WARN_LLADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
      LOG_WARN_(", seqno %u\n", packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO));
    } else {
      mac_sequence_register_seqno();
    }

#if CSMA_SEND_SOFT_ACK
    if(packetbuf_attr(PACKETBUF_ATTR_MAC_ACK)) {
      ackdata[0] = FRAME802154_ACKFRAME;
      ackdata[1] = 0;
      ackdata[2] = ((uint8_t *)packetbuf_hdrptr())[2];
      NETSTACK_RADIO.send(ackdata, CSMA_ACK_LEN);
    }
#endif /* CSMA_SEND_SOFT_ACK */
    if(!duplicate) {
      LOG_INFO("received packet from ");
      LOG_INFO_LLADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
      LOG_INFO_(", seqno %u, len %u\n", packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO), packetbuf_datalen());
      //EXTRA BEGIN
      #if IOTORII_NODE_TYPE == 0
      NETSTACK_NETWORK.input();
      #elif IOTORII_NODE_TYPE > 0
      iotorii_operation();
      #endif
      //EXTRA END
    }
  }
}
/*---------------------------------------------------------------------------*/
//EXTRA BEGIN
#if IOTORII_NODE_TYPE == 0
const struct mac_driver csma_driver = {
  "CSMA",
#elif IOTORII_NODE_TYPE > 0
const struct mac_driver iotorii_csma_driver = {
  "IoTorii CSMA",
#endif
//EXTRA END
  init,
  send_packet,
  input_packet,
  on,
  off,
  max_payload,
};
/*---------------------------------------------------------------------------*/
