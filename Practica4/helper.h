#ifndef HELPER_H_
#define HELPER_H_

typedef uint8_t byte;
typedef int bool;

extern uint32_t timer_ms_count;

#define false 0
#define true 1

#define LEFT 0
#define RIGHT 1

#define LEFT_WHEEL 3
#define RIGHT_WHEEL 2

// Colores para el LED RGB
#define RED 0x40
#define GREEN 0x10
#define BLUE 0x40

// --- Botones ---

#define BUTTON_S1 1
#define BUTTON_S2 2
#define JOYSTICK_LEFT 3
#define JOYSTICK_RIGHT 4
#define JOYSTICK_UP 5
#define JOYSTICK_DOWN 6
#define JOYSTICK_CENTER 7

// --- UART  ---

typedef struct RxReturn{
    byte StatusPacket[16];
    byte time_out;
    bool error;
} RxReturn;

#define TXD2_READY (UCA2IFG & UCTXIFG)

// --- Instrucciones ---

#define WRITE_DATA 0x03
#define READ_DATA 0x02
#define BROADCASTING 0xFE

// --- Control Table ---

// EEPROM AREA - Direcciones de inicio
#define CW_ANGLE_LIMIT_L 0x06
#define CW_ANGLE_LIMIT_H 0x07
#define CCW_ANGLE_LIMIT_L 0x08
#define CCW_ANGLE_LIMIT_H 0x09


// RAM AREA - Direcciones de inicio
#define MOV_SPEED_L 0x20
#define MOV_SPEED_H 0x21
#define M_LED 0x19
#define LEFT_SENSOR 0x1A
#define CENTER_SENSOR 0x1B
#define RIGHT_SENSOR 0x1C

// Funciones para el tiempo (Opcional)
void increase_time(int type, int *sec);
void decrease_time(int type, int *sec);
void delay_t(uint32_t temps_ms);

#endif /* HELPER_H_ */
