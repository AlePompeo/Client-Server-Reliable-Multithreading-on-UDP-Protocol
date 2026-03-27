#include "macros.h"
#include "ClientSender.h"
#include "clientfileoperation.h"
#include "reliableUDP.h"

nodo* phead = NULL; 

nodo* alloc_node(void) {
    nodo* new;
    new = malloc(sizeof(nodo));
    if (new == NULL){
        perror(KRED "[SENDER]: " KWHT "Error function malloc" RESET);
        exit(EXIT_FAILURE);
    }
    return new;
}

unsigned long calculateSampleRTT(struct timeval *start, struct timeval *end) {
    long seconds = end->tv_sec - start->tv_sec;
    long microseconds = end->tv_usec - start->tv_usec;

    // Gestione del caso in cui i microsecondi del timestamp finale siano inferiori a quelli iniziali
    if (microseconds < 0) {
        seconds -= 1;
    }

    // Calcolo del SampleRTT in microsecondi
    unsigned long sampleRTT = ((seconds * 1000000L + end->tv_usec) - start->tv_usec);
    // Limita il valore massimo del SampleRTT (ad esempio, 1 secondo = 1.000.000 microsecondi)
    if (sampleRTT > 1000000L) {
        sampleRTT = 500;
    }

    return sampleRTT;
}

nodo* insert_in_queue(nodo* head, unsigned int seq_num) {
    nodo* new_node = alloc_node();
    new_node->next = NULL;
    new_node->seq_num = seq_num;
    new_node->finished = 0;
    if(gettimeofday(&new_node->timer, NULL) == -1){  // Riempie la struttura timer del nodo
        perror(KRED "[SENDER]: " KWHT "Errore in gettimeofday()\n" RESET);
        exit(EXIT_FAILURE);
    } 
    if (head == NULL){
        return new_node;
    }
    nodo* p;
    for (p = head; p->next != NULL; p = p->next);
    p->next = new_node;
    new_node->next = NULL;
    return head;
}

nodo* delete_node_in_head(nodo* head) {
    if (head != NULL) {
        nodo* temp = head;  // Salva il nodo corrente
        head = head->next;  // Aggiorna la testa della lista
        free(temp);         // Libera il nodo rimosso
    }
    return head;
}

nodo* remove_nodo(nodo* head, unsigned int seq_num, struct timeval* t) {
    nodo* prev = NULL;
    nodo* curr = head;

    while (curr != NULL) {
        if (curr->seq_num == seq_num) { // Nodo trovato
            if (prev == NULL) { // Il nodo da eliminare è il primo della lista
                head = curr->next;
            } else { // Il nodo da eliminare non è il primo
                prev->next = curr->next;
            }
            *t = curr->timer; // Copia il timer del nodo eliminato
            free(curr); // Libera il nodo
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    return head;
}

void sndBufInit(struct snd_thread_info* snd) {
    for(int i = 0; i < N; i++) {
        snd->buf[i] = NULL;
    }
}

char* addSeqNum(char* data, unsigned int seqnum) {
    size_t data_len = strlen(data);
    size_t buffer_size;

    // Determina la dimensione del buffer in base al numero di sequenza
    if (seqnum >= N) {
        buffer_size = HEADER + 2; // HEADER + spazio + terminatore
    } else {
        buffer_size = HEADER + 1 + data_len + 1; // HEADER + spazio + dati + terminatore
    }

    // Alloca memoria per il buffer
    char* data_send = malloc(buffer_size);
    if (data_send == NULL) {
        perror("Error allocating memory for data_send");
        exit(EXIT_FAILURE);
    }

    // Formatta il numero di sequenza e aggiunge i dati
    if (seqnum >= N) {
        snprintf(data_send, buffer_size, "%d ", seqnum); // Solo numero di sequenza e spazio
    } else {
        snprintf(data_send, buffer_size, "%d %s", seqnum, data); // Numero di sequenza, spazio e dati
    }

    return data_send;
}

void* thread_send_job(void* p) {
    struct snd_thread_info* snd = p;
    unsigned int full_pos;
    phead = NULL;
    while(1) {
        struct snd_pack* pkt = malloc(sizeof(struct snd_pack));
        if(pkt == NULL) {
            exit(EXIT_FAILURE);
        }
        int size_read = readFile(snd->fd, pkt->data);
        pkt->data[size_read] = '\0';
        if (pthread_mutex_lock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        full_pos = (snd->send_base + WS) % N;
        while (full_pos == snd->next_tosend) {	     // Controllo se la finestra è piena
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            if (pthread_mutex_lock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            full_pos = (snd->send_base + WS) % N;
        }
        if (size_read == 0) {	// Se leggo 0 bytes ho finito il mio compito
            pkt->finished = true;		// Segnalo ultimo pacchetto
            pkt->acked = false;
            pkt->retransmitted = false;
            snd->buf[snd->next_tosend] = pkt;	// Inserisco il pacchetto
            pkt->seqnum = snd->next_tosend + N;	// Numero di sequenza fittizio per il ricevitore
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            pthread_exit(0);
        }
        pkt->seqnum = snd->next_tosend;
        pkt->acked = false;
        pkt->finished = false;
        pkt->retransmitted = false;
        socklen_t slen = sizeof(snd->si_other);
        float ran = random() / RAND_MAX;
        if(ran < (1 - P)) {  // Invio pacchetto
            calculateChecksum(pkt->data, strlen(pkt->data));
            char* data_send = addSeqNum(pkt->data, pkt->seqnum);
            if(sendto(snd->sock_fd, data_send, strlen(data_send) + 1, 0, (struct sockaddr*)&snd->si_other, slen) < 0) {
                perror(KRED "[SENDER]: " KWHT "Error in sendto sender()\n" RESET);
                exit(EXIT_FAILURE);
            }
            free(data_send);
            if(!PERFORMANCE) {
                printf(KRED "[ACK MANAGER]: " KWHT "Sent packet: %d\n" RESET, pkt->seqnum);
            }
            if (pkt->seqnum == 99) {
                usleep(2);
                if(!PERFORMANCE) {
                    printf(KRED "[ACK MANAGER]: " KWHT "Transmitted all N packets of the sliding window. Resetting the window to 0\n" RESET);
                }
            }
        }
        phead = insert_in_queue(phead, pkt->seqnum); // Inserisco nodo nella timeout list
        snd->buf[snd->next_tosend] = pkt;
        snd->next_tosend = (snd->next_tosend + 1) % N;
        if (pthread_mutex_unlock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        usleep(1);
    }
}

void* thread_timeout_job(void* p) {
    struct snd_thread_info* snd = p;
    snd->timeoutManager = pthread_self(); // Salvo ID thread nella struttura snd_thread_info
    while(1) {
        if (pthread_mutex_lock(&snd->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        if(phead == NULL) {	// Timeout list vuota
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            continue;
        }
        struct timeval t1, t2;
        if(gettimeofday(&t1, NULL) == -1){
            exit(EXIT_FAILURE);
        }
        t2 = phead->timer;	// Il timeout del pacchetto in testa è il primo ad essere controllato
        unsigned long diff = ((t1.tv_sec - t2.tv_sec) * 1000000L + t1.tv_usec) - t2.tv_usec;
        if(diff > snd->t.TIMEOUT){  // Se timeout scaduto, rinvio il pacchetto
            snd->t.TIMEOUT += (1/P);   // Raddoppio timeout per evitare ritrasmissioni eccessive
            struct snd_pack* pkt = snd->buf[phead->seq_num % N];
            pkt->retransmitted = true;
            float ran = random() / RAND_MAX;
            int slen = sizeof(snd->si_other);
            if(ran < (1 - P)) {
                char* data_send = addSeqNum(pkt->data, pkt->seqnum);
                errno = 0;
                calculateChecksum(pkt->data, strlen(pkt->data));
                if(sendto(snd->sock_fd, data_send, strlen(data_send) + 1, 0, (struct sockaddr*)&snd->si_other, slen) < 0) {
                    exit(EXIT_FAILURE);
                }
                free(data_send);
                // Stampa esplicita per ritrasmissione
                if(!PERFORMANCE) {
                    printf(KRED "[RETRANSMISSION]: " KWHT "Packet %d retransmitted due to timeout expiration\n" RESET, pkt->seqnum);
                }
            }
            phead = delete_node_in_head(phead); // Rimuovo pacchetto ritrasmesso dalla testa della lista
            phead = insert_in_queue(phead, pkt->seqnum); // Reinserisco pacchetto in coda alla lista
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
        }
        else {
            if (pthread_mutex_unlock(&snd->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
        }
        usleep(1);
    }
}

int parseAck(char* ack) {
    int value;
    char* p;
    errno = 0;
    value = (int) strtoul(ack, &p, 10);
    if (errno != 0 && *p != '\0'){
        perror(KRED "[SENDER]: " KWHT "Error function strtoul" RESET);
        exit(EXIT_FAILURE);
    }
    return value;
}

void setRcvTimeout(int sockfd, long unsigned timeout) {
    struct timeval t;
    if(timeout >= 1000000) {  // Divido i secondi dai microsecondi
        t.tv_sec = timeout / 1000000;
        t.tv_usec = timeout % 1000000;
    }
    else {
        t.tv_sec = 0;
        t.tv_usec = timeout;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
        perror(KRED "[RECEIVER]: " KWHT "Error in setsockopt()\n" RESET);
        exit(EXIT_FAILURE);
    }
}

void final_job(struct snd_thread_info* snd_ti, struct snd_pack* pkt, char* ACK_torcv) {
    int temp = 0;
    float ran = random() / RAND_MAX;
    int slen = sizeof(snd_ti->si_other);
    if(ran < (1 - P)) {
        char* data_send = addSeqNum(pkt->data, pkt->seqnum);
        calculateChecksum(pkt->data, strlen(pkt->data));
        if(sendto(snd_ti->sock_fd, data_send, strlen(data_send) + 1, 0, (struct sockaddr*)&snd_ti->si_other, slen) < 0) {
            exit(EXIT_FAILURE);
        }
        if(!PERFORMANCE) {
            temp = pkt->seqnum % UINT_MAX;
            printf(KRED "[ACK MANAGER]: " KWHT "Sent packet: %d\n" RESET, temp);
        }
    }
    phead = insert_in_queue(phead, pkt->seqnum); // Inserisco pacchetto finale nella lista; timeoutManager gestirà ritrasmissioni
    setRcvTimeout(snd_ti->sock_fd, snd_ti->t.TIMEOUT);    // Aggiorno timeout
    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
         exit(EXIT_FAILURE);
    }
    while(true) {  // Attendo notizie dal ricevitore
        if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        errno = 0;
        if(recvfrom(snd_ti->sock_fd, ACK_torcv, HEADER + 1, 0, (struct sockaddr*)&snd_ti->si_other, (unsigned int*)&slen) < 0) {
            if(!PERFORMANCE) {
                printf(KRED "[ACK MANAGER]: " KWHT "ACK ->%s\n, pkt->seqnum %d\n" RESET, ACK_torcv, pkt->seqnum);
            }
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                if(!PERFORMANCE) {	
                    printf(KRED "[ACK MANAGER]: " KWHT "Timer expired\n" RESET);
                }
                setRcvTimeout(snd_ti->sock_fd, snd_ti->t.TIMEOUT);
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }
            if(errno == ECONNREFUSED){ 					
                break;
            }
            exit(EXIT_FAILURE);
        }
        int final_ACK = parseAck(ACK_torcv);
        if(final_ACK >= N){  // ACK finale ricevuto
            if(!PERFORMANCE) {
                int temp1 = final_ACK % UINT_MAX;
                printf(KRED "[ACK MANAGER]: " KWHT "Received ACK: %d\n" RESET, temp1);
            }
            phead = delete_node_in_head(phead);
            free(snd_ti->buf[snd_ti->send_base]);
            snd_ti->buf[snd_ti->send_base] = NULL;
            break;
        }
        if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
    }
    if(pthread_cancel(snd_ti->timeoutManager) != 0){	// Cancello thread timeoutManager
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
        exit(EXIT_FAILURE);
    }
}

void thread_ack_job(void* p) {
    struct snd_thread_info* snd_ti = p;
    long unsigned SampleRTT = 0;
    struct timeval *t1, *t2;
    t1 = malloc(sizeof(struct timeval));
    t2 = malloc(sizeof(struct timeval));
    int slen = sizeof(snd_ti->si_other);
    while(1) {
        char ACK_torcv[HEADER + 1];
        unsigned int ACK_num;
        if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        if(snd_ti->send_base == snd_ti->next_tosend) {  // Nessun ACK da ricevere
            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
        }
        struct snd_pack* pkt = snd_ti->buf[snd_ti->send_base];
        while(pkt == NULL) {
            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            if (pthread_mutex_lock(&snd_ti->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            pkt = snd_ti->buf[snd_ti->send_base];
        }
        if(pkt->finished) { // Tutti gli ACK sono stati ricevuti
            if(!PERFORMANCE) {
                printf(KRED "[ACK MANAGER]: " KWHT "Received all ACK\n" RESET);
            }
            final_job(snd_ti, pkt, ACK_torcv);
            return;
        }
        setRcvTimeout(snd_ti->sock_fd, snd_ti->t.TIMEOUT); // Aggiorno timeout
        errno = 0;
        if(recvfrom(snd_ti->sock_fd, ACK_torcv, HEADER + 1, 0, (struct sockaddr*)&snd_ti->si_other, (unsigned int*)&slen) < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }
            if(errno == ECONNREFUSED){ 					
                return;
            }
            exit(EXIT_FAILURE);
        }
        if(parseAck(ACK_torcv) == -1) {  // Scarto ack non valido
            if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                exit(EXIT_FAILURE);
            }
            usleep(1);
            continue;
        }
        ACK_num = (unsigned int) parseAck(ACK_torcv);
        if(snd_ti->send_base < (snd_ti->send_base + WS) % N) {  // Caso in cui la finestra non è spezzata
            if((ACK_num >= snd_ti->send_base) && (ACK_num < snd_ti->send_base + WS)) { // ACK nella finestra
                if((snd_ti->buf[ACK_num])->acked == true){  // ACK già ricevuto, lo scarto
                    if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                        exit(EXIT_FAILURE);
                    }
                    usleep(1);
                    continue;
                }
                (snd_ti->buf[ACK_num])->acked = true;
                if(gettimeofday(t1, NULL) == -1){
                    exit(EXIT_FAILURE);
                }
                phead = remove_nodo(phead, ACK_num, t2);
                if(!(snd_ti->buf[ACK_num])->retransmitted){  // Se pacchetto non ritrasmesso
                    SampleRTT = calculateSampleRTT(t2, t1);  // Calcola il SampleRTT
                    snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                    if(!PERFORMANCE) {
                        printf(KRED "[ACK MANAGER]: " KWHT "Packet %d not retransmitted\n" RESET, pkt->seqnum);
                    }
                }
                if(!PERFORMANCE) {
                    printf(KRED "[ACK MANAGER]: " KWHT "Received ACK: %d | Timeout: %lu\n" RESET, ACK_num, snd_ti->t.TIMEOUT);
                }
                if(snd_ti->send_base == ACK_num) {  // Aggiorno send_base se l'ACK ricevuto corrisponde
                    int old_base = snd_ti->send_base;
                    while((snd_ti->buf[snd_ti->send_base])->acked) {
                        free(snd_ti->buf[snd_ti->send_base]);
                        snd_ti->buf[snd_ti->send_base] = NULL;
                        snd_ti->send_base = (snd_ti->send_base + 1) % N;
                        if(snd_ti->buf[snd_ti->send_base] == NULL)
                            break;
                    }
                    if((unsigned int)old_base != snd_ti->send_base) {
                        if(!PERFORMANCE) {
                            printf(KRED "[SLIDING WINDOW]: " KWHT "Sliding window moved: old send_base = %d, new send_base = %d\n" RESET, old_base, snd_ti->send_base);
                        }
                    }
                }
            }
        }
        else if (ACK_num >= snd_ti->send_base) {  // Caso finestra spezzata (pacchetto fine buffer)
            if((snd_ti->buf[ACK_num])->acked == true){  // ACK già ricevuto, lo scarto
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }
            (snd_ti->buf[ACK_num])->acked = true;
            if(gettimeofday(t1, NULL) == -1){
                exit(EXIT_FAILURE);
            }
            phead = remove_nodo(phead, ACK_num, t1);
            if(!(snd_ti->buf[ACK_num])->retransmitted){  // Se pacchetto non ritrasmesso
                SampleRTT = calculateSampleRTT(t2, t1);  // Calcola il SampleRTT
                snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                if(!PERFORMANCE) {
                    printf(KRED "[ACK MANAGER]: " KWHT "Packet %d not retransmitted\n" RESET, pkt->seqnum);
                }
            }
            if(!PERFORMANCE) {
                printf(KRED "[ACK MANAGER]: " KWHT "Received ACK: %d | Timeout: %lu\n" RESET, ACK_num, snd_ti->t.TIMEOUT);
            }
            if(snd_ti->send_base == ACK_num) {  // Aggiorno send_base se l'ACK ricevuto corrisponde
                int old_base = snd_ti->send_base;
                while((snd_ti->buf[snd_ti->send_base])->acked) {
                    free(snd_ti->buf[snd_ti->send_base]);
                    snd_ti->buf[snd_ti->send_base] = NULL;
                    snd_ti->send_base = (snd_ti->send_base + 1) % N;
                    if(snd_ti->buf[snd_ti->send_base] == NULL)
                        break;
                }
                if((unsigned int)old_base != snd_ti->send_base) {
                    if(!PERFORMANCE) {
                        printf(KRED "[SLIDING WINDOW]: " KWHT "Sliding window moved: old send_base = %d, new send_base = %d\n" RESET, old_base, snd_ti->send_base);
                    }
                }
            }
        }
        else if (ACK_num < (snd_ti->send_base + WS) % N) {  // Caso finestra spezzata (pacchetto inizio buffer)
            if((snd_ti->buf[ACK_num])->acked == true){  // ACK già ricevuto, lo scarto
                if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
                    exit(EXIT_FAILURE);
                }
                usleep(1);
                continue;
            }
            (snd_ti->buf[ACK_num])->acked = true;
            if(gettimeofday(t1, NULL) == -1){
                exit(EXIT_FAILURE);
            }
            phead = remove_nodo(phead, ACK_num, t2);
            if(!(snd_ti->buf[ACK_num])->retransmitted){  // Se pacchetto non ritrasmesso
                SampleRTT = calculateSampleRTT(t2, t1);  // Calcola il SampleRTT
                snd_ti->t.TIMEOUT = estimateTimeout(&snd_ti->t.EstimatedRTT, &snd_ti->t.DevRTT, SampleRTT);
                if(!PERFORMANCE) {
                    printf(KRED "[ACK MANAGER]: " KWHT "Packet %d not retransmitted\n" RESET, pkt->seqnum);
                }
            }
            if(!PERFORMANCE) {
                printf(KRED "[ACK MANAGER]: " KWHT "Received ACK: %d | Timeout: %lu\n" RESET, ACK_num, snd_ti->t.TIMEOUT);
            }
            if(snd_ti->send_base == ACK_num){  // Aggiorno send_base se l'ACK ricevuto corrisponde
                int old_base = snd_ti->send_base;
                while((snd_ti->buf[snd_ti->send_base])->acked) {
                    free(snd_ti->buf[snd_ti->send_base]);
                    snd_ti->buf[snd_ti->send_base] = NULL;
                    snd_ti->send_base = (snd_ti->send_base + 1) % N;
                    if(snd_ti->buf[snd_ti->send_base] == NULL)
                        break;
                }
                if((unsigned int)old_base != snd_ti->send_base) {
                    if(!PERFORMANCE) {
                        printf(KRED "[SLIDING WINDOW]: " KWHT "Sliding window moved: old send_base = %d, new send_base = %d\n" RESET, old_base, snd_ti->send_base);
                    }
                }
            }
        }
        if(!PERFORMANCE) {
            printf(KRED "[ACK MANAGER]: " KWHT "SampleRTT:%lu | EstimatedRTT:%lu | DevRTT:%lu | TIMEOUT:%lu\n\n" RESET, SampleRTT, snd_ti->t.EstimatedRTT, snd_ti->t.DevRTT, snd_ti->t.TIMEOUT);
        }
        if (pthread_mutex_unlock(&snd_ti->mtx) != 0) {
            exit(EXIT_FAILURE);
        }
        usleep(1);
    }
}

void sendFile(int sockfd, int fd, struct sockaddr_in si_other) {
    pthread_t tid;
    struct snd_thread_info* snd = malloc(sizeof(struct snd_thread_info));
    if (snd == NULL){
        perror(KRED "[SENDER]: " KWHT "Error function malloc" RESET);
        exit(EXIT_FAILURE);
    }
    sndBufInit(snd);
    snd->send_base = 0;
    snd->next_tosend = 0;
    snd->fd = fd;
    snd->sock_fd = sockfd;
    snd->t.EstimatedRTT = 0;
    snd->t.DevRTT = 0;
    snd->t.TIMEOUT = 1000000;
    snd->si_other = si_other;
    if(pthread_mutex_init(&snd->mtx, NULL) != 0) {
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&tid, NULL, thread_send_job, snd) != 0){
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&tid, NULL, thread_timeout_job, snd) != 0){
        exit(EXIT_FAILURE);
    }
    // Lavoro thread_main
    thread_ack_job(snd);
}