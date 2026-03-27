#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <limits.h> /* PATH_MAX */
#include <wait.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <zlib.h>

//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

//macros
#define HEADER 10 //dimensione header del pacchetto udp
#define DATA_SIZE 1500
#define MAX_LINE_SIZE 64 //ack size receiver 
#define BUFLEN 512  //Max length of buffer
#define MAX_THREADS 50
#define N 50 //finestra scorrevole
#define P 0.2 //probabilità di perdita del pacchetto
#define WR N/2 //finestra scorrevole ricezione
#define WS N/2 //finestra scorrevole spedizione
#define MAXIMUM_ATTEMPT 10
#define SND_MSG_TIMEOUT 1000000
#define FIXEDTIMEOUT 0
#define FIXEDRTT FIXEDTIMEOUT
#define T 1000000
#define RTT T
#define PERFORMANCE 0
#define ENCODE_BLOCK_SIZE 4096 // Blocchi più grandi per I/O
#define OUTPUT_BUFFER_SIZE (ENCODE_BLOCK_SIZE * 4 / 3 + 4) // Buffer Base64 massimo
#define DECODE_BLOCK_SIZE 4
#define OUTPUT_BLOCK_SIZE 3
#define INPUT_BUFFER_SIZE 4096
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EAGAIN 11
#define EWOULDBLOCK EAGAIN
#define ECONNREFUSED 111

#endif