#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>

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

