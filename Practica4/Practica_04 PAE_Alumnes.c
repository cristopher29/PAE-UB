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
#include <uart.h>
#include <dynamixel.h>

char saludo[16] = " PRACTICA 4 F"; //max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)

uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
int estado_P7 = 0;          //La dirección de los LEDs del Puerto 7 (0 = Ninguna, 1 = Der->Izq, 2 = Izq->Der)
uint32_t delay_P7 = 500;    //Retraso para los LEDs del Puerto 7

int activado = 0;

void init_timers(void){

    // De momento deshabilitamos el timer de 1 ms, ya que solo se activara al utilizar la función delay_t
    // Timer a 1ms
    TA0CTL |= TASSEL_1 + TACLR;                 // Usamos ACLK (2^15 Hz), Reiniciamos el contador
    TA0CCTL0 &= ~(CAP+CCIFG);                   // Deshabilitamos el modo "capture" (Trabajamos en modo "compare") y quitamos el flag de interrupción
    TA0CCTL0 |= CCIE;                           // Habilitamos las interrupciones (Para CCR0)
    TA0CCR0 = 33;                               // 32768hz / 32 = 1024hz ~= 1ms

    // Timer a 1ms (Para el timeout)
    TA1CTL |= TASSEL_1 + MC_1 + TACLR;          // Usamos ACLK (2^15 Hz) , Reiniciamos el contador
    TA1CCTL0 &= ~(CAP+CCIFG);                   // Deshabilitamos el modo "capture" (Trabajamos en modo "compare") y quitamos el flag de interrupción
    TA1CCTL0 |= CCIE;                           // Habilitamos las interrupciones (Para CCR0)
    TA1CCR0 = 33;                               // 32768hz / 32768 ~= 1hz ~= 1000ms ~= 1sec

}

void main(void)
{

    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();			//Ajustes del clock (Unified Clock System)
    init_botons();				//Configuramos botones
    init_timers();              //Inicializamos los timers
    init_UART();                //Inicializamos la UART

    init_interrupciones();		//Configurar y activar las interrupciones de los botones
    init_LCD();					//Inicializamos la pantalla
    halLcdPrintLine(saludo, linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;					//Aumentamos el valor de linea y con ello pasamos a la linea siguiente


    wheelMode();

    int distR, distC, distL;
    int maxDistRam, maxDistRom, obstacle, speed = 500, speed2=300;

    motorLed(2, 1);
    motorLed(3, 1);

    setCompareDistance(100,150);

    maxDistRam = readMaxDist(100, 0x34);
    sprintf(cadena, " max D RAM %3d", maxDistRam);
    escribir(cadena, linea+1);

    maxDistRom = readMaxDist(100, 0x14);
    sprintf(cadena, " max D ROM %3d", maxDistRom);
    escribir(cadena, linea+2);

    //Bucle principal (infinito):
    do
    {

        distC = readSensor(100, CENTER_SENSOR);
        sprintf(cadena, " dist. C %3d", distC);
        escribir(cadena, linea+3);

        distR = readSensor(100, RIGHT_SENSOR);
        sprintf(cadena, " dist. R %3d", distR);
        escribir(cadena, linea+4);

        distL = readSensor(100, LEFT_SENSOR);
        sprintf(cadena, " dist. L %3d", distL);
        escribir(cadena, linea+5);


        if(activado){

            obstacle = getObstacleDetected(100);

            if(obstacle == 0){
                forward(speed);
            }else if(obstacle == BIT0){ // Obs. Left

                turnOnItselfRight(speed);

            }else if(obstacle == BIT1){ // Obs. Center

                if(distL < distR || (distL == 0 && distR == 0))
                {
                    turnOnItselfLeft(speed);
                }else if(distL > distR){
                    turnOnItselfRight(speed);
                }else{
                    backward(speed);
                }

            }else if(obstacle == BIT2){ // Obs. Right

                turnOnItselfLeft(speed);

            }else if(obstacle == (BIT2|BIT0)){ // Obs. Right & Left

                forward(speed);

            }else if(obstacle == (BIT1|BIT0)){ // Obs. Center & Left

                turnOnItselfRight(speed);

            }else if(obstacle == (BIT1|BIT2)){ // Obs. Center & Right

                turnOnItselfLeft(speed);

            }else if(obstacle == (BIT1|BIT2|BIT0)){ // Obs. Right & Center & Left

                stop();
            }
        }


        if (estado_anterior != estado)				// Dependiendo del valor del estado se encenderá un LED u otro.
        {
            estado_anterior = estado;               // Actualizamos el valor de estado_anterior, para que no esté siempre escribiendo.

            switch (estado)
            {
            case BUTTON_S1:                         // Activar robot
                activado = 1;
                break;
            case BUTTON_S2:                         // Desactivar robot
                activado = 0;
                stop();
                break;
            case JOYSTICK_LEFT:

				break;
            case JOYSTICK_RIGHT:

                break;
            case JOYSTICK_UP:


                break;
            case JOYSTICK_DOWN:

                break;
            case JOYSTICK_CENTER:

                break;
            default:
                break;
            }

            sprintf(cadena, " estado %d", estado);  // Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena, linea);                // Escribimos la cadena al LCD
        }

    } while (1); //Condicion para que el bucle sea infinito
}




// Interrupción para el TIMER DE 1 MS
void TA0_0_IRQHandler(void) {

    TA0CCTL0 &= ~CCIE;      // Deshabilitamos interrupciones mientras tratamos esta

    timer_ms_count++;

    TA0CCTL0 &= ~CCIFG;     // Quitamos flag de interrupción
    TA0CCTL0 |= CCIE;       // Habilitamos interrupciones
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
        estado = BUTTON_S1;
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
