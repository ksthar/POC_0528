/*
 * serial.c  - rename! --> this is RS485 specific uart layer,
 * 						     it knows nothing about the RSI protocol...
 *
 *  Created on: May 21, 2014
 *      Author: kbroerman
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>

#include "commonTypes.h"
#include "serial.h"
#include "gpio.h"
#include "misc.h"

int rs485_rcvEnable(void)
{
	//RS485 RE/ (APx4 receive)
	//printf("485 rcv en\n");
	gpio_setValue(RS485_RE, 0);	//Rx enabled
	delayuS(25);	//let it settle...
}

int rs485_rcvDisable(void)
{
	//RS485 RE/ (APx4 receive)
	gpio_setValue(RS485_RE, 1);	//Rx disabled
	delayuS(25);	//let it settle...
}

int rs485_xmitEnable(void)
{
	//RS485 DE (APx4 transmit)
	gpio_setValue(RS485_DE, 1);	//Tx enabled
	delayuS(25);	//let it settle...
}

int rs485_xmitDisable(void)
{
	//RS485 DE (APx4 transmit)
	gpio_setValue(RS485_DE, 0);	//Tx disabled
	delayuS(25);	//let it settle...
}

//inputs:
//		fd:     uart file descriptor
//		count:	number of bytes to wait for
//		buffer: receive buffer
//
//returns bytes received
int rs485_read( int fd, char *buffer, int requested )
{
	if (fd == -1) return -1;
	int actual;

	gpio_setValue(RS485_LED_RED, 0);		//FIXME: replace with LED thread to blink LEDs

	//must do this at higher layer:
	//RS485 RE/ (APx4 receive)
	//gpio_setValue(RS485_RE, 0);	//Rx enabled

	actual = read(fd, buffer, requested);	//blocking read
	gpio_setValue(RS485_LED_RED, 1);
	return actual;
}

//inputs:
//		fd:     uart file descriptor
//		count:	number of bytes to write
//		buffer: transmit buffer
//
//returns bytes sent
int rs485_write( int fd, char *buffer, int requested )
{
	if (fd == -1) return -1;
	int actual;

	gpio_setValue(RS485_LED_GREEN, 1);		//FIXME: replace with LED thread to blink LEDs

	//printf("writing %d bytes\n", requested);

	//now assume this is done at higher layer:
	//RS485 DE (APx4 transmit)
	//gpio_setValue(RS485_DE, 1);	//Tx enabled
	//delayuS(25);	//let it settle...

	actual = write(fd, buffer, requested);
	gpio_setValue(RS485_LED_GREEN, 0);		//LED off
	return actual;
}

int rs485_open(void)
{
	//export gpios 41 (RS485 RE/) & 42 (RS485 DE) so userspace can control RS485 transceiver output enables:
	gpio_init(RS485_RE);
	gpio_init(RS485_DE);
	rs485_rcvDisable();
	rs485_xmitDisable();

	//export gpios 44 & 45 (CommLEDGreen = Tx, CommLEDRed = Rx)
	gpio_init(RS485_LED_GREEN);
	gpio_init(RS485_LED_RED);
	gpio_setValue(RS485_LED_GREEN, 0);	//off
	gpio_setValue(RS485_LED_RED, 0);	//off

	//-------------------------
	//----- SETUP AUART 2 -----
	//-------------------------
	//before running, assumes GPIO pins 41 and 42 are already set to proper values
	int uart2_fd = -1;
	int count;

	//OPEN THE UARTint rs485_open(void)

	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart2_fd = open("/dev/ttyAPP2", O_RDWR | O_NOCTTY ); // | O_NDELAY);	//Open in non blocking read/write mode
	if (uart2_fd == -1)
	{
		//ERROR - CAN'T OPEN AUART 2
		printf("Error - Unable to open AUART 2!\n");
		return uart2_fd;
	}

	printf("AUART 2 opened...\n");

	//CONFIGURE THE UART
			//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	//memset(&options, 0, sizeof(options));
	tcgetattr(uart2_fd, &options);
	cfmakeraw(&options);
	options.c_cc[VMIN]  = 1;	//if blocking read: wait for at least 1 char (SOF)
	options.c_cc[VTIME] = 0;	//read: 100 mS interchar timeout
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

#if 0
	//printf("options.c_cc[VMIN] = %d\n", options.c_cc[VMIN]);
	//printf("options.c_cc[VTIME] = %d\n", options.c_cc[VTIME]);

	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag &= ~(ICANON);	//raw mode

	options.c_cc[VMIN]  = 0;	//if blocking read: wait for at least 1 char (SOF)
	options.c_cc[VTIME] = 0;	//read: 100 mS interchar timeout
#endif


	tcsetattr(uart2_fd, TCSANOW, &options);
	tcflush(uart2_fd, TCIOFLUSH);

#if 0
	struct serial_struct serial;
	int ret = ioctl(uart2_fd, TIOCGSERIAL, &serial);

	printf("fifo size = %d\n", serial.xmit_fifo_size);

	if (ret != 0) puts("ooops1");
	serial.flags | ASYNC_LOW_LATENCY;
	ret = ioctl(uart2_fd, TIOCSSERIAL, &serial);
	if (ret != 0) puts("ooops2");
#endif

	return uart2_fd;
}

int rs485_close(int fd)
{
	close(fd);
	gpio_deinit(RS485_RE);
	gpio_deinit(RS485_DE);
	gpio_deinit(RS485_LED_GREEN);
	gpio_deinit(RS485_LED_RED);

}

void rs485UartVminVtime( int fd, int vmin, int vtime )
{
	struct termios options;

	tcgetattr(fd, &options);

	options.c_cc[VMIN]  = vmin;	//change to timed read
	options.c_cc[VTIME] = vtime;	//read: 100 mS

	tcsetattr(fd, TCSANOW, &options);
	//delayuS(25);	//FIXMWE remove
}




