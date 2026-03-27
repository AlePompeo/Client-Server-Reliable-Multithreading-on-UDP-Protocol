#ifndef SERVERRECEIVER_H
#define SERVERRECEIVER_H

#include "macros.h"

struct rcv_info_pack {
    char data[DATA_SIZE + 1];	//+ 1 per '\0'
    unsigned int seqnum;
};

struct timeoutrcv{
    long unsigned EstimatedRTT;
    long unsigned DevRTT;
    long unsigned TIMEOUT;
};

struct rcv_info {
    int fd;
    int sock_fd;
    long file_size;
    unsigned int rcv_base; // num.seq base
    struct timeoutrcv t;
    struct sockaddr_in si_other;
    struct rcv_info_pack* buf[N];
};

long calculateTimeDifference(struct timeval start, struct timeval end);
void rcvBufInit(struct rcv_info *rcv_inf);
void sendAcknowledgement(int sock_fd,int seq_num,struct sockaddr_in si_other);
void setTimeoutRcv(int sockfd, long timeout);
unsigned int parseLine(char *buffer,char *data);
void rcvFile(int sock_fd,int fd,long size,struct sockaddr_in si_other);

#endif

