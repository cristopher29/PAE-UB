#include <msp432p401r.h>
#include <stdint.h>
#include <stdio.h>
#include <helper.h>

uint32_t current_time = 0; // Variable que se usa en el Timer0
byte received_data;
byte read_data_UART;

// Función time_out es True si el tiempo del contador (current_time) supera el tiempo que hemos pasado como parámetro (time)
bool time_out(int time){
    return (current_time>=time);
}


// Sentido: Recibir datos
void sentit_dades_Rx(void)
{ //Configuració del Half Duplex dels motors: Recepció
    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 (Rx)
}

// Sentido: Enviar datos
void sentit_dades_Tx(void)
{ //Configuració del Half Duplex dels motors: Transmissió
    P3OUT |= BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)
}

void init_UART(void)
{
    UCA2CTLW0 |= UCSWRST;           //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK;     //UCSYNC=0 mode asíncron
                                    //UCMODEx=0 seleccionem mode UART
                                    //UCSPB=0 nomes 1 stop bit
                                    //UC7BIT=0 8 bits de dades
                                    //UCMSB=0 bit de menys pes primer
                                    //UCPAR=x ja que no es fa servir bit de paritat
                                    //UCPEN=0 sense bit de paritat
                                    //Triem SMCLK (16MHz) com a font del clock BRCLK

    UCA2MCTLW = UCOS16;             // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3;                    //Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                                    //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
                                    //el rellotge de la UART ha de ser de 8MHz (24MHz/3).

    //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3; //I/O funció: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3);

    //Configurem pin de selecció del sentit de les dades Transmissió/Recepeció
    P3SEL0 &= ~BIT0; //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0; //Port P3.0 com sortida (Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0; //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la línia de comunicacions sèrie
    UCA2IE |= UCRXIE; //Això només s’ha d’activar quan tinguem la rutina de recepció
}


// Enviar un byte por la UART
void TxUAC2(byte bTxdData)
{
    while(!TXD2_READY); // Espera a que estigui preparat el buffer de transmissió
    UCA2TXBUF = bTxdData;
}

//TxPacket() 3 paràmetres: ID del Dynamixel, Mida dels paràmetres, Instruction byte. torna la mida del "Return packet"
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte bParameters[16])
{
    byte bCount,bCheckSum,bPacketLength;
    byte TxBuffer[32];

    if(bParameters[0] <= 5){ // Comprovació per seguretat
        return 0;
    }

    sentit_dades_Tx();  //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)

    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;

    TxBuffer[2] = bID; //ID del mòdul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength+2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction; //Instrucció que enviem al Mòdul

    for(bCount = 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d’enviar
    {
        TxBuffer[bCount+5] = bParameters[bCount];
    }

    bCheckSum = 0;
    bPacketLength = bParameterLength+4+2;

    for(bCount = 2; bCount < bPacketLength-1; bCount++) //Càlcul del checksum
    {
        bCheckSum += TxBuffer[bCount];
    }

    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)

    for(bCount = 0; bCount < bPacketLength; bCount++) //Aquest bucle és el que envia la trama al Mòdul Robot
    {
        TxUAC2(TxBuffer[bCount]);
    }

    while( (UCA2STATW&UCBUSY)); //Espera fins que s’ha transmès el últim byte

    sentit_dades_Rx(); //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta

    return bPacketLength;
}


struct RxReturn RxPacket(void)
{
    struct RxReturn respuesta;
    byte bCount, bLenght, bCheckSum = 0;
    bool Rx_time_out = false;

    sentit_dades_Rx(); // Ponemos la línea half duplex en Rx

    //Activa_Interrupcion_TimerA1();

    // Leemos los 4 primeros parámetros
    for(bCount = 0; bCount < 4; bCount++)
    {
        current_time=0;

        received_data=0; // No

        while (!received_data) // No se ha recibido el dato?
        {
            Rx_time_out = time_out(20);  // Tiempo de espera en milisegundos
            if (Rx_time_out)break;      // Si se ha acabado el tiempo de espera para recibir un dato salimos.
        }

        if (Rx_time_out)break; // Salimos del for si ha habido un Timeout

        // Todo ha ido bien, leemos un dato
        respuesta.StatusPacket[bCount] = read_data_UART;

    }


    if (!Rx_time_out){  // Si no hay Timeout

        bLenght = respuesta.StatusPacket[3]+4; // bLenght total de lo que vamos a leer

        // Leemos el resto de datos
        for(bCount = 4; bCount < bLenght; bCount++)
        {
            current_time=0;

            received_data=0; // No

            while (!received_data) // No se ha recibido el dato?
            {
                Rx_time_out = time_out(20);  // Tiempo de espera en milisegundos
                if (Rx_time_out)break;      // Si se ha acabado el tiempo de espera para recibir un dato salimos.
            }

            if (Rx_time_out)break; // Salimos del for si ha habido un Timeout

            //Si no, es que todo ha ido bien, y leemos un dato:
            respuesta.StatusPacket[bCount] = read_data_UART;
        }
    }

    if(!Rx_time_out){   // Si no hay Timeout

        for(bCount = 2; bCount < bLenght-1; bCount++) //Cálculo del checksum
        {
            bCheckSum += respuesta.StatusPacket[bCount];
        }

        bCheckSum = ~bCheckSum; // Checksum a complemento a 1
        // Comparamos el checksum recibido con el calculado
        if(respuesta.StatusPacket[bLenght-1] != bCheckSum){
            respuesta.error = true;
        }

    }

    return respuesta;
}

// Interrupción al recibir un dato
void EUSCIA2_IRQHandler(void){

    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX

    read_data_UART = UCA2RXBUF; // Dato leído
    received_data = 1; // Ha llegado

    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX
}

// Interrupción para el TIMER DE 1 MS
void TA1_0_IRQHandler(void) {

    TA1CCTL0 &= ~CCIE;      // Deshabilitamos interrupciones mientras tratamos esta

    current_time++;         // Contador timeout

    TA1CCTL0 &= ~CCIFG;     // Quitamos flag de interrupción
    TA1CCTL0 |= CCIE;       // Habilitamos interrupciones
}
