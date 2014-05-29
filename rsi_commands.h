/*
 * rsi_commands.h
 *
 *  Created on: May 22, 2014
 *      Author: kbroerman
 */

#ifndef RSI_COMMANDS_H_
#define RSI_COMMANDS_H_

#include "commonTypes.h"

sInt32_t processPollRsd(uInt8_t * rsiResponseFrame);
sInt32_t processPollApm(uInt8_t * rsiResponseFrame);
sInt32_t processApmTimedUnlock(uInt8_t * rsiRequestFrame, uInt8_t * rsiResponseFrame);

#endif /* RSI_COMMANDS_H_ */
