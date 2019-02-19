#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <helper.h>
#include <uart.h>
#include <lib_PAE2.h>

// Funcion para que las ruedas puedan girar continuamente
void wheelMode(void){

    byte bID = BROADCASTING; // No obtenemos respuesta en este modo
    byte bInstruction = WRITE_DATA;
    byte bParameterLength = 5;
    byte bParameters[16];

    // Empezamos por la dirección 0x06 (6)
    bParameters[0] = CW_ANGLE_LIMIT_L;

    // Datos a escribir
    bParameters[1] = 0;  // CW_ANGLE_LIMIT_L = 0
    bParameters[2] = 0;  // CW_ANGLE_LIMIT_H = 0
    bParameters[3] = 0;  // CCW_ANGLE_LIMIT_L = 0
    bParameters[4] = 0;  // CCW_ANGLE_LIMIT_H = 0

    // Enviamos los datos
    TxPacket(bID, bParameterLength, bInstruction, bParameters);
}



void moveWheel(byte ID, bool rotation, unsigned int speed)
{
    struct RxReturn returnPacket;
    byte speed_H,speed_L;
    speed_L = speed;

    if(speed<1024){ // Velocidad max. 1023

        if(rotation){ // Rotation == 1
            speed_H = (speed >> 8)+4;   // Mover a la derecha (CW)
        }else{
            speed_H = speed >> 8;       // Mover a la izquierda (CCW)
        }

        byte bInstruction = WRITE_DATA;
        byte bParameterLength = 3;
        byte bParameters[16];

        // Empezamos por la dirección 0x20 (32)
        bParameters[0] = MOV_SPEED_L;

        // Escribimos la velocidad y la dirección
        bParameters[1] = speed_L;
        bParameters[2] = speed_H;

        TxPacket(ID, bParameterLength, bInstruction, bParameters);
        returnPacket = RxPacket();

    }
}

void stop(void)
{
    moveWheel(RIGHT_WHEEL, 0, 0);
    moveWheel(LEFT_WHEEL, 0, 0);
}

void turnLeft(unsigned int speed){
    // Girar a la izquierda - Mover a la derecha todas las ruedas
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, RIGHT, speed);
        moveWheel(LEFT_WHEEL, RIGHT, 0);
    }
}

void turnLeftD(unsigned int degree){
    // Girar a la izquierda - Mover a la derecha todas las ruedas
    moveWheel(RIGHT_WHEEL, RIGHT, 300);
    moveWheel(LEFT_WHEEL, RIGHT, 0);
    delay_t(degree*28.5);
    stop();
}

void turnOnItselfLeft(unsigned int speed)
{
    // Girar a la izquierda - Mover a la derecha todas las ruedas
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, RIGHT, speed);
        moveWheel(LEFT_WHEEL, RIGHT, speed);
    }
}

void turnRight(unsigned int speed)
{
   // Girar a la derecha - Mover a la izquierda todas las ruedas
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, LEFT, 0);
        moveWheel(LEFT_WHEEL, LEFT, speed);
    }
}

void turnRightD(unsigned int degree)
{
    // Girar a la izquierda - Mover a la derecha todas las ruedas
    moveWheel(RIGHT_WHEEL, LEFT, 0);
    moveWheel(LEFT_WHEEL, LEFT, 300);
    delay_t(degree*28.5);
    stop();
}

void turnOnItselfRight(unsigned int speed)
{
   // Girar a la derecha - Mover a la izquierda todas las ruedas
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, LEFT, speed);
        moveWheel(LEFT_WHEEL, LEFT, speed);
    }
}

void forward(unsigned int speed)
{
    // Mover hacia delante
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, RIGHT, speed);
        moveWheel(LEFT_WHEEL, LEFT, speed);
    }
}

void backward(unsigned int speed)
{
    // Mover hacia atrás
    if(speed < 1024){
        moveWheel(RIGHT_WHEEL, LEFT, speed);
        moveWheel(LEFT_WHEEL, RIGHT, speed);
    }
}

void motorLed(byte ID, bool status)
{

    struct RxReturn returnPacket;
    byte bInstruction = WRITE_DATA;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = M_LED;
    bParameters[1] = status;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);
    returnPacket = RxPacket();

}

int readSensor(byte ID, byte sensor)
{

    struct RxReturn returnPacket;

    byte bInstruction = READ_DATA;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = sensor;
    bParameters[1] = 1;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);
    returnPacket = RxPacket();

    return returnPacket.StatusPacket[5];
}

void setCompareDistance(byte ID,unsigned int dist)
{

    struct RxReturn returnPacket;

    byte bInstruction = WRITE_DATA;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = 0x14; //0x34
    bParameters[1] = dist;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);
    returnPacket = RxPacket();

}

int getObstacleDetected(byte ID)
{

    struct RxReturn returnPacket;

    byte bInstruction = READ_DATA;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = 0x20;
    bParameters[1] = 1;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);

    returnPacket = RxPacket();

    return returnPacket.StatusPacket[5];
}

int readMaxDist(byte ID, byte position)
{

    struct RxReturn returnPacket;

    byte bInstruction = READ_DATA;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = position;
    bParameters[1] = 1;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);

    returnPacket = RxPacket();

    return returnPacket.StatusPacket[5];
}
