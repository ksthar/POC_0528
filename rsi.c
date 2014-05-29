/*
 * rsi.c
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>
#include <errno.h>

#include "commonTypes.h"
#include "rsi.h"
#include "rsi_commands.h"
#include "serial.h"
#include "crc.h"
#include "misc.h"
#include "led.h"
#include "gpio.h"

#define RSI_DEBUG

int rsiOpen(void)
{
	crcInit();
	return rs485_open();
}

int rsiClose(int fd)
{
	rs485_close(fd);
}

void dbg_print_msg( int write, char *buf, int len )
{
#ifdef RSI_DEBUG
	int i;
	if (write)
	   printf("RS485 msg write: ");
	else
	   printf("RS485 msg read: ");

	for (i=0; i < len; i++)
		printf("%02x ", buf[i]);
	printf("\n");
#endif
}

//inputs:
//		fd:     	uart file descriptor
//		buffer: 	transmit buffer
//		numBytes:	# bytes to send
//
//returns bytes received. returns only if valid frame received.
int rsiWrite( int fd, char *buffer, int numBytes )
{
	if (fd == -1) return -1;
	rs485_xmitEnable();		//half duplex mode, enable Tx
	rs485_write( fd, buffer, numBytes );
	tcdrain(fd);	//FIXME: force the write to complete - do we need it?

	dbg_print_msg( 1, buffer, numBytes );

	//FIXME! at 9600 baud, need to ensure Tx is done before disabling transceiver!
	delayuS(100000);	//9600 baud * 64 bytes max = 66 mS ==> wait 100 mSec
	rs485_xmitDisable();
}

//inputs:
//		fd:     uart file descriptor
//		buffer: receive buffer
//
//returns bytes received. returns only if valid frame received.
int rsiRead( int fd, char *buffer )
{
	if (fd == -1) return -1;
	int count, validRsiFrame = STATUS_FAILURE;

	tcflush(fd, TCIFLUSH);	//in half duplex mode, ignore chars we sent and may have received...
	rs485_rcvEnable();

	while (validRsiFrame != STATUS_SUCCESS) {

		//rs485UartVminVtime( fd, 0, 0 );  //no block
		buffer[0] = 0;

		// wait for RSI SOF...
		do
		{
			count = rs485_read(fd, buffer, 1);
			if (count < 0) puts("-");
			delayuS(10);	//don't burn up all the CPU if nonblocking read enabled

			if (count == -1 && errno != EAGAIN)	//FIXME needs work
				puts("unhandled read error!\n");

		} while (buffer[0] != RSI_SOF);

		rs485UartVminVtime( fd, RSI_MAX_FRAME_SIZE, 1 );	//wait for max size frame (or) 0.1 sec timeout

		//now capture remainder of packet. length is not fully known!
		//  if bit 7 of Type = 1: indicates 2 length bytes
		//  Length bytes indicates # data bytes
		//  tricky: FCS can be one or 2 bytes, not always indicated by cmd Type!!

		//BETTER?: if we can rely on intercharacter timeout (0.1 sec) then
		//  ask for max cmd size & wait for timeout...

		//KERNEL BUG?  current timeout appears to be about 2 seconds (?)
		count = rs485_read(fd, &buffer[1], RSI_MAX_FRAME_SIZE-1); //should return after 0.1 sec
		printf("rd = %d\n", count+1);

		//RS485 Bytes received
		dbg_print_msg( 0, buffer, count+1 );

		//validate message CRC/checksum
		validRsiFrame = rsiValidateFrame( &buffer[1], count);
	} //waiting for validRsiFrame

	//gpio_setValue( RS485_LED_RED, 0 );
	rs485_rcvDisable();
	return (count+1);
}

//inputs:
//	buffer:  received RSI frame, starting with address field
//	msgLength: RSI message length, not including SOF.  Includes CRC/checksum byte(s)
sInt32_t rsiValidateFrame( char *buffer, int msgLength )
{

	uInt8_t crcVal[CRC_SIZE];              // CRC value
	sInt32_t retVal = STATUS_FAILURE;      // Variable for Return value

	//assume frame has CRC...
	calculateCrc( buffer, msgLength-2, crcVal);
	//uInt32_t crc = (crcVal[1] << 8) | crcVal[0];
	//printf("expected crc = 0x%04x\n", crc);

	if ((crcVal[CRC_LSB_INDEX] == buffer[msgLength - 2]) &&
	    (crcVal[CRC_MSB_INDEX] == buffer[msgLength - 1])) {

		//printf("validate CRC: OK!\n");
		retVal = STATUS_SUCCESS;
	}
	else {	//frame must have 1-byte checksum...
		uInt8_t checkSum = calculateChecksum( buffer, msgLength-1 );
		if (checkSum == buffer[msgLength - 1]) {
			//printf("validate checksum: OK!\n");
			retVal = STATUS_SUCCESS;
		}
		else
			printf("invalid RSI frame check sequence!\n");
	}

	//FIXME: validate RSD/ACP frame address here?

	return retVal;
}

//on success, sets the rsiResponseFrame framelength:
sInt32_t rsiHandler(uInt8_t *rsiRequestFrame, uInt8_t *rsiResponseFrame,
        sInt32_t * frameLength)
{
    sInt32_t retVal = STATUS_FAILURE;        // Variable for Return value
    uInt8_t cCommandType = ZERO; 			 // Variable for Command type

    // Checking if rsiResponseFrame and rsiRequestFrame pointers are INVALID
    if ((NULL == rsiRequestFrame) || (NULL == rsiResponseFrame))
    	return retVal;

    cCommandType = rsiRequestFrame[2];

    // check for command type
    switch (cCommandType) {

        case POLL_RSD_CRC:
        case POLL_RSD_CHECKSUM:
            retVal = processPollRsd(rsiResponseFrame);
            if (retVal < ZERO)
            {
                retVal = STATUS_FAILURE;
            }
            break;

        case POLL_APM_CRC:
        case POLL_APM_CHECKSUM:
            retVal = processPollApm(rsiResponseFrame);
            if (retVal < ZERO)
            {

                retVal = STATUS_FAILURE;
            }
            break;

        case APM_TIMED_UNLOCK:
            retVal = processApmTimedUnlock(rsiRequestFrame, rsiResponseFrame);
            if (retVal < ZERO)
            {

                retVal = STATUS_FAILURE;
            }
            break;


        default:
        	printf("FIXME! unsupported RSI command...");
            retVal = STATUS_FAILURE;
            break;

    } // End switch

    if (STATUS_FAILURE < retVal)
    {
        // Assign frame length
        *frameLength = retVal;
        retVal = STATUS_SUCCESS;
    }

    return retVal;
} // End rsiHandler()






