#include "macros.h"
#include "clientsyncmsg.h"
#include "ClientSender.h"

char* rcvMsg_sync(int sock, char *rcv, unsigned int size, struct sockaddr_in *servaddr) {
	bool SYN = false;
	socklen_t addr_len = sizeof(struct sockaddr);
	socklen_t len = sizeof(*servaddr);
	setRcvTimeout(sock, RCV_MSG_TIMEOUT);

	while (true) {
		char pack_rcv[size];
		errno = 0;

		// Receive packet
		if (recvfrom(sock, pack_rcv, size, 0, (struct sockaddr *)servaddr, &addr_len) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return SYN ? rcv : NULL;
			}
			return NULL;
		}

		// Handle specific error messages
		if (strncmp(pack_rcv, "Errore 4", strlen("Errore 4")) == 0 || 
			strncmp(pack_rcv, "Errore 5", strlen("Errore 5")) == 0) {
			return strcpy(rcv, pack_rcv);
		}

		// Handle SYN message
		if (strncmp(pack_rcv, "SYN", strlen("SYN")) == 0) {
			SYN = true;
			if ((float)random() / RAND_MAX < (1 - (P*0.1))) {
				if (sendto(sock, "SYNACK", 7, 0, (struct sockaddr *)servaddr, len) < 0) {
					perror("Errore in sendto() nel rcv_msg()\n");
					exit(EXIT_FAILURE);
				}
			}
			continue;
		}

		// Handle other messages
		rcv = strcpy(rcv, pack_rcv);
		if ((float)random() / RAND_MAX < (1 - (P*0.1))) {
			if (sendto(sock, "ACK", 4, 0, (struct sockaddr *)servaddr, len) < 0) {
				perror("Errore in sendto() nel rcv_msg()\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}




