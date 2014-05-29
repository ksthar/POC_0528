/*
 * led.c
 *
 *  Created on: May 23, 2014
 *      Author: kbroerman
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "led.h"

char *ledName[] = { RED, GREEN, BLUE };

//tri-color LED control
int led_setValue(char *led, int value)
{
	char buf[60];
	int len = sprintf(buf, "/sys/class/leds/%s", led);
	strcat( &buf[len], "/brightness" );
	int fd = open(buf, O_WRONLY);
	if (fd >= 0) {
		if (value < 0)   value = 0;
		if (value > 255) value = 255;

		len = sprintf(buf, "%d", value);
		write(fd, buf, len);
		close(fd);
	}
	return 0;
}
