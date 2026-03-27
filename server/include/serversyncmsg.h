#ifndef SERVERSYNCMSG_H
#define SERVERSYNCMSG_H

#include "macros.h"

bool sndMsg_sync(int sock, char* snd_msg, struct sockaddr_in *snd_addr,socklen_t msg);

#endif