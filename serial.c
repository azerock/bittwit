#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>  // TODO: remove this if it is no longer necessary
#include "serial.h"

#define DEFAULTPORT 1
#define MINPORT 1
#define MAXPORT 10

PBTPort open_port(int port_number)
{
	PBTPort pbtport = malloc(sizeof(BTPort));
	if (!pbtport)
		return(NULL);

	pbtport->port_number = port_number;

	char device[11];
	sprintf(device, "/dev/ttyS%i", port_number - 1);

	// might need to consider O_RDWR | O_ASYNC
	// if we want data back. for now, write-only
	pbtport->handle = open(device, O_WRONLY | O_NOCTTY | O_SYNC);
	if (!pbtport->handle) {
		free(pbtport);
		return (NULL);
	}

	struct termios termios_p;
	tcgetattr(pbtport->handle, &termios_p);

	// "Raw" mode from termios man page
	cfmakeraw(&termios_p);
/*  // Use the following on systems that don't implement cfmakeraw()
	termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_p->c_cflag &= ~(CSIZE | PARENB);
	termios_p->c_cflag |= CS8;
*/

	cfsetspeed(&termios_p, B9600);

	tcsetattr(pbtport->handle, TCSAFLUSH, &termios_p);

	return (pbtport);
}
void close_port(PBTPort pbtport)
{
	close(pbtport->handle);
	free(pbtport);
}

int write_port(PBTPort pbtport, char* data, int length)
{ // returns TRUE if data written successfully, FALSE otherwise
	errno = 0;
	int written = write(pbtport->handle, data, length);
	return (!errno && written == length);
}

#if 0
int OLDmain(int argc, char * argv[])
{
	PBTPort pbtport;
	int port_number = DEFAULTPORT;

	if (argc > 1) {
		errno = 0;
		port_number = strtol(argv[1], NULL, 10);
		if (errno != 0 || port_number < MINPORT || port_number > MAXPORT) {
			// TODO: yell at user
			return(1);
		}
	}

	printf("Using port %i\n", port_number);

	pbtport = open_port(port_number);
	if (!pbtport) {
		perror("open_port");
		return(1);
	}
/*
	// DAT
	char buffer[] = { 0,0,0,4,
		0x0B, 0x0B, 0x0B, 0x0B,
		0,0,0,6, 0,0,0,4,
		0,0,0,0 };
	int res = write_port(pbtport, buffer, 4*4 + 4);
	if (!res)
		perror("write_port");

	sleep(1);
	
	char buffer2[] = { 0,0,0,4,
		0x03, 0x03, 0x03, 0x03,
		0,0,0,6, 0,0,0,4,
		0,0,0,0 };
	res = write_port(pbtport, buffer2, 4*4 + 4);
	if (!res)
		perror("write_port");
	// END DAT
*/
	close_port(pbtport);

	return(0);
}
#endif
