/*
 * gpio.h
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#ifndef GPIO_H_
#define GPIO_H_

#define GPIO_41			(41)
#define GPIO_42			(42)
#define RS485_RE		(GPIO_41)
#define RS485_DE		(GPIO_42)

#define GPIO_44			(44)
#define GPIO_45			(45)
#define RS485_LED_GREEN	(GPIO_44)		//Tx
#define RS485_LED_RED	(GPIO_45)		//Rx

int gpio_init(int gpio);
int gpio_deinit(int gpio);
int gpio_setValue(int gpio, int value);


#endif /* GPIO_H_ */
