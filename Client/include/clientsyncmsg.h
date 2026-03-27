#ifndef CLIENTSYNCMSG_H
#define CLIENTSYNCMSG_H

#include "macros.h"

char* rcvMsg_sync(int sock, char *rcv, unsigned int size, struct sockaddr_in *servaddr);
	
#endif		
