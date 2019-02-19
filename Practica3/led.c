#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>


void init_leds_p7(void)
{

    //A RELLENAR POR EL ALUMNO
    P7SEL1 |= 0x00;         // Función por defecto
    P7SEL0 |= 0x00;
    P7DIR |= 0xFF;          // Iniciamos los LEDs (P7) como salidas
    P7OUT = 0x00;           // Apagamos todos los LEDs (P7)
}

void init_led_rgb(void)
{
    //Leds RGB del MK II:
    P2DIR |= 0x50;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
    P5DIR |= 0x40;  //Pin P5.6 (B)como salida Led (RGB)
    P2OUT &= 0xAF;  //Inicializamos Led RGB a 0 (apagados)
    P5OUT &= ~0x40; //Inicializamos Led RGB a 0 (apagados)

}

void led_rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
    if(r){
        P2OUT |= BIT6;
    }else{
        P2OUT &= ~BIT6;
    }
    if(g){
        P2OUT |= BIT4;
    }else{
        P2OUT &= ~BIT4;
    }
    if(b){
        P5OUT |= BIT6;
    }else{
        P5OUT &= ~BIT6;
    }
}
void led_rgb_toggle(void)
{
    P2OUT ^= BIT6;              // Conmutamos el estado del LED R (bit 6)
    P2OUT ^= BIT4;              // Conmutamos el estado del LED G (bit 4)
    P5OUT ^= BIT6;              // Conmutamos el estado del LED B (bit 6)
}
