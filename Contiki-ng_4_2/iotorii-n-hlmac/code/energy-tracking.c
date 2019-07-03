#include "sys/energest.h"
#include "energy_constants.h"

PROCESS(energy_tracking, "Energy tracking through the Energest module");

PROCESS_THREAD(energy_tracking, ev, data) {
    static struct etimer energy_timer;

    PROCESS_BEGIN();

    etimer_set(&energy_timer, REFRESH_INTERVAL * CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&energy_timer));
        etimer_reset(&energy_timer);

        energest_flush();

        printf("Uptimes\n#######\n\tCPU times:\n");
        printf("ACTIVE %4lus\tLOW POWER: %4lus\tDEEP SLEEP: %4lus\t TOTAL: %4lus\n",
                (unsigned long) (energest_type_time(ENERGEST_TYPE_CPU) / ENERGEST_SECOND),
                (unsigned long) (energest_type_time(ENERGEST_TYPE_LPM) / ENERGEST_SECOND),
                (unsigned long) (energest_type_time(ENERGEST_TYPE_DEEP_LPM) / ENERGEST_SECOND),
                (unsigned long) (ENERGEST_GET_TOTAL_TIME() / ENERGEST_SECOND));
        printf("\tRADIO times:\nTX: %4lus\tRX: %4lus\tOFF: %4lus\n",
                (unsigned long) (energest_type_time(ENERGEST_TYPE_TRANSMIT) / ENERGEST_SECOND), 
                (unsigned long) (energest_type_time(ENERGEST_TYPE_LISTEN) / ENERGEST_SECOND),
                (unsigned long) ((ENERGEST_GET_TOTAL_TIME() -
                energest_type_time(ENERGEST_TYPE_TRANSMIT) - 
                energest_type_time(ENERGEST_TYPE_LISTEN)) / ENERGEST_SECOND));
        printf("Energy consumptions\n###################\n\tCPU: %lg mJ\n",
                (double) (MCU_VOLTAGE * (energest_type_time(ENERGEST_TYPE_CPU) * ACTIVE_CURRENT +
                          energest_type_time(ENERGEST_TYPE_LPM) * SLEEP_CURRENT +
                          energest_type_time(ENERGEST_TYPE_DEEP_LPM) * DEEP_SLEEP_CURRENT)));
        printf("\t RADIO: %lg mJ\n",
                (double) (energest_type_time(ENERGEST_TYPE_TRANSMIT) * TX_POWER +
                          energest_type_time(ENERGEST_TYPE_LISTEN) * RX_POWER));
    }
    PROCESS_END();
}