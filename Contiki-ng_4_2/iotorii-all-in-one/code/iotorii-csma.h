/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), and David Carrascal(1);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran
 *                        Polytechnic), Iran.
 * Adapted to use IoTorii, a link layer protocol for Low pawer and Lossy
 * Network (LLN), over the IEEE 802.15.4 standard CSMA protocol (nonbeacon-enabled).
 *
 */
/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
//#ifndef CSMA_H_
//#define CSMA_H_

#ifndef IOTORII_CSMA_H_
#define IOTORII_CSMA_H_

#include "hlmacaddr.h"
#include "net/linkaddr.h"
//EXTRA END

#include "contiki.h"
#include "net/mac/mac.h"
#include "dev/radio.h"

//EXTRA
#ifdef IOTORII_CONF_NODE_TYPE
#define IOTORII_NODE_TYPE IOTORII_CONF_NODE_TYPE
#else
#define IOTORII_NODE_TYPE 0 //To support the traditional MAC operation
#endif
//EXTRA

#ifdef CSMA_CONF_SEND_SOFT_ACK
#define CSMA_SEND_SOFT_ACK CSMA_CONF_SEND_SOFT_ACK
#else /* CSMA_CONF_SEND_SOFT_ACK */
#define CSMA_SEND_SOFT_ACK 0
#endif /* CSMA_CONF_SEND_SOFT_ACK */

#ifdef CSMA_CONF_ACK_WAIT_TIME
#define CSMA_ACK_WAIT_TIME CSMA_CONF_ACK_WAIT_TIME
#else /* CSMA_CONF_ACK_WAIT_TIME */
#define CSMA_ACK_WAIT_TIME                      RTIMER_SECOND / 2500
#endif /* CSMA_CONF_ACK_WAIT_TIME */

#ifdef CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#define CSMA_AFTER_ACK_DETECTED_WAIT_TIME CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME
#else /* CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME */
#define CSMA_AFTER_ACK_DETECTED_WAIT_TIME       RTIMER_SECOND / 1500
#endif /* CSMA_CONF_AFTER_ACK_DETECTED_WAIT_TIME */

#define CSMA_ACK_LEN 3

/* Default MAC len for 802.15.4 classic */
#ifdef  CSMA_MAC_CONF_LEN
#define CSMA_MAC_LEN CSMA_MAC_CONF_LEN
#else
#define CSMA_MAC_LEN 127 - 2
#endif

//EXTRA BEGIN
struct neighbour_table_entry{
  struct neighbour_table_entry *next;
  linkaddr_t addr;
  uint8_t id;
};
typedef struct neighbour_table_entry neighbour_table_entry_t;
uint8_t number_of_neighbours;
//EXTRA END

/* just a default - with LLSEC, etc */
#define CSMA_MAC_MAX_HEADER 21

//EXTRA BEGIN
#if IOTORII_NODE_TYPE == 0 //The traditional MAC operation
extern const struct mac_driver csma_driver;
#elif IOTORII_NODE_TYPE > 0 // 1 => the root node, 2 => the common nodes
extern const struct mac_driver iotori_csma_driver;
#endif
//EXTRA END

/* CSMA security framer functions */
int csma_security_create_frame(void);
int csma_security_parse_frame(void);

/* key management for CSMA */
int csma_security_set_key(uint8_t index, const uint8_t *key);


#endif /* IOTORII_CSMA_H_ */
