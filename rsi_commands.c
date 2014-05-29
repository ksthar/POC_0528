/*
 * rsi_commands.c
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#include "commonTypes.h"
#include "rsi.h"
#include "rsi_commands.h"
#include "crc.h"

//locals:
static sInt32_t prepareStatusIdle(uInt8_t *rsiResponseFrame);
static sInt32_t prepareApmStatus(uInt8_t *rsiResponseFrame);

sInt32_t processPollRsd(uInt8_t * rsiResponseFrame)
{
    sInt32_t frameLength = STATUS_FAILURE;         		// response frame length
    static rsdStatus_t iRsdStatus = RSD_STATUS_IDLE;  	// FIXME: RSD status flag

	printf("processPollRsd\n");

    if (NULL == rsiResponseFrame)
    	return frameLength;

    // Check RSD status flag and prepare appropriate response frame
    switch (iRsdStatus)
    {
        case RSD_STATUS_IDLE:
            // prepare status idle response frame
        	printf("rsd status idle\n");
            frameLength = prepareStatusIdle(rsiResponseFrame);
            if (STATUS_FAILURE == frameLength)		//check if prepareStausIdle returns failure
            {

            }
            break;

        case RSD_STATUS_CHANGE:
            // prepare status change response frame
        	printf("rsd status changed\n");
            //frameLength = prepareStatusChange(rsiResponseFrame);
            if (STATUS_FAILURE == frameLength)		//check if prepareStausChange returns failure
            {

            }
            break;

        case RSD_STATUS_CARDDATA:
            // prepare status card data response frame
        	printf("rsd status card data\n");
            //frameLength = prepareStatusCardData(rsiResponseFrame);
            if (STATUS_FAILURE == frameLength) //check if prepareStausCardData returns failure
            {

            }
            break;

        default:
            // prepare status idle response frame
        	printf("rsd status unknown\n");
            //frameLength = prepareStatusIdle(rsiResponseFrame);
            if (STATUS_FAILURE == frameLength) //check if prepareStausIdle returns failure
            {

            }
            break;
    } //switch

    return (frameLength);
}

sInt32_t processPollApm(uInt8_t * rsiResponseFrame)
{
	sInt32_t frameLength = STATUS_FAILURE;         		// response frame length

    printf("processPollApm\n");
    if (NULL == rsiResponseFrame)
    	return frameLength;

    // prepare response frame
    frameLength = prepareApmStatus(rsiResponseFrame);
    if (STATUS_FAILURE == frameLength)		//check if prepareStausIdle returns failure
    {

    }

    return (frameLength);
}

sInt32_t processApmTimedUnlock(uInt8_t * rsiRequestFrame, uInt8_t * rsiResponseFrame)
{
	sInt32_t frameLength = STATUS_FAILURE;         		// response frame length
	sInt8_t  secondsToUnlock;


	printf("processApmTimedUnlock\n");
    if (NULL == rsiRequestFrame || NULL == rsiResponseFrame)
    	return frameLength;

    // prepare response frame
    frameLength = prepareApmStatus(rsiResponseFrame);
    if (STATUS_FAILURE == frameLength)		//check if prepareStausIdle returns failure
    {

    }

    //pass info to BLE task... (non-blocking: send a message...)
    secondsToUnlock = rsiRequestFrame[4];
    printf("timed unlock: %d\n", secondsToUnlock);

    return (frameLength);
}

// build response frames:

 static sInt32_t prepareStatusIdle(uInt8_t *rsiResponseFrame)
{
    sInt32_t frameLength = STATUS_FAILURE;         // response frame length
    uInt8_t crcVal[CRC_SIZE];                      // variable for CRC Value

    // Checking if input rsiResponseFrame pointer is invalid
    if (NULL == rsiResponseFrame)
    	return frameLength;

    // Standard RSD_STATUS_IDLE Response
    rsiResponseFrame[0] = RSI_SOF;           // SOF
    rsiResponseFrame[1] = ACP_ADDR;       	 // ACP address
    rsiResponseFrame[2] = RSD_STATUS;    	 // response type
    rsiResponseFrame[3] = 0;  				 // length

    // Calculate response frame CRC
    calculateCrc( &rsiResponseFrame[1], 3, crcVal );

    // Add CRC bytes in the response frame
    rsiResponseFrame[4] = crcVal[CRC_LSB_INDEX];
    rsiResponseFrame[5] = crcVal[CRC_MSB_INDEX];
    frameLength = RSI_HEADER_LENGTH + CRC_SIZE;

    return frameLength;
} // end prepareStatusIdle()

static sInt32_t prepareApmStatus(uInt8_t *rsiResponseFrame)
{
    sInt32_t frameLength = STATUS_FAILURE;         // response frame length
    uInt8_t crcVal[CRC_SIZE];                      // variable for CRC Value

    // Checking if input rsiResponseFrame pointer is invalid
    if (NULL == rsiResponseFrame)
    	return frameLength;

    // Standard RSD_STATUS_IDLE Response
    rsiResponseFrame[0] = RSI_SOF;           // SOF
    rsiResponseFrame[1] = ACP_ADDR;       	 // ACP address
    rsiResponseFrame[2] = APM_STATUS;    	 // response type
    rsiResponseFrame[3] = 3;

    rsiResponseFrame[4] = 0;
    rsiResponseFrame[5] = 0;
    rsiResponseFrame[6] = 0x80;	//bit 7 = lock status (unlocked)

    // Calculate response frame CRC
    calculateCrc( &rsiResponseFrame[1], 6, crcVal );

    // Add CRC bytes in the response frame
    rsiResponseFrame[7] = crcVal[CRC_LSB_INDEX];
    rsiResponseFrame[8] = crcVal[CRC_MSB_INDEX];
    frameLength = RSI_HEADER_LENGTH + 3 + CRC_SIZE;

    return frameLength;
} // end prepareApmStatus()


