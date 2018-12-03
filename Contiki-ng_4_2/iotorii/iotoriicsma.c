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
#include "iotoriicsma.h"
#include "os/lib/list.h"
#include "sys/ctimer.h"
#include "lib/list.h"
#include <stdlib.h> //For malloc()
#include "lib/random.h"
//EXTRA END
#include "net/mac/mac-sequence.h"
#include "net/packetbuf.h"
#include "net/netstack.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "CSMA"
#define LOG_LEVEL LOG_LEVEL_MAC

#if IOTORII_NODE_TYPE == 1 //only root
static struct ctimer sethlmac_timer;
#endif
#if IOTORII_NODE_TYPE > 0 //root and common node
static struct ctimer hello_timer;
#endif

//EXTRA BEGIN
//Create Neighbour table
LIST(iotorii_nd_table);
//number_of_neighbours = 0;
//LIST(iotorii_hlmac_table);
//EXTRA END

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
      //NETSTACK_NETWORK.input();
      //iotorii_operation();
      //EXTRA END
    }
  }
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
//EXTRA BEGIN
#if IOTORII_NODE_TYPE == 1
static void
handle_sethlmac_timer()
{
  //ctimer_reset(&timer);
}
#endif
/*---------------------------------------------------------------------------*/
#if IOTORII_NODE_TYPE > 0
static void
handle_hello_timer()
{
  //ctimer_reset(&timer);
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
  LOG_DBG("Hello prepared to send\n");
  send_packet(NULL,NULL);
  ctimer_reset(&hello_timer); //Restart the timer from the previous expire time.
  //ctimer_restart(&hello_timer); //Restart the timer from current time.
  //ctimer_stop(&hello_timer); //Stop the timer.


}
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
  //Common node operation
#ifdef IOTORII_NODE_TYPE
//LIST
#if IOTORII_NODE_TYPE == 1 //Root node, we set a timer to send a SetHLMAC address.
  LOG_INFO("This node operates as the root.");
  clock_time_t sethlmac_start_time;
  sethlmac_start_time = CLOCK_SECOND; //after 2 seconds
  LOG_DBG("Scheduling SetHLMAC after %u ticks in the future\n", (unsigned)sethlmac_start_time);
  ctimer_set(&sethlmac_timer, sethlmac_start_time, handle_sethlmac_timer, NULL);
#endif
#if IOTORII_NODE_TYPE > 0 //Root or Common node, we set a timer to send a Hello message.
  LOG_INFO("This node operates as a common node.");
  clock_time_t hello_start_time;
  hello_start_time = random_rand() % (3) * CLOCK_SECOND;//after 1 second
  LOG_DBG("Scheduling Hello after %u ticks in the future\n", (unsigned)hello_start_time);
  ctimer_set(&hello_timer, hello_start_time, handle_hello_timer, NULL);

  //Create Neighbour table
  list_init(iotorii_nd_table);
  number_of_neighbours = 0;
  //LIST(iotorii_hlmac_table);

#endif
#endif
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
const struct mac_driver csma_driver = {  //const struct mac_driver csma_driver = { //EXTRA
  "CSMA",
  init,
  send_packet,
  input_packet,
  on,
  off,
  max_payload,
};
/*---------------------------------------------------------------------------*/
//EXTRA BEGIN
void
handle_hello() //To process an IoTorii Hello control broadcast packet received from another node
{
  if (number_of_neighbours < 256){
    uint8_t address_is_in_table = 0;
    const linkaddr_t *sender_addr = packetbuf_addr(PACKETBUF_ADDR_SENDER);
    struct neighbour_table_entry *new_nb;
    //Check if the address is in the list.
    for(new_nb=list_head(iotorii_nd_table); new_nb!=NULL; new_nb=new_nb->next){
      if (linkaddr_cmp(&(new_nb->addr), sender_addr))
        address_is_in_table = 1;
    }
    if (!address_is_in_table){
      new_nb = (struct neighbour_table_entry *)malloc(sizeof(struct neighbour_table_entry));
      new_nb->id = ++number_of_neighbours;
      new_nb->addr = *sender_addr;
      list_add(iotorii_nd_table, new_nb);
      LOG_DBG("A new neighbour was added to IoTorii neighbour table, address: ");
      LOG_DBG_LLADDR(sender_addr);
      LOG_DBG(", ID: %d\n", new_nb->id);
    }else
      LOG_ERR("Address of hello (");
      LOG_DBG_LLADDR(sender_addr);
      LOG_DBG(") message had been received already!\n");
  } else{
    LOG_ERR ("The IoTorii neighbour table is full! \n");
  }
}
/*---------------------------------------------------------------------------*/
void
handle_sethlamc() //To process an IoTorii SetHLMAC control broadcast packet received from another node
{
  //maxjitter 5 ms
  //jitter = 0~5ms


}
/*---------------------------------------------------------------------------*/
void
iotorii_operation(void)
{
  if (packetbuf_holds_broadcast()){
    //if (hello identification) //
    if (packetbuf_datalen() == 0)
      handle_hello();
    //else if (SetHLMAC identification)
    else
      handle_sethlamc();
    //else
      //handle_data_broadcast(); //To send a data broadcast packet, received from another node, to the upper layer
  }//else
    //handle_handle_unicast();  //For handling an unicast packet received from another node
}
/*---------------------------------------------------------------------------*/
