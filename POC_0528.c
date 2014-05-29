/*
 * Copyright (C) Copyright K Broerman Allegion 2014.
 *
 * Author: K Broerman
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the PG_ORGANIZATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY	THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS-IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "commonTypes.h"
#include "rsi.h"
#include "led.h"


int main(void) {	//this code emulates the RSI process...

	puts("RSI-BLE POC 1...");
	led_setValue(GREEN, 0);
	led_setValue(RED, 0);
	led_setValue(BLUE, 0);

	int rsi_fd = rsiOpen();
	int reqLength, respLength, count, status;
	int running = 1;


	char rsiRequestFrame[RSI_MAX_FRAME_SIZE];
	char rsiResponseFrame[RSI_MAX_FRAME_SIZE];

	while (running) {	//emulate RSI process

		//---- listen for RS485 command from ACP (master)
		memset(rsiRequestFrame,  0, sizeof(rsiRequestFrame));
		memset(rsiResponseFrame, 0, sizeof(rsiResponseFrame));
		reqLength = rsiRead(rsi_fd, rsiRequestFrame);

		//process the command, generate immediate response
		status = rsiHandler(rsiRequestFrame, rsiResponseFrame, &reqLength);

		//send immediate response back to ACP
		if (status == STATUS_SUCCESS) {
			//printf("need to send %d bytes back...\n", reqLength);
			rsiWrite(rsi_fd, rsiResponseFrame, reqLength);
		}

		//are we done? set the flag...
		//running = 0;

	}	//while running

	printf("done...\n");

	//----- close the UART, release the GPIOs, etc -----
	rsiClose(rsi_fd);
	return 0;
}

