/*------------------------------------------------------------------------------
 * Copyright(c)2014 Allegion PLC as an unpublished work.
 *
 * The information contained herein is confidential property of Allegion PLC.
 * All rights reserved.  Reproduction, adaptation, or translation without the
 * express written consent of Allegion PLC is prohibited, except as allowed
 * under the copyright laws.
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * File Name	  : crc.c
 * Purpose of File: Contains function implementation for CRC and Checksum
 *                  calculation for RSI protocol. These functions performs
 *                  Initialization of CRC table, Validation of CRC for given
 *                  RSI Frame, Checksum calculation for given data and CRC
 *                  calculation for given data is obtained from
 *                  AD-series RSI Serial RS-485 Protocol Specifications ver-4.0
 *----------------------------------------------------------------------------*/

/*-------------- HEADER INCLUDE FILES ----------------------------------------*/
#include "crc.h"
//#include "rsiModuleCommon.h"

/*-------------- FUNCTIONS DEFINITIONS ---------------------------------------*/

/******************************************************************************
 * Description:  crcInit()              - Initializes the CRC module with CRC
 *                                        table.
 *
 * Arguments                            - none
 * Return                               - none
 * Notes/Preconditions                  - none
 ******************************************************************************/
void crcInit(void)
{
    // Call buildXmodem only once. No need to do this at all if the array is
    // statically initialized.
    buildXmodem();
#ifdef CRC_DEBUG
    printTable();	//prints the CRC table
#endif
}   // End of crcInit()

/******************************************************************************
 * Description:  calculateChecksum()    - Calculates checksum of received RSI
 *                                        frame
 *
 * Arguments: uInt8_t * rsiFrame   		- Pointer to RSI frame not including SOF
 *            sInt32_t  frameLength     - Length of frame not including SOF
 * Return:    uInt8_t checkSum                      : on Failure
 *                                      - calculated Checksum value - on Success
 * Notes/Preconditions                  - This function should be called only
 *                                        after receiving RSI frame
 ******************************************************************************/
uInt8_t calculateChecksum(uInt8_t * rsiFrame, sInt32_t frameLength)
{
    sInt32_t i;
    uInt8_t checkSum = ZERO; // variable for checksum

    if (rsiFrame != NULL) {	// If no RSI frame is received then return with checksum as zero

        //calculate the checksum
        for (i = ZERO; i < frameLength; i++)
            checkSum = (checkSum + rsiFrame[i]);

        //logic for checksum calculation
        checkSum = (~checkSum + ONE);
    }
    return (checkSum);
}   // End of calculateChecksum()

/******************************************************************************
 * Description:  calculateCrc()    - Calculates CRC of received RSI Frame.
 *
 * Arguments: uInt8_t * rsiFrame   - Pointer to RSI frame not including SOF
 *            sInt32_t frameLength - Length of RSI frame not including SOF
 *            uInt8_t * crcVal     - Pointer to CRC value result.
 * Return:    sInt32_t retVal      - STATUS_SUCCESS  : CRC calculated.
 *                                   STATUS_FAILURE  : On Failure
 *                                 -
 * Notes/Preconditions             - This function should be called only
 *                                   after receiving RSI frame
 ******************************************************************************/
sInt32_t calculateCrc(uInt8_t * rsiFrame, sInt32_t frameLength, uInt8_t *crcVal)
{
    uInt16_t tempCrc = ZERO;   // Temporary variable for CRC
    sInt32_t retVal = STATUS_FAILURE;     // General return status values

    // Checking if input is invalid
    if (rsiFrame != NULL)
    {
        // if RSI frames are received, then calculate the CRC value
        tempCrc = ZERO; // It is important to initialize the CRC.

        //Call calculateCrcBuf to get the CRC value of the buffer received
        tempCrc = calculateCrcBuf(rsiFrame, frameLength, tempCrc);
        crcVal[ZERO] = (tempCrc & LSB_MASK);
        crcVal[ONE] = (tempCrc & MSB_MASK) >> BYTE_SIZE;

        retVal = STATUS_SUCCESS;
    }

    return (retVal);
}   // End of calculateCrc()

/******************************************************************************
 * Description:  printTable()     - Print the global table of CRC if CRC_DEBUG
 *                                  is defined for debugging
 *
 * Arguments                      - none
 * Return                         - none
 * Notes/Preconditions            - Global CRC table is initialized.
 *                                - CRC_DEBUG macro defined
 ******************************************************************************/
void printTable(void)
{
#ifdef CRC_DEBUG
    sInt32_t i = ZERO;   // index variable
    printf("uInt16_t gTable[CRC_TBL_SIZE] =");
    // print Complete CRC Table if required for debugging
    for (i = ZERO; i < CRC_TBL_SIZE; i++)
    {
        if (ZERO == (i % BYTE_SIZE))
            putchar('\t');
        printf("0x%04X, ", gTable[i]);
        if (ZERO == ((i + ONE) % BYTE_SIZE))
            putchar('\n');
    }
    printf("};\n\n");
#endif
}   // End of printTable()

/******************************************************************************
 * Description:  buildXmodem()          - Initialize the CRC table at run-time
 *
 * Arguments                            - none
 * Return                               - none
 * Notes/Preconditions                  - none
 ******************************************************************************/
void buildXmodem(void)
{
    sInt32_t i = ZERO;
    uInt8_t tempVar;

    // Creating CRC values for Global CRC table
    for (i = ZERO; i < CRC_TBL_SIZE; i++)
    {
        tempVar = i ^ (i >> 4);
        gTable[i] = tempVar ^ ((uInt16_t) tempVar << 5)
                ^ ((uInt16_t) tempVar << 12);
    }
}   // End of buildXmodem()

/******************************************************************************
 * Description:  xmodemCrc()      - Update the CRC value with a new byte
 *
 * Arguments: uInt8_t byteVal     - Value accounted as byte
 *            uInt16_t tempCrc    - Temporary CRC value.
 * Return: uInt16_t crcResult       - Calculated value of tempCrc
 * Notes/Preconditions            - none
 ******************************************************************************/
uInt16_t xmodemCrc(uInt8_t byteVal, uInt16_t tempCrc)
{
    uInt16_t crcResult = ZERO;
    crcResult = (tempCrc << BYTE_SIZE)
            ^ gTable[byteVal ^ (tempCrc >> BYTE_SIZE)];

    return (crcResult);
}   // End of xmodemCrc()

/******************************************************************************
 * Description:  calculateCrcBuf()- Calculate the CRC of a received
 *                                  character buffer.
 *
 * Arguments: uInt8_t * crcBuf    - Pointer to CRC buffer.
 *            sInt32_t bufLength  - Length of frame received.
 *            uInt16_t tempCrc    - Temporary CRC value.
 * Return:    uInt16_t crcResult    - Calculated CRC value of buffer
 *                                  received.
 * Notes/Preconditions            - This function should be called only
 *                                  when character buffer is received.
 ******************************************************************************/
uInt16_t calculateCrcBuf(uInt8_t * crcBuf, sInt32_t bufLength, uInt16_t tempCrc)
{
	uInt16_t crcResult = ZERO;

    // Checking if input is invalid
    if (NULL == crcBuf)
    {
    	// If no data in crcBuf then return with tempCrc which is initialized to
    	// ZERO before call


    }
    else // calculating CRC if input is Valid
    {
        while (bufLength--)
        {
            // Update the CRC value with a new byte
            tempCrc = xmodemCrc(*crcBuf, tempCrc);
            crcBuf++; // Increment a byte
        }
        crcResult = tempCrc;
    }
    return (crcResult);
}   // End of calculateCrcBuf()

