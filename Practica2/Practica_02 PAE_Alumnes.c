/******************************
 * 
 * Practica_02_PAE Programació de Ports
 * i pràctica de les instruccions de control de flux:
 * "do ... while", "switch ... case", "if" i "for"
 * UB, 02/2017.
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h"> //Libreria grafica + configuracion reloj MSP432

char saludo[16] = " PRACTICA 2 PAE"; //max 15 caracteres visibles
char cadena[16]; //Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 1;
uint8_t estado = 0;
uint8_t estado_anterior = 8;
uint32_t retraso = 500;

/**************************************************************************
 * INICIALIZACIÓN DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_interrupciones()
{
    // Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
    // accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
    // Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto
    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACIÓN DE LA PANTALLA LCD.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

/**************************************************************************
 * BORRAR LINEA
 * 
 * Datos de entrada: Linea, indica la linea a borrar
 * 
 * Sin datos de salida
 * 
 **************************************************************************/
void borrar(uint8_t Linea)
{
    halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

/**************************************************************************
 * ESCRIBIR LINEA
 * 
 * Datos de entrada: Linea, indica la linea del LCD donde escribir
 * 					 String, la cadena de caracteres que vamos a escribir
 * 
 * Sin datos de salida
 * 
 **************************************************************************/
void escribir(char String[], uint8_t Linea)

{
    halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

/**************************************************************************
 * INICIALIZACIÓN DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
 * 
 * Sin datos de entrada
 * 
 * Sin datos de salida
 * 
 **************************************************************************/
void init_botons(void)
{
    //Configuramos botones y leds
    //***************************

    //Leds RGB del MK II:
    P2DIR |= 0x50;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
    P5DIR |= 0x40;  //Pin P5.6 (B)como salida Led (RGB)
    P2OUT &= 0xAF;  //Inicializamos Led RGB a 0 (apagados)
    P5OUT &= ~0x40; //Inicializamos Led RGB a 0 (apagados)

    //Boton S1 del MK II:
    P5SEL0 &= ~0x02;   //Pin P5.1 como I/O digital,
    P5SEL1 &= ~0x02;   //Pin P5.1 como I/O digital,
    P5DIR &= ~0x02; //Pin P5.1 como entrada
    P5IES &= ~0x02;   // con transicion L->H
    P5IE |= 0x02;     //Interrupciones activadas en P5.1,
    P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 5
    //P5REN: Ya hay una resistencia de pullup en la placa MK II

    //Boton S2 del MK II:
    P3SEL0 &= ~0x20;   //Pin P3.5 como I/O digital,
    P3SEL1 &= ~0x20;   //Pin P3.5 como I/O digital,
    P3DIR &= ~0x20; //Pin P3.5 como entrada
    P3IES &= ~0x20;   // con transicion L->H
    P3IE |= 0x20;   //Interrupciones activadas en P3.5
    P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3
    //P3REN: Ya hay una resistencia de pullup en la placa MK II

    //Configuramos los GPIOs del joystick del MK II:
    P4DIR &= ~(BIT1 + BIT5 + BIT7 );   //Pines P4.1, 4.5 y 4.7 como entrades,
    P4SEL0 &= ~(BIT1 + BIT5 + BIT7 ); //Pines P4.1, 4.5 y 4.7 como I/O digitales,
    P4SEL1 &= ~(BIT1 + BIT5 + BIT7 );
    P4REN |= BIT1 + BIT5 + BIT7;  //con resistencia activada
    P4OUT |= BIT1 + BIT5 + BIT7;  // de pull-up
    P4IE |= BIT1 + BIT5 + BIT7;   //Interrupciones activadas en P4.1, 4.5 y 4.7,
    P4IES &= ~(BIT1 + BIT5 + BIT7 ); //las interrupciones se generaran con transicion L->H
    P4IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4

    P5DIR &= ~(BIT4 + BIT5 );  //Pines P5.4 y 5.5 como entrades,
    P5SEL0 &= ~(BIT4 + BIT5 ); //Pines P5.4 y 5.5 como I/O digitales,
    P5SEL1 &= ~(BIT4 + BIT5 );
    P5IE |= BIT4 + BIT5;  //Interrupciones activadas en 5.4 y 5.5,
    P5IES &= ~(BIT4 + BIT5 ); //las interrupciones se generaran con transicion L->H
    P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4
    // - Ya hay una resistencia de pullup en la placa MK II
}

/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void delay_t(uint32_t temps)
{
    volatile uint32_t i = 0;

    /**************************
     * A RELLENAR POR EL ALUMNO
     **************************/
    while (i < temps * 1000)
    {
        i++;
    }

}

/*****************************************************************************
 * CONFIGURACIÓN DEL PUERTO 7. A REALIZAR POR EL ALUMNO
 * 
 * Sin datos de entrada
 * 
 * Sin datos de salida
 *  
 ****************************************************************************/
void config_P7_LEDS(void)
{

    //A RELLENAR POR EL ALUMNO
    P7DIR |= 0xFF;          // Iniciamos los LEDs (P7) como salidas
    P7SEL1 |= 0x00;         // y tendran la función por defecto
    P7SEL0 |= 0x00;
    P7OUT = 0x00;           // Apagamos todos los LEDs (P7)
}

void main(void)
{

    WDTCTL = WDTPW + WDTHOLD; // Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_16MHz();			//Ajustes del clock (Unified Clock System)
    init_botons();				//Configuramos botones y leds

    config_P7_LEDS();			//Configuramos los LEDs del Puerto 7
    uint32_t delay_P7 = 250;	//Retraso para los LEDs del Puerto 7

    init_interrupciones();		//Configurar y activar las interrupciones de los botones
    init_LCD();					//Inicializamos la pantalla
    halLcdPrintLine(saludo, linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;					//Aumentamos el valor de linea y con ello pasamos a la linea siguiente
    
	int estado_P7 = 2;			//La dirección de los LEDs del Puerto 7 (2 = Ninguna, 1 = Der->Izq, 0 = Izq->Der)

    //Bucle principal (infinito):
    do
    {
        if(estado_P7 == 1){				// LEDS DER->IZQ
            P7OUT >>= 1;				// DIVIDIR - desplazamiento hacia la derecha de los bits
            if (P7OUT == 0x00)			// Reiniciar desplazamiento 
            {
                P7OUT = 0x80;
            }
            delay_t(delay_P7);
        }else if(estado_P7 == 0){		// LEDS IZQ->DER
            P7OUT <<= 1;				// MULTIPLICAR - desplazamiento hacia la izquierda de los bits
            if (P7OUT == 0x00)			// Reiniciar desplazamiento 
            {
                P7OUT = 0x01;
            }
            delay_t(delay_P7);
        }

        if (estado_anterior != estado)				// Dependiendo del valor del estado se encenderá un LED u otro.
        {
            sprintf(cadena, " estado %d", estado);	// Guardamos en cadena la siguiente frase: estado "valor del estado"
            escribir(cadena, linea);				// Escribimos la cadena al LCD
            estado_anterior = estado;				// Actualizamos el valor de estado_anterior, para que no esté siempre escribiendo.

            /**********************************************************+
             A RELLENAR POR EL ALUMNO BLOQUE switch ... case
             Para gestionar las acciones:
             Boton S1, estado = 1
             Boton S2, estado = 2
             Joystick left, estado = 3
             Joystick right, estado = 4
             Joystick up, estado = 5
             Joystick down, estado = 6
             Joystick center, estado = 7
             ***********************************************************/

            switch (estado)
            {
            case 1:
                P2OUT |= (BIT6 + BIT4 );	// LED R Y LED G (1,1)
                P5OUT |= BIT6;				// LED B (1)
                break;
            case 2:
                P2OUT &= ~(BIT6 + BIT4 );	// LED R Y LED G (0,0)
                P5OUT &= ~BIT6;				// LED B (0)
                break;
            case 3:
                P2OUT |= (BIT6 + BIT4 );	// LED R Y LED G (1,1)
                P5OUT |= BIT6;				// LED B (1)
                P7OUT = 0x80;
				estado_P7=1;				// LEDs P7 de Der->Izq
                break;
            case 4:
                P2OUT |= (BIT6 + BIT4 );	// LED R Y LED G (1,1)
                P5OUT &= ~BIT6;				// LED B (0)
                P7OUT = 0x01;
				estado_P7=0;				// LEDs P7 de Izq->Der
                break;
            case 5:
                P2OUT |= BIT6;				// LED R (1)
                P2OUT &= ~BIT4;				// LED G (0)
                P5OUT |= BIT6;				// LED B (1)
                if (delay_P7 > 100){		// Manteniendo la dirección en la que se encienden los LEDS (fijada en case 3 o case 4), decrementar la velocidad (con un límite inferior)
                    delay_P7 = delay_P7 - 100; // Menos delay -> Mayor velocidad
                }
                break;
            case 6:
                P2OUT &= ~BIT6;				// LED R (0)
                P2OUT |= BIT4;				// LED G (1)
                P5OUT |= BIT6;				// LED B (1)
                if (delay_P7 < 2000){		// Manteniendo la dirección en la que se encienden los LEDS (fijada en case 3 o case 4), incrementar la velocidad (con un límite superior)
                    delay_P7 = delay_P7 + 100; // Más delay -> Menor velocidad
                }
                break;
            case 7:
                P2OUT ^= BIT6;				// Conmutamos el estado del LED R (bit 6)
                P2OUT ^= BIT4;				// Conmutamos el estado del LED G (bit 4)
                P5OUT ^= BIT6;				// Conmutamos el estado del LED B (bit 6)
                break;
            default:
                break;
            }

        }

        /*
         P2OUT ^= BIT6;		// Conmutamos el estado del LED R (bit 6)
         delay_t(retraso);	// periodo del parpadeo
         P2OUT ^= BIT4;		// Conmutamos el estado del LED G (bit 4)
         delay_t(retraso);	// periodo del parpadeo
         P5OUT ^= BIT6;	    // Conmutamos el estado del LED B (bit 6)
         delay_t(retraso);  // periodo del parpadeo
         */
    }
    while (1); //Condicion para que el bucle sea infinito
}

/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectará qué botón se ha pulsado
 * 		 
 * Sin Datos de entrada
 * 
 * Sin datos de salida
 * 
 * Actualizar el valor de la variable global estado
 * 
 **************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void)
{ //interrupcion del pulsador S2

    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= ~BIT5;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    if (flag == 0x0C)
    { // Pulsador S2 P3.5
        estado = 2;
    }

    P3IE |= BIT5;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void)
{ //interrupción de los botones. Actualiza el valor de la variable global estado.

    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= ~(BIT7 | BIT5 | BIT1 ); //interrupciones Joystick en port 4 desactivadas (0x5D)
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO BLOQUE switch ... case
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    switch (flag)
    {
    case 0x0C: // Joystick Derecha P4.5
        estado = 4;
        break;
    case 0x10: // Joystick Izquierda P4.7
        estado = 3;
        break;
    case 0x04: // Joystick Centro P4.1
        estado = 7;
        break;
    default:
        break;
    }

    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P4IE |= (BIT7 | BIT5 | BIT1 ); //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void)
{ //interrupción de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= ~(BIT4 | BIT5 | BIT1 ); //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior = 0;

    /**********************************************************+
     A RELLENAR POR EL ALUMNO BLOQUE switch ... case
     Para gestionar los estados:
     Boton S1, estado = 1
     Boton S2, estado = 2
     Joystick left, estado = 3
     Joystick right, estado = 4
     Joystick up, estado = 5
     Joystick down, estado = 6
     Joystick center, estado = 7
     ***********************************************************/

    switch (flag)
    {
    case 0x04:  // Pulsador S1
        estado = 1;
        break;
    case 0x0A: // Joystick Arriba
        estado = 5;
        break;
    case 0x0C: // Joystick Abajo
        estado = 6;
        break;
    default:
        break;
    }

    /***********************************************
     * HASTA AQUI BLOQUE CASE
     ***********************************************/

    P5IE |= (BIT4 | BIT5 | BIT1 ); //interrupciones Joystick y S1 en port 5 reactivadas
}
