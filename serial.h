#ifndef _BITTWIT_SERIAL_H_
#define _BITTWIT_SERIAL_H_

typedef struct
{
    int handle;
    int port_number;
} BTPort, *PBTPort;

PBTPort open_port(int port_number);
void close_port(PBTPort pbtport);
int write_port(PBTPort pbtport, char* data, int length);

#endif // _BITTWIT_SERIAL_H_
