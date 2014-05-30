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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#include "commonTypes.h"
#include "rsi.h"
#include "led.h"

// UNIX_PATH_MAX needed for domain socket
#define	UNIX_PATH_MAX	108

// handler for socket messages
int connection_handler( int connection_fd, int ctr, int write_it )
{
	int nbytes;
	char buffer[256];
	char myMsg[ 4 ];
	//printf( "Entering the connection handler.\n" );

	if( write_it == 1) {
		sprintf( myMsg, "%02d",ctr );
		nbytes = snprintf(buffer, 256, myMsg);
		//printf( "\033[035m->  SENDING TO leet:\033[037m   %s\n", myMsg);
		printf( "." );
		write(connection_fd, buffer, nbytes);
		// next time, read reply
	} else {
		nbytes = read(connection_fd, buffer, 256);
		buffer[nbytes] = 0;
		//printf("\033[032m<-  MESSAGE FROM leet:\033[034m %s\033[037m\n", buffer);
		printf( "%s", buffer );
		// next time, write a command
	}
	close(connection_fd);
	return 0;
}

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

	// ======================= SET UP SOCKET ==========================
	// socket will be ./demo_socket
	// socket vars
	struct sockaddr_un address;
	int socket_fd, connection_fd, writeToLeet;
	socklen_t address_length;
	pid_t child;
	address_length = 0;
	writeToLeet = 1;
	int counter = 0;
	char ctrString[ 4 ];

	// ignore the SIGCHLD signal to prevent zombie processes
	signal( SIGCHLD, SIG_IGN );

	printf( "Starting socket...\n" );

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
	  printf( "socket() failed\n" );
	  return 1;
	}

	unlink("./demo_socket");

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, "./demo_socket");

	if(bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
	{
		printf( "bind() failed\n" );
		return 1;
	}

	if(listen(socket_fd, 5) != 0)
	{
		printf( "listen() failed\n" );
		return 1;
	}

	// ======================= MAIN LOOP ==========================
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

			// =================== SOCKET HANDLING ========================
			// send/receive socket messages
			// NOTE: accept() is blocking in this usage
			connection_fd = accept( socket_fd, (struct sockaddr *) &address, &address_length );

			// accept() returns a nonnegative integer that is a descriptor for the accepted socket
			if( connection_fd != -1)
			{
				child = fork();
				if(child == 0)
				{
					// always send the 0x01 command for now
					return connection_handler(connection_fd, 1, writeToLeet);

				} else {
					close(connection_fd);
				} // if child
			} else {
				printf( "Error accepting message.\n" );
				perror( "accept" );
			} // if connection_fd

			/*
			if( writeToLeet == 1) {
				writeToLeet = 0;
			} else {
				writeToLeet = 1;
			} // if writeToLeet
			*/
		} // if status == STATUS_SUCCESS

		//are we done? set the flag...
		//running = 0;




	}	//while running
	printf( "Closing socket...\n" );
	close(socket_fd);
	unlink("./demo_socket");
	printf("done...\n");

	//----- close the UART, release the GPIOs, etc -----
	rsiClose(rsi_fd);
	return 0;
}

