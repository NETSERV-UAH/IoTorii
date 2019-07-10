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
 *
 */

/**
 * \file
 *         This process starts the IoTorii mac protocol
 */

#include "contiki.h"
#include "net/netstack.h"
#include <stdio.h>
#include "sys/energest.h"

/*---------------------------------------------------------------------------*/
PROCESS(start_iotorii_common_node, "Process to Start common node");
PROCESS(energy_tracking, "Energy tracking through the Energest module");
AUTOSTART_PROCESSES(&start_iotorii_common_node, &energy_tracking);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_iotorii_common_node, ev, data)
{

  PROCESS_BEGIN();
  
  NETSTACK_RADIO.on();
  //NETSTACK_MAC.init(); //MAC is initiated twice
  //netstack_init(); //MAC is initiated twice


  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(energy_tracking, ev, data) {
    static struct etimer energy_timer;

    PROCESS_BEGIN();

    etimer_set(&energy_timer, REFRESH_INTERVAL * CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&energy_timer));
        etimer_reset(&energy_timer);

        energest_flush();

        printf("Uptimes\n#######\n\tCPU times:\n");
        printf("\t\tACTIVE: %4lus\tLOW POWER: %4lus\tDEEP SLEEP: %4lus\t TOTAL: %4lus\n\n",
                (unsigned long) (energest_type_time(ENERGEST_TYPE_CPU) / ENERGEST_SECOND),
                (unsigned long) (energest_type_time(ENERGEST_TYPE_LPM) / ENERGEST_SECOND),
                (unsigned long) (energest_type_time(ENERGEST_TYPE_DEEP_LPM) / ENERGEST_SECOND),
                (unsigned long) (ENERGEST_GET_TOTAL_TIME() / ENERGEST_SECOND));
        printf("\tRADIO times:\n\t\tTX: %4lus\tRX: %4lus\tOFF: %4lus\n\n",
                (unsigned long) (energest_type_time(ENERGEST_TYPE_TRANSMIT) / ENERGEST_SECOND), 
                (unsigned long) (energest_type_time(ENERGEST_TYPE_LISTEN) / ENERGEST_SECOND),
                (unsigned long) ((ENERGEST_GET_TOTAL_TIME() -
                energest_type_time(ENERGEST_TYPE_TRANSMIT) - 
                energest_type_time(ENERGEST_TYPE_LISTEN)) / ENERGEST_SECOND));
        printf("Energy consumptions\n###################\n\tCPU: %lg mJ\n",
                (double) (MCU_VOLTAGE * (energest_type_time(ENERGEST_TYPE_CPU) * ACTIVE_CURRENT +
                          energest_type_time(ENERGEST_TYPE_LPM) * SLEEP_CURRENT +
                          energest_type_time(ENERGEST_TYPE_DEEP_LPM) * DEEP_SLEEP_CURRENT)));
        printf("\tRADIO: %lg mJ\n",
                (double) (energest_type_time(ENERGEST_TYPE_TRANSMIT) * TX_POWER +
                          energest_type_time(ENERGEST_TYPE_LISTEN) * RX_POWER));
    }
    PROCESS_END();
}