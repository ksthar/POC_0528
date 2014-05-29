/*
 * gpio.c
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "gpio.h"

int gpio_init(int gpio)
{
	//export gpio so userspace can tweek it:
	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd >= 0) {
		char buf[10];
		sprintf(buf, "%d", gpio);
		//printf("gpio_init : buf = %s\n", buf);
		write(fd, buf, strlen(buf));
		close(fd);
	}
	return fd;
}

int gpio_deinit(int gpio)
{
	//unexport gpio:
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		char buf[10];
		sprintf(buf, "%d", gpio);
		//printf("gpio_init : buf = %s\n", buf);
		write(fd, buf, strlen(buf));
		close(fd);
	}
	return fd;
}

int gpio_setValue(int gpio, int value)
{
	char buf[60];
	int len = sprintf(buf, "/sys/class/gpio/gpio%d", gpio);
	strcat( &buf[len], "/direction" );
	int fd = open(buf, O_WRONLY);
	if (fd >= 0) {
		if (value)
			write(fd, "high", 4);
		else
			write(fd, "low",  3);
		close(fd);
	}
	return 0;
}

