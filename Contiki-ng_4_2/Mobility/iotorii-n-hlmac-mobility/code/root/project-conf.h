/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), and David Carrascal(1);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran
 *                        Polytechnic), Iran.
 * Adapted to use IoTorii, a link layer protocol for Low pawer and Lossy Network
 * (LLN), over the IEEE 802.15.4 none beacon mode.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
//#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
/*---------------------------------------------------------------------------*/
//#define RTIMER_CONF_CLOCK_SIZE 4
/*---------------------------------------------------------------------------*/
/* Configuring the developer debug log */
#define LOG_CONF_DBG_DEVELOPER 0
/*---------------------------------------------------------------------------*/
/* Configuring the statistics debug log */
#define LOG_CONF_DBG_STATISTIC 1
/*---------------------------------------------------------------------------*/
/* Configuring the maximum number of HLMAC addresses a node can get
 * -1 is unlimited
 * default value is 1
 */
#define HLMAC_CONF_MAX_HLMAC 1
//#define HLMAC_CONF_MAX_HLMAC 3
/*---------------------------------------------------------------------------*/
/* Configure the IoTorii root node for iotoriicsma.h */
#define IOTORII_CONF_NODE_TYPE 1
/*---------------------------------------------------------------------------*/
/* Configure the IoTorii common nodes for iotoriicsma.h */
//#define IOTORII_CONF_NODE_TYPE 2
/*---------------------------------------------------------------------------*/
/* Time to send Hello messages
 * Unit : second
 * IoTorii starts to randomly send a hello message in range
 * [IOTORII_CONF_HELLO_START_TIME/2 IOTORII_CONF_HELLO_START_TIME]
 * after initializing a node.
 */
#define IOTORII_CONF_HELLO_START_TIME 4
/*---------------------------------------------------------------------------*/
/* Time to send the first SetHLMAC message by a root node
 * Unit : second
 * IoTorii starts to send a SetHLMAC message
 * at t = IOTORII_CONF_SETHLMAC_START_TIME
 * after initializing the root node.
 */
#define IOTORII_CONF_SETHLMAC_START_TIME 10
/*---------------------------------------------------------------------------*/
/* Delay before sending a SetHLMAC message
 * Unit : tick
 * 1 second is 128 ticks
 * IoTorii uniformly waits for random delay in range
 * [IOTORII_CONF_SETHLMAC_DELAY/2 IOTORII_CONF_SETHLMAC_DELAY]
 * before sending a SetHLMAC in a common node.
 */
#define IOTORII_CONF_SETHLMAC_DELAY 10
/*---------------------------------------------------------------------------*/
/* Time to print statistics
 * Unit : second
 * IoTorii nodes start to print statistic logs on the output
 * at t = IOTORII_CONF_SETHLMAC_START_TIME
 * after initializing a node.
 */
#define IOTORII_CONF_STATISTICS_TIME 20
/*---------------------------------------------------------------------------*/
/* Configure the csma_driver for netstack.h */
#ifndef IOTORII_CONF_NODE_TYPE
#define IOTORII_CONF_NODE_TYPE 1 //Default is root
#endif

#if IOTORII_CONF_NODE_TYPE > 0 //For the IoTorii root ar common nodes
#define NETSTACK_CONF_MAC      iotorii_csma_driver
#elif IOTORII_CONF_NODE_TYPE == 0 //For the traditional MAC operation
#define NETSTACK_CONF_MAC      csma_driver
#endif
/*---------------------------------------------------------------------------*/
/* Configure the routing_driver for netstack.h */
//#define NETSTACK_CONF_ROUTING      nullrouting_driver
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
