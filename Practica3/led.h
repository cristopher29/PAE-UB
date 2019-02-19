/*
 * leds.h
 *
 *  Created on: 12 mar. 2018
 *      Author: Equipo
 */

#ifndef LED_H_
#define LED_H_

void init_leds_p7(void);
void init_led_rgb(void);
void led_rgb_set(uint8_t r, uint8_t g, uint8_t b);
void led_rgb_toggle(void);

#endif /* LED_H_ */
