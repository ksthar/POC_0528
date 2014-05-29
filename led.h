/*
 * led.h
 *
 *  Created on: May 23, 2014
 *      Author: kbroerman
 */

#ifndef LED_H_
#define LED_H_

#define RED		("red:led")
#define GREEN	("green:led")
#define BLUE	("blue:led")

int led_setValue(char *led, int value);

#endif /* LED_H_ */
