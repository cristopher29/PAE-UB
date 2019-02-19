//*****************************************************************************
//
// MSP432 Practica1_PAE_main.c
// C. Serre, UB, 2017
//
//****************************************************************************
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432

#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

//Mis variables globales:
uint8_t estado=0;
uint8_t estado_anterior=8;
uint8_t linea = 0; //Para memorizar en que linea estoy escribiendo en todo momento
extern const Graphics_Image  logoUB8BPP_UNCOMP; //Los datos del logo UB, que estan en "logoUB_256.c"
uint16_t x0, y0; //posicion doonde pintar el logo UB

//Los colores por defecto vienen definidos en la libreria lib_PAE:
// Color_Fondo = COLOR_16_BEIGE;
// Color_Texto = COLOR_16_DARKER_GRAY;
// Color_Fondo_Inv = COLOR_16_BLUE;
// Color_Texto_Inv = COLOR_16_BEIGE;

void init_puertos(void)
{
//Configuramos botones y leds
//***************************

//Led RGB del launchpad:
  P2DIR |= BIT0 | BIT1 | BIT2;	//Puerto P2.0 (R), 2.1 (G) y 2.2 (B) como salidas Led2 (RGB)
  P2OUT &= ~(BIT0 | BIT1 | BIT2);	//Inicializamos Led RGB a 0 (apagados)

//Configuramos los GPIOs del joystick:
  P4DIR &= ~(BIT1 + BIT5 + BIT7);	//Pines P4.1, 4.5 y 4.7 como entrades,
  P4SEL0 &= ~(BIT1 + BIT5 + BIT7);	//Pines P4.1, 4.5 y 4.7 como I/O digitales,
  P4SEL1 &= ~(BIT1 + BIT5 + BIT7);
  P4REN |= BIT1 + BIT5 + BIT7;	//con resistencia activada
  P4OUT |= BIT1 + BIT5 + BIT7;	// de pull-up
  P4IE |= BIT1 + BIT5 + BIT7; 	//Interrupciones activadas en P4.1, 4.5 y 4.7,
  P4IES &= ~(BIT1 + BIT5 + BIT7);	//las interrupciones se generaran con transicion L->H
  P4IFG = 0;	//Limpiamos todos los flags de las interrupciones del puerto 4

  P5DIR &= ~(BIT4 + BIT5);	//Pines P5.4 y 5.5 como entrades,
  P5SEL0 &= ~(BIT4 + BIT5);	//Pines P5.4 y 5.5 como I/O digitales,
  P5SEL1 &= ~(BIT4 + BIT5);
  P5IE |= BIT4 + BIT5; 	//Interrupciones activadas en 5.4 y 5.5,
  P5IES &= ~(BIT4 + BIT5);	//las interrupciones se generaran con transicion L->H
  P5IFG = 0;	//Limpiamos todos los flags de las interrupciones del puerto 4
// - Ya hay una resistencia de pullup en la placa

}

void init_interrupciones(){
	// Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
	// accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
	// Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

void EncenderRGB_Launchpad(uint8_t cuales){
	P2OUT &= ~0x07; //Primero apago los Leds
	P2OUT |= cuales; //y luego enciendo los que me indica el parametro recibido
}

void pintaLogo(Graphics_Image Imagen, uint8_t x0, uint8_t y0){
    uint16_t  w, h;
    w = Imagen.xSize;
    h = Imagen.ySize;
    halLcdDrawImageLut(x0, y0, w, h, Imagen.pPixel, Imagen.pPalette, Imagen.numColors);
}

void borrar_Logo(Graphics_Image Imagen, uint8_t x0, uint8_t y0){
    uint16_t x, y;
    x = x0 + Imagen.xSize;
    y = y0 + Imagen.ySize;
	halLcdFillRect(x0, y0, x, y, Color_Fondo);
}

//Funcion principal del programa:
void main(void)
{
	//Variables locales de mi funcion principal:
	uint8_t RGB = 0;
	volatile uint32_t i;  // Volatile to prevent optimization
	//Un vector de cadenas con "etiquetas" para mostrar en mi LCD:
	char *Jstick[] = {"Boton: ninguno.", "S1 Launchpad   ", "S2 Launchpad   ",
	  				  "Jstck Left     ", "Jstck Right    ",
	  				  "Jstck Up       ", "Jstck Down     ",
					  "Jstck Center   ",
	  	  	  	  	  "Boton_S1_MKII  ", "Boton_S2_MKII  "};
	//Nota: estas etiquetas estan previstas para ocupar una linea entera: 15 caracteres + uno oculto de terminacion de cadena (codigo ASCII 0)
	//      que el compilador me añade automaticamente.
	char cadena[16]; //Una cadena con espacio para 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)

	//La primera instruccion de mi main() siempre ha de ser la siguiente:
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
	
    //Inicializaciones:
    init_ucs_24MHz();		//Ajustes del clock (Unified Clock System)
    init_puertos();			//Configuramos botones y leds
    init_interrupciones(); //Configurar y activar las interrupciones del joystick
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
    Color_Texto_Inv = COLOR_16_ORANGE; //cambiamos el color del texto en modo INVERT_TEXT
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
    //Escribimos un mensage de bienvenida:
    halLcdPrintLine( " Hola Alumnes! ",linea++,INVERT_TEXT);
    halLcdPrintLine( " Benvinguts a  ",linea++,NORMAL_TEXT);
    halLcdPrintLine( "PAE, Practica 1",linea++,NORMAL_TEXT);
    //Dibujamos una linea de separacion:
    halLcdDrawLine(1, 68, 126, 68, COLOR_16_GOLD);
    //Pintamos el logo:
    x0 = (getScreenWidth() - logoUB8BPP_UNCOMP.xSize)>>1; //Coordenadas para pintar el logo centrado
    y0 = (getScreenHeight() - logoUB8BPP_UNCOMP.ySize)-1; //y abajo de la pantalla
    pintaLogo(logoUB8BPP_UNCOMP, x0, y0);

    //Bucle infinito de espera de eventos:
    while(1){
    	if (estado_anterior!= estado)
    	{  // Si el estado ha cambiado, respondemos a los cambios de estado, y escribimos info en LCD.
    		sprintf(cadena,"Estado: %02d",estado); //Nota: la funcion sprintf añadira el caracter oculto de terminacion de cadena
    		halLcdPrintLine( cadena,linea,NORMAL_TEXT);
    		halLcdPrintLine( Jstick[estado],linea+1,NORMAL_TEXT);
     		estado_anterior=estado; // Actualizamos el valor de estado_anterior, para que solo se ejecute cuando se haya
    	                            // apretado un boton. Esta variable se ha de reponer a 0 al inicio de la ISR de los botones.
     		switch(estado){
     		case Jstick_Left:
     			borrar_Logo(logoUB8BPP_UNCOMP, x0, y0);
     			x0 = 0; //Coordenadas para pintar el logo a la izquierda
     			pintaLogo(logoUB8BPP_UNCOMP, x0, y0);
     			break;
     		case Jstick_Right:
     			borrar_Logo(logoUB8BPP_UNCOMP, x0, y0);
     			x0 = (getScreenWidth() - logoUB8BPP_UNCOMP.xSize); //Coordenadas para pintar el logo a la derecha
     			pintaLogo(logoUB8BPP_UNCOMP, x0, y0);
     			break;
     		case Jstick_Center:
     			borrar_Logo(logoUB8BPP_UNCOMP, x0, y0);
     		    x0 = (getScreenWidth() - logoUB8BPP_UNCOMP.xSize)>>1; //Coordenadas para pintar el logo centrado
     			pintaLogo(logoUB8BPP_UNCOMP, x0, y0);
     			break;
     		}
   	}//fin del if (estado)
		RGB++;
		if (RGB>7) RGB = 0;
		EncenderRGB_Launchpad(RGB);
    	 //Una especie de "delay()" por software, para ver parpadear los LEDs:
    	i = 100000;
    	do 	{ //El "delay()" no es mas que un bucle tonto para perder tiempo
    		i--;
    	}
    	while (i != 0);
    }//fin del bucle infinito
}//fin del main()

//Mis rutinas de atencion a las interrupciones (ISR) del joystick:
void PORT4_IRQHandler(void){
	uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
	P4IE &= ~(BIT1 + BIT5 + BIT7);  //Interrupciones desactivadas
	estado_anterior=0;

		switch(flag){
	    case 0x04: //pin 1
	        estado = Jstick_Center; //center
	        break;
	    case 0x0C: //pin 5
	        estado = Jstick_Right; //right
	        break;
	    case 0x10: //pin 7
	        estado = Jstick_Left; //Left
	        break;
		}

		P4IE |= BIT1 + BIT5 + BIT7; 	//Interrupciones reactivadas
}

void PORT5_IRQHandler(void){
	uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
	P5IE &= ~(BIT4 + BIT5);  //Interrupciones desactivadas
	estado_anterior=0;

	if (flag == 0x0A) estado = Jstick_Up; //pin 4
	else if (flag == 0x0C) estado = Jstick_Down; //pin5

	P5IE |= BIT4 + BIT5; 	//Interrupciones reactivadas
}

//Fin del fichero.
