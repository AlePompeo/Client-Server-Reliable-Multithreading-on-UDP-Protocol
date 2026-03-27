#include "macros.h"
#include "ClientReceiver.h"
#include "clientfileoperation.h"
#include "reliableUDP.h"

void rcvBufInit(struct rcv_info *rcv_inf) {
    for(int i = 0; i < N; i++) {
        rcv_inf->buf[i] = NULL;
    }
}

// Funzione per calcolare la differenza di tempo in microsecondi
long calculateTimeDifference(struct timeval start, struct timeval end) {
    return (long) ((end.tv_sec - start.tv_sec) * 1000000L + end.tv_usec) - start.tv_usec;
}

void sendAcknowledgement(int sock_fd,int seq_num,struct sockaddr_in si_other){
    float ran = random()/RAND_MAX;
    int slen = sizeof(si_other);

    if(ran < (1 - P)) {

        char ACK_tosend[MAX_LINE_SIZE + 1];

        if(sprintf(ACK_tosend, "%d", seq_num) == 0) {  		//copio il numero di sequenza convertito in stringa in ACK_tosend
            perror(KRED "[RECEIVER]: " KWHT "Error in sprintf()\n" RESET);
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        if(sendto(sock_fd, ACK_tosend, strlen(ACK_tosend) + 1,0,(struct sockaddr*)&si_other, slen) < 0) {
            perror(KRED "[RECEIVER]: " KWHT "Error in sendto()\n" RESET);
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }
        if (!PERFORMANCE) {
            printf(KRED "[ACK MANAGER]: " KWHT "Sent ACK for packet %d\n" RESET, seq_num);
        }
    }

}

void setTimeoutRcv(int sockfd, long timeout){

    struct timeval t;

    if(timeout >= 1000000){			//per evitare errori di range nell'assegnazione
        t.tv_sec = timeout / 1000000;
        t.tv_usec = timeout % 1000000;
    }
    else{
        t.tv_sec = 0;
        t.tv_usec = timeout;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
        perror(KRED "[RECEIVER]: " KWHT "Error in setsockopt()\n" RESET);
        exit(EXIT_FAILURE);
    }
}

unsigned int parseLine(char *buffer,char *data){

    char* p = NULL;
    unsigned int seqnum;

    errno = 0;
    seqnum = (unsigned int) strtoul(buffer ,&p ,0);	//(1)

    if(errno != 0 || *p != ' '){
        perror(KRED "[RECEIVER]: " KWHT "Error in strtoul\n" RESET);
        sleep(5);//evita chiusura brusca terminale
        exit(EXIT_FAILURE);
    }
    p++;
    data = strcpy(data, p);

    return seqnum;

}

void rcvFile(int sock_fd,int fd,long size,struct sockaddr_in si_other){
    unsigned int attempt = 0;
    int ack_to_resend = -1;
    struct rcv_info *rcv_inf;
    long sampleRTT = 0;

    rcv_inf = malloc(sizeof(struct rcv_info));
    if(rcv_inf == NULL){
        perror(KRED "[RECEIVER]: " KWHT "Error function malloc" RESET);
        sleep(5);//evita chiusura brusca terminale
        exit(1);
    }
    rcvBufInit(rcv_inf);
    rcv_inf->sock_fd = sock_fd;
    rcv_inf->fd=fd;
    rcv_inf->file_size = size;
    rcv_inf->rcv_base = 0;
    rcv_inf->si_other = si_other;

    rcv_inf->t.EstimatedRTT = 0;
    rcv_inf->t.DevRTT = 0;
    rcv_inf->t.TIMEOUT = 1000000;

    setTimeoutRcv(rcv_inf->sock_fd, rcv_inf->t.TIMEOUT);

    while (1) {
        char pack_rcv[HEADER + 1 + DATA_SIZE + 1];
        unsigned int last_pos;

        struct timeval start_time, end_time;

        // Inizio del timer per il SampleRTT
        gettimeofday(&start_time, NULL);

        errno = 0;
        if(recvfrom(rcv_inf->sock_fd, pack_rcv, HEADER + 1 + DATA_SIZE + 1, 0, NULL, 0) < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK){ 	//RCV_FILE_TIMEOUT ricezione scaduto
                if(attempt < MAXIMUM_ATTEMPT){		//il mittente non risponde
                    sendAcknowledgement(rcv_inf->sock_fd, ack_to_resend,rcv_inf->si_other);	//testo connessione (ICMP)

                    attempt++;
                    continue;
                }
                fprintf(stderr, "\n\nIl server non mostra segni di attività!\n");
                fprintf(stderr, "\nImpossibile completare download!\n");
                free(rcv_inf);
                return;
            }
            if(errno == ECONNREFUSED){		//connessione interrotta
                fprintf(stderr, "\n\nConnessione interrotta!\n");
                fprintf(stderr, "\nImpossibile completare download!\n");
                free(rcv_inf);
                return;
            }					//errore inatteso
            perror("\nErrore inatteso della funzione recvfrom(). Programma terminato\n");
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        rcv_inf->t.TIMEOUT = estimateTimeout(&rcv_inf->t.EstimatedRTT, &rcv_inf->t.DevRTT, sampleRTT);

        attempt = 0;

        struct rcv_info_pack *ip = malloc(sizeof(struct rcv_info_pack));
        if(!ip) {
            fprintf(stderr, KRED "[RECEIVER]: " KWHT "Error in malloc()\n" RESET);
            sleep(5);//evita chiusura brusca terminale
            exit(EXIT_FAILURE);
        }

        ip->seqnum = parseLine(pack_rcv, ip->data);
        calculateChecksum(ip->data, strlen(ip->data));
        ack_to_resend = ip->seqnum;

        if(!PERFORMANCE) {
            printf(KRED "[ACK MANAGER]: " KWHT "Received packet %d\n" RESET, ip->seqnum);
            printf(KRED "[ACK MANAGER]: " KWHT "SampleRTT:%lu | EstimatedRTT:%lu | DevRTT:%lu | TIMEOUT:%lu\n\n" RESET, sampleRTT, rcv_inf->t.EstimatedRTT, rcv_inf->t.DevRTT, rcv_inf->t.TIMEOUT);    
        }

        if(ip->seqnum >= N){					//ricevuti tutti i pacchetti

            sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

            if(close(rcv_inf->sock_fd) == -1) {
                perror(KRED "[RECEIVER]: " KWHT "Error in close()\n" RESET);
                sleep(5);//evita chiusura brusca terminale
                exit(EXIT_FAILURE);
            }
            free(ip);

            if(rcv_inf->file_size != 0) {
                free(rcv_inf);
                if(!PERFORMANCE) {
                    printf(KRED "\n\n[ACK MANAGER]: " KWHT "Download completed!\n" RESET);
                }
            }
            return;
        }

        last_pos = (rcv_inf->rcv_base + WR) % N;

        if(!PERFORMANCE) {
            printf(KRED "[ACK MANAGER]: " KWHT "Sliding window: base=%d, last_pos=%d\n" RESET, rcv_inf->rcv_base, last_pos);
        }
        if(rcv_inf->rcv_base < last_pos) { 			//(1)caso base (la finestra non è spezzata)

            if((ip->seqnum >= rcv_inf->rcv_base) && (ip->seqnum < last_pos)) { //pacchetto nella finestra

                if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                    rcv_inf->buf[ip->seqnum] = ip;

                    sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                    if(ip->seqnum == rcv_inf->rcv_base){
                        while(ip != NULL){
                            writeFile(rcv_inf->fd, ip->data);
                            if(!PERFORMANCE) {
                                printf(KRED "[ACK MANAGER]: " KWHT "Written packet %d\n" RESET, ip->seqnum);
                            }
                            free(rcv_inf->buf[rcv_inf->rcv_base]);
                            rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                            rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                            ip = rcv_inf->buf[rcv_inf->rcv_base];
                        }
                    }
                }
                else{						//pacchetto già ricevuto (nella finestra)
                    sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
                }
            }
            else {							//pacchetto già ricevuto (fuori dalla finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else if (ip->seqnum >= rcv_inf->rcv_base) { 	//(2)caso finestra spezzata (pacchetto fine buffer)

            if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                rcv_inf->buf[ip->seqnum] = ip;

                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                if(ip->seqnum == rcv_inf->rcv_base){
                    while(ip != NULL){
                        writeFile(rcv_inf->fd, ip->data);
                        if(!PERFORMANCE) {
                            printf(KRED "[ACK MANAGER]: " KWHT "Written packet %d\n" RESET, ip->seqnum);
                        }
                        free(rcv_inf->buf[rcv_inf->rcv_base]);
                        rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                        rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = rcv_inf->buf[rcv_inf->rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else if (ip->seqnum < last_pos){		//(3)caso finestra spezzata (pacchetto inizio buffer)

            if(rcv_inf->buf[ip->seqnum] == NULL){ 		//pacchetto ancora non ricevuto
                rcv_inf->buf[ip->seqnum] = ip;

                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);

                if(ip->seqnum == rcv_inf->rcv_base){
                    while(ip != NULL){
                        writeFile(rcv_inf->fd, ip->data);
                        if(!PERFORMANCE) {
                            printf(KRED "[ACK MANAGER]: " KWHT "Written packet %d\n" RESET, ip->seqnum);
                        }
                        free(rcv_inf->buf[rcv_inf->rcv_base]);
                        rcv_inf->buf[rcv_inf->rcv_base] = NULL;
                        rcv_inf->rcv_base = (rcv_inf->rcv_base + 1) % N;//aggiorno posizione rcv_base
                        ip = rcv_inf->buf[rcv_inf->rcv_base];
                    }
                }
            }
            else{						//pacchetto già ricevuto (nella finestra)
                sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            }
        }
        else { 							//pacchetto già ricevuto (fuori dalla finestra)
            sendAcknowledgement(rcv_inf->sock_fd, ip->seqnum,rcv_inf->si_other);
            ack_to_resend = ip->seqnum;
        }
        // Fine del timer per il SampleRTT
        gettimeofday(&end_time, NULL);
        // Calcolo del SampleRTT
        sampleRTT = calculateTimeDifference(start_time, end_time);
    }

}