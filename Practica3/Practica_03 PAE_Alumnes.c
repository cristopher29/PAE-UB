/******************************
 * 
 * Practica_03_PAE Timers
 * UB, 2018.
 *
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432

// Librerias de ayuda
#include <helper.h>
#include <interrupt.h>
#include <button.h>
#include <lcd.h>
#include <led.h>

char saludo[16] = " PRACTICA 3 PAE"; //max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)

uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
int estado_P7 = 0;          //La dirección de los LEDs del Puerto 7 (0 = Ninguna, 1 = Der->Izq, 2 = Izq->Der)
uint32_t delay_P7 = 500;    //Retraso para los LEDs del Puerto 7

// Variables Práctica 3
uint32_t timer_ms_count = 0;
int sec=0; // Reloj
int sec_a=0; // Alarma
int estado_temp = 0;
int modificando_hms = 1; // 1-> Hora, 2-> Min, 3-> Sec

/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void delay_t(uint32_t temps_ms)
{
    timer_ms_count = 0;                     // Reiniciar Timer ms
    TA0CTL |= MC_1;                         // Se habilita el Timer
    do {

    } while (timer_ms_count <= temps_ms);   // Timer ira sumando +1 a la varibale timer_ms_count hasta llegar al tiempo establecido (Ver función de interrupción de Timer ms)
    TA0CTL &= ~MC_1;                        // Se deshabilita el timer
}


void init_timers(void){

    // De momento deshabilitamos el timer de 1 ms, ya que solo se activara al utilizar la función delay_t
    // Timer a 1ms
    TA0CTL |= TASSEL_1 + TACLR;                 // Usamos ACLK (2^15 Hz), Reiniciamos el contador
    TA0CCTL0 &= ~(CAP+CCIFG);                   // Deshabilitamos el modo "capture" (Trabajamos en modo "compare") y quitamos el flag de interrupción
    TA0CCTL0 |= CCIE;                           // Habilitamos las interrupciones (Para CCR0)
    TA0CCR0 = 32;                               // 32768hz / 32 = 1024hz ~= 1ms

    // Timer a 1sec
    TA1CTL |= TASSEL_1 + MC_1 + TACLR;          // Usamos ACLK (2^15 Hz) , Reiniciamos el contador
    TA1CCTL0 &= ~(CAP+CCIFG);                   // Deshabilitamos el modo "capture" (Trabajamos en modo "compare") y quitamos el flag de interrupción
    TA1CCTL0 |= CCIE;                           // Habilitamos las interrupciones (Para CCR0)
    TA1CCR0 = 32767;                            // 32768hz / 32767 ~= 1hz ~= 1000ms ~= 1sec

}

void main(void)
{

    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();			//Ajustes del clock (Unified Clock System)
    init_botons();				//Configuramos botones
    init_led_rgb();             //Configuramos LED RGB
    init_leds_p7();             //Configuramos los LEDs del Puerto 7
    init_timers();              //Inicializamos los timers

    init_interrupciones();		//Configurar y activar las interrupciones de los botones
    init_LCD();					//Inicializamos la pantalla
    halLcdPrintLine(saludo, linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;					//Aumentamos el valor de linea y con ello pasamos a la linea siguiente
    


    //Bucle principal (infinito):
    do
    {

        if (estado_anterior != estado)				// Dependiendo del valor del estado se encenderá un LED u otro.
        {
            estado_anterior = estado;               // Actualizamos el valor de estado_anterior, para que no esté siempre escribiendo.

            switch (estado)
            {
            case BUTTON_S1:                         // Modificar hora

                led_rgb_set(1,1,1);                 // LED R (1) LED G (1) LED B (1)

                TA1CTL &= ~MC_1;                    // Deshabilitamos el Timer del reloj

                break;
            case BUTTON_S2:                         // Modificar alarma

                led_rgb_set(0,0,0);                 // LED R (0) LED G (0) LED B (0)
                borrar(linea+4);                    // Borramos " Es la hora!"
                break;
            case JOYSTICK_LEFT:

                led_rgb_set(1,1,1);                 // LED R (1) LED G (1) LED B (1)

                P7OUT = 0x80;
				estado_P7=1;		                // LEDs P7 de Der->Izq

				// 1-> Hora, 2-> Min, 3-> Sec
				if (modificando_hms>1 && estado_temp != 0){
                    modificando_hms--;
                }
				break;
            case JOYSTICK_RIGHT:

                led_rgb_set(1,1,0);                 // LED R (1) LED G (1) LED B (0)

                P7OUT = 0x01;
				estado_P7=2;				        // LEDs P7 de Izq->Der

				// 1-> Hora, 2-> Min, 3-> Sec
				if (modificando_hms<3 && estado_temp != 0){
				    modificando_hms++;
				}
                break;
            case JOYSTICK_UP:

                led_rgb_set(1,0,1);                 // LED R (1) LED G (0) LED B (1)

                if(estado_temp == 1){               // Modificando Hora
                    increase_time(modificando_hms, &sec);
                }else if(estado_temp==2){           // Modificando Alarma
                    increase_time(modificando_hms, &sec_a);
                }else if(estado_temp==0){
                    if (delay_P7 > 100){            // Manteniendo la dirección en la que se encienden los LEDS (fijada en case 3 o case 4), decrementar la velocidad (con un límite inferior)
                        delay_P7 = delay_P7 - 100;  // Menos delay -> Mayor velocidad
                    }
                }
                break;
            case JOYSTICK_DOWN:

                led_rgb_set(0,1,1);

                if(estado_temp == 1){               // Modificando Hora
                    decrease_time(modificando_hms, &sec);
                }else if(estado_temp==2){
                    decrease_time(modificando_hms, &sec_a);
                }else if(estado_temp==0){
                    if (delay_P7 < 1000){           // Manteniendo la dirección en la que se encienden los LEDS (fijada en case 3 o case 4), incrementar la velocidad (con un límite superior)
                        delay_P7 = delay_P7 + 100;  // Más delay -> Menor velocidad
                    }
                }
                break;
            case JOYSTICK_CENTER:

                led_rgb_toggle();

                if (estado_temp == 1){              // S1
                    TA1CTL |= MC_1;                 // Habilitamos el Timer del reloj
                }
                estado_temp=0;
                break;
            default:
                break;
            }

            sprintf(cadena, " estado %d", estado);  // Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena, linea);                // Escribimos la cadena al LCD
            borrar(linea+1);
            sprintf(cadena, " vel : %d ms", delay_P7);
            escribir(cadena, linea+1);


        }

        sprintf(cadena, " Hora: %02d:%02d:%02d", (sec / 3600) % 24, (sec / 60) % 60, sec % 60);
        escribir(cadena, linea+2);

        sprintf(cadena, " Alar: %02d:%02d:%02d", (sec_a / 3600) % 24, (sec_a / 60) % 60, sec_a % 60);
        escribir(cadena, linea+3);

        if(sec>0 && sec == sec_a){ // Si los segundos de la alarma son iguales a los segundos del reloj:
            sprintf(cadena, " Es la hora!");
            escribir(cadena, linea+4);
        }

        if(estado_P7 == 1){             // LEDS DER->IZQ
            P7OUT >>= 1;                // DIVIDIR - desplazamiento hacia la derecha de los bits
            if (P7OUT == 0x00)          // Reiniciar desplazamiento
            {
                P7OUT = 0x80;
            }
            delay_t(delay_P7);
        }else if(estado_P7 == 2){       // LEDS IZQ->DER
            P7OUT <<= 1;                // MULTIPLICAR - desplazamiento hacia la izquierda de los bits
            if (P7OUT == 0x00)          // Reiniciar desplazamiento
            {
                P7OUT = 0x01;
            }
            delay_t(delay_P7);
        }

    }
    while (1); //Condicion para que el bucle sea infinito
}



// Interrupción para el TIMER DE 1 MS
void TA0_0_IRQHandler(void) {

    TA0CCTL0 &= ~CCIE;      // Deshabilitamos interrupciones mientras tratamos esta

    timer_ms_count++;

    TA0CCTL0 &= ~CCIFG;     // Quitamos flag de interrupción
    TA0CCTL0 |= CCIE;       // Habilitamos interrupciones
}

// Interrupción para el TIMER DE 1 SEC
void TA1_0_IRQHandler(void) {

    TA1CCTL0 &= ~CCIE;      // Deshabilitamos interrupciones mientras tratamos esta

    if(sec == 86399){  // 23:59:59
        sec=0;
    }else{
        sec++;
    }

    TA1CCTL0 &= ~CCIFG;     // Quitamos flag de interrupción
    TA1CCTL0 |= CCIE;       // Habilitamos interrupciones
}

//ISR para las interrupciones del puerto 3: (Pulsador S2)
void PORT3_IRQHandler(void)
{
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= ~BIT5;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior = 0;

    if (flag == 0x0C)
    { // Pulsador S2 P3.5
        estado = BUTTON_S2; // Alarma
        estado_temp = estado; // GUARDAMOS EL VALOR ESTADO PARA SABER SI SE HA PRESIONADO EL PULSADOR S2
    }

    P3IE |= BIT5;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4: (Joystick Derecha, Joystick Derecha, Joystick Centro)
void PORT4_IRQHandler(void)
{
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= ~(BIT7 | BIT5 | BIT1 ); //interrupciones Joystick en port 4 desactivadas (0x5D)
    estado_anterior = 0;
    switch (flag)
    {
    case 0x0C: // Joystick Derecha P4.5
        estado = JOYSTICK_RIGHT;
        break;
    case 0x10: // Joystick Izquierda P4.7
        estado = JOYSTICK_LEFT;
        break;
    case 0x04: // Joystick Centro P4.1
        estado = JOYSTICK_CENTER;
        break;
    default:
        break;
    }

    P4IE |= (BIT7 | BIT5 | BIT1 ); //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5: (Pulsador S1, Joystick Arriba, Joystick Abajo)
void PORT5_IRQHandler(void)
{
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= ~(BIT4 | BIT5 | BIT1 ); //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior = 0;

    switch (flag)
    {
    case 0x04:  // Pulsador S1
        estado = BUTTON_S1; // Rejoj
        estado_temp = estado; // GUARDAMOS EL VALOR ESTADO PARA SABER SI SE HA PRESIONADO EL PULSADOR S1
        break;
    case 0x0A: // Joystick Arriba
        estado = JOYSTICK_UP;
        break;
    case 0x0C: // Joystick Abajo
        estado = JOYSTICK_DOWN;
        break;
    default:
        break;
    }

    P5IE |= (BIT4 | BIT5 | BIT1 ); //interrupciones Joystick y S1 en port 5 reactivadas
}
