/*
 * valores.h
 *
 *  Created on: 12 mar. 2018
 *      Author: Equipo
 */

#ifndef HELPER_H_
#define HELPER_H_

// Colores para el LED RGB
#define RED 0x40
#define GREEN 0x10
#define BLUE 0x40

#define BUTTON_S1 1
#define BUTTON_S2 2
#define JOYSTICK_LEFT 3
#define JOYSTICK_RIGHT 4
#define JOYSTICK_UP 5
#define JOYSTICK_DOWN 6
#define JOYSTICK_CENTER 7

void increase_time(int type, int *sec);
void decrease_time(int type, int *sec);

#endif /* HELPER_H_ */
