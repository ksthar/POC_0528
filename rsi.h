/*
 * rsi.h
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#ifndef RSI_H_
#define RSI_H_

#include "commonTypes.h"

#define RSI_MAX_FRAME_SIZE			(64)
#define RSI_HEADER_LENGTH			(4)
#define RSI_SOF						(0x0A)
#define	ACP_ADDR					(0xFF)

//ACP request codes:
#define POLL_APM_CRC                (0x44)
#define POLL_APM_CHECKSUM           (0x3B)
#define POLL_RSD_CRC                (0x3A)
#define POLL_RSD_CHECKSUM           (0x74)
#define APM_TIMED_UNLOCK           	(0x56)

//Gateway response codes:
#define APM_STATUS		          	(0x30)
#define RSD_STATUS		          	(0x31)
#define RSD_STATUS_EXTENDED			(0x34)

//#define GET_OEM_CODE                (0x6F)
//#define GET_RSD_INFORMATION         (0x3C)
//#define GET_FIRMWARE_VERSION        (0x2C)
//#define SET_RSD_CONFIGURATION       (0x77)

typedef enum rsdStatus {
	RSD_STATUS_IDLE,
	RSD_STATUS_CHANGE,
	RSD_STATUS_CARDDATA,
} rsdStatus_t;

int rsiOpen(void);
int rsiClose(int fd);
int rsiRead( int fd, char *buffer );
int rsiWrite( int fd, char *buffer, int numBytes );
int rsiValidateMsg( char *buffer, int msgLength );
sInt32_t rsiHandler(uInt8_t *rsiRequest, uInt8_t *rsiResponseFrame,
        sInt32_t * frameLength);


#endif /* RSI_H_ */
