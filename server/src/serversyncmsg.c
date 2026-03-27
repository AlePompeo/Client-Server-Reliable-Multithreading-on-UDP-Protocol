#include "macros.h"
#include "serversyncmsg.h"
#include "serversender.h"

bool sndMsg_sync(int sock, char *snd_msg, struct sockaddr_in *snd_addr, socklen_t msg) {
    struct sockaddr_in rcv_addr = {0}; // Initialize to zero to avoid uninitialized memory issues
    socklen_t rcv_addr_len = sizeof(struct sockaddr);
    int retry_count = 0;
    char rcv_msg[7]; // Buffer per messaggi ricevuti

    setRcvTimeout(sock, SND_MSG_TIMEOUT);

    while (retry_count <= 2) {
        // Simula l'invio con perdita di pacchetti
        if ((float)rand() / RAND_MAX < (1 - (P*0.1))) {
            if (sendto(sock, snd_msg, strlen(snd_msg) + 1, 0, (struct sockaddr *)snd_addr, msg) == -1) {
                perror(KRED "[MSGSYNC]: " KWHT "Error in sendto()" RESET);
                return false;
            }
        }

        errno = 0;
        // Attende l'ACK
        if (recvfrom(sock, rcv_msg, sizeof(rcv_msg), 0, (struct sockaddr *)&rcv_addr, &rcv_addr_len) >= 0) {
            // Verifica che l'ACK provenga dall'indirizzo previsto
            if (rcv_addr.sin_addr.s_addr == snd_addr->sin_addr.s_addr &&
                ntohs(rcv_addr.sin_port) == ntohs(snd_addr->sin_port)) {
                
                // Gestisce il messaggio ricevuto
                if (strncmp(rcv_msg, "ACK", 3) == 0) {
                    strncpy(snd_msg, "SYN", 4); // Prepara il messaggio di fine (FIN)
                } else if (strncmp(rcv_msg, "SYNACK", 6) == 0) {
                    printf(KRED "[MSGSYNC]: " KGRN "Succesfully SYNC\n" RESET);
                    return true; // Sincronizzazione completata con successo
                } else {
                    exit(EXIT_FAILURE); // Errore di sincronizzazione
                }
            }
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror(KRED "[MSGSYNC]: " KWHT "Error in recvfrom()" RESET);
            return false;
        }

        retry_count++; // Incrementa il contatore dei tentativi
    }

    return false; // Dopo 3 tentativi falliti, rinuncia
}
