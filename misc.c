/*
 * misc.c
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#include <time.h>
#include "commonTypes.h"
#include <fcntl.h>


void delayuS(uInt32_t uSec)
{
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = uSec * 1000;
	nanosleep(&ts, NULL);
}

int uptimeInSec( char *timeString)
{
	int num = 0, fd = open("/proc/uptime", O_RDONLY );
	if (fd > 0) {
		num = read( fd, timeString, 30 );
		close(fd);
	}
	return num;
}
