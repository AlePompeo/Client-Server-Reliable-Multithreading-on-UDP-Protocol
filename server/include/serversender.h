#ifndef SERVERSENDER_H
#define SERVERSENDER_H

#include "macros.h"

struct snd_pack {
    char data[DATA_SIZE + 1];	//+ 1 per '\0'
    bool acked;
    unsigned int seqnum;
    bool finished;
    bool retransmitted;
};

struct timeout{
    long unsigned EstimatedRTT;
    long unsigned DevRTT;
    long unsigned TIMEOUT;
};

struct  snd_thread_info {
    pthread_t timeoutManager;
    pthread_mutex_t mtx;

    int fd;
    int sock_fd;

    struct timeout t;

    unsigned int send_base;
    unsigned int next_tosend;
    struct snd_pack* buf[N];
    struct sockaddr_in si_other;
};

struct node_t {
    unsigned int seq_num;
    bool finished;
    struct timeval timer;
    struct node_t *next;
};

typedef struct node_t nodo;
extern nodo* phead;

unsigned long calculateSampleRTT(struct timeval *start, struct timeval *end);
void sndBufInit(struct snd_thread_info *snd);
char* addSeqNum(char *data, unsigned int seqnum);
void* thread_send_job(void*p);
long unsigned estimateTimeout(long unsigned *EstimatedRTT, long unsigned *DevRTT, long unsigned SampleRTT);
void *thread_timeout_job(void *p);
int parseAck(char*ack);
void setRcvTimeout(int sockfd, long unsigned timeout);
void proceduraFinale(struct snd_thread_info *snd_ti, struct snd_pack *pkt, char* ACK_torcv);
void thread_ack_job(void* p);
void sendFile(int sockfd,int fd,struct sockaddr_in si_other);

nodo* alloc_node(void);
nodo* insert_in_queue(nodo* head,unsigned int seq_num); 
nodo* delete_node_in_head(nodo * head);
nodo* remove_nodo(nodo * head,unsigned int seq_num,struct timeval *t);

#endif