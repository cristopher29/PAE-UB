#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>

uint32_t timer_ms_count;

void delay_t(uint32_t temps_ms)
{
    timer_ms_count = 0;                     // Reiniciar Timer ms
    TA0CTL |= MC_1;                         // Se habilita el Timer
    do {

    } while (timer_ms_count <= temps_ms);   // Timer ira sumando +1 a la varibale timer_ms_count hasta llegar al tiempo establecido (Ver función de interrupción de Timer ms)
    TA0CTL &= ~MC_1;                        // Se deshabilita el timer
}

void increase_time(int type, int *sec){
    switch (type) {
        case 1:
            *sec = *sec + 3600;
            if ((*sec / 3600)%24 == 0) {
                *sec = *sec - 86400;
            }
            break;
        case 2:
            *sec = *sec + 60;
            if ((*sec / 60) % 60 == 0) {
                *sec = *sec - 3600;
            }
            break;
        case 3:
            *sec = *sec + 1;
            if (*sec % 60 == 0) {
                *sec = *sec - 60;
            }
            break;
    }
}

void decrease_time(int type, int *sec){
    switch (type) {
        case 1:
            if ((*sec / 3600)%24 == 0) {
                *sec = *sec + 86400;
            }
            *sec = *sec - 3600;
            break;
        case 2:
            if ((*sec / 60) % 60 == 0) {
                *sec = *sec + 3600;
            }
            *sec = *sec - 60;
            break;
        case 3:
            if (*sec % 60 == 0) {
                *sec = *sec + 60;
            }
            *sec = *sec - 1;
            break;
    }
}

