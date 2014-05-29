/*
 * serial.h
 *
 *  Created on: May 21, 2014
 *      Author: kbroerman
 */

#ifndef SERIAL_H_
#define SERIAL_H_

int rs485_open(void);
int rs485_close(int fd);

int rs485_rcvEnable(void);
int rs485_rcvDisable(void);
int rs485_xmitEnable(void);
int rs485_xmitDisable(void);

int rs485_read( int fd, char *buffer, int requested );
int rs485_write( int fd, char *buffer, int requested );

void rs485TimedRead( int fd, int enabled );
void rs485UartVminVtime( int fd, int vmin, int vtime );

#endif /* SERIAL_H_ */
