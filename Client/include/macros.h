#ifndef MACROS_H
#define MACROS_H

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <dirent.h>
#include <zlib.h>
#include <limits.h> /* PATH_MAX */


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
#define BUFLEN 32768 //Max length of buffer
#define DATA_SIZE 1500
#define MAX_LINE_SIZE 64
#define HEADER 10
#define MAXIMUM_ATTEMPT 10
#define N 50
#define WR N/2
#define WS N/2
#define P 0.2
#define RCV_MSG_TIMEOUT 1000000
#define MAX_PATH 4096
#define CHUNK 16384
#define INITIAL_BUFFER_SIZE 32768
#define ENCODE_BLOCK_SIZE 4096 // Blocchi più grandi per I/O
#define OUTPUT_BUFFER_SIZE (ENCODE_BLOCK_SIZE * 4 / 3 + 4) // Buffer Base64 massimo
#define FIXEDTIMEOUT 0
#define FIXEDRTT FIXEDTIMEOUT
#define T 1000000
#define RTT T
#define PERFORMANCE 0
#define DECODE_BLOCK_SIZE 4
#define OUTPUT_BLOCK_SIZE 3
#define INPUT_BUFFER_SIZE 4096
#define MAX_THREADS 1

#endif