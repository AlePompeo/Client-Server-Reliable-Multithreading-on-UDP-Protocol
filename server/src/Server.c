// Progetto IIW
// Versione: 11.0
// Autore: Alessio Pompeo
// Autore: Davide Edoardo Stoica
// Autore: Consuelo Caporaso
// Anno: 2025

#include "macros.h"
#include "serverfileoperation.h"
#include "serversender.h"
#include "serverreceiver.h"
#include "serversyncmsg.h"
#include "encode64.h"
#include "decode64.h"
#include "reliableUDP.h"

//config
char ip_address[BUFLEN]; // Variabile globale per l'indirizzo IP  
char server_port[BUFLEN]; // Variabile globale per la porta predefinita
int signal_flag = 0; // Variabile globale per il segnale

//Save port and socket globally
static int connection_index = 0;
static int connection_ports[MAX_THREADS] = {0};
static int connection_socks[MAX_THREADS] = {0};
static time_t connection_time[MAX_THREADS] = {0};

//threads
pthread_t input_thread;
pthread_t threads[MAX_THREADS];
volatile bool process_on = 1;

//client
int client_number = 0;
int client_number_for_port = 0;
int thread_no = 0;

struct client_info {
    int sock;
    int port;
    socklen_t fromlen;
    struct sockaddr_in from;
    char buf[BUFLEN];
    struct sockaddr_in server_addr;
    char * filepath;
};

void die(char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

int read_file_list(char** filelist, char* token, char* cmd, struct client_info* clinf) {
    int Number_file = 0;
    const char *directory = "server_files"; // Directory corrente
    char *listfiles = list_files(directory);

    // Conta il numero di file nella lista
    char *tok = strtok(listfiles, "\n");
    while (tok != NULL) {
        Number_file++;
        tok = strtok(NULL, "\n");
    }

    // Ottiene il percorso del file utilizzando i parametri forniti
    clinf->filepath = obtain_path(clinf->filepath, token, cmd);

    // Apre il file in modalità lettura
    FILE *file = fopen(clinf->filepath, "r");
    if (file == NULL) {
        perror(KRED "[SERVER]: " KWHT "Error opening file" RESET);
        exit(EXIT_FAILURE);
    }
    
    char segment[BUFLEN];
    int i = 0;

    // Legge il contenuto del file in filelist, assicurandosi che fscanf legga esattamente 1 token
    while (fscanf(file, "%s", segment) == 1) {
        filelist[i] = strdup(segment); // Alloca spazio e copia il token in filelist
        if (filelist[i] == NULL) {
            perror(KRED "[SERVER]: " KWHT "Error allocating memory for filelist segment" RESET);
            exit(EXIT_FAILURE);
        }
        i++;
    }

    free(listfiles);
    memset(clinf->filepath, '\0', strlen(clinf->filepath));
    return Number_file;
}

void rebind_socket(struct client_info *clinf) {
    // Chiude la socket corrente
    close(clinf->sock);

    // Crea una nuova socket
    clinf->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clinf->sock < 0) {
        perror(KRED "[SERVER]: " KWHT "Error creating socket" RESET);
        exit(EXIT_FAILURE);
    }

    // Imposta il TOS (ad esempio, DSCP per priorità alta: 0x2E)
    int tos = 0x2E; // Expedited Forwarding (EF)
    if (setsockopt(clinf->sock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
        perror(KRED "[SERVER]: " KWHT "Error setting TOS" RESET);
    }

    // Configura l'indirizzo del server
    clinf->server_addr.sin_family = AF_INET;

    // Imposta un indirizzo IP specifico
    if (inet_aton(ip_address, &clinf->server_addr.sin_addr) == 0) { // Sostituisci con il tuo IP desiderato
        perror(KRED "[SERVER]: " KWHT "Error setting specific IP address" RESET);
        exit(EXIT_FAILURE);
    }

    // Configura la porta del server
    clinf->server_addr.sin_port = htons(clinf->port);

    // Effettua il bind della socket
    if (bind(clinf->sock, (struct sockaddr *)&clinf->server_addr, sizeof(clinf->server_addr)) < 0) {
        perror(KRED "[SERVER]: " KWHT "Error binding socket" RESET);
        exit(EXIT_FAILURE);
    }
}

void send_response(struct client_info *client) {
    if (sendto(client->sock, client->buf, sizeof(client->buf) + 10, 0, (struct sockaddr *)&client->from, sizeof(client->from)) == -1) {
        die("sendto()");
    }
}

void receive_response(struct client_info *client) {
    socklen_t fromlen = sizeof(client->from);
    if (recvfrom(client->sock, client->buf, sizeof(client->buf), 0, (struct sockaddr *)&client->from, &fromlen) == -1) {
        die("recvfrom()");
    }
}

void handle_empty_file(struct client_info *client) {
    fprintf(stderr,KRED "[CLIENT OBSERVER]: " KCYN "Empty file\n" RESET);
    strcpy(client->buf, "FAIL");
    send_response(client);
}

void handle_file_not_found(struct client_info *client) {
    fprintf(stderr,KRED "[CLIENT OBSERVER]: " KCYN "File not found in server_files\n" RESET);
    strcpy(client->buf, "FAIL");
    send_response(client);
}

void handle_error(struct client_info *client) {
    printf(KRED "[CLIENT OBSERVER]: " KCYN "File not found in Client Upload folder\n" RESET);
    closeFile(client->sock);
    rebind_socket(client);
    memset(client->filepath, '\0', BUFLEN);
}

void handle_exit(struct client_info *client) {
    // Pulisce il buffer del client
    memset(client->buf, 0, sizeof(client->buf));
    printf(KRED "[CLIENT OBSERVER]: " KGRN " Terminating session on port %d with socket %d\n" RESET, client->port, client->sock);
    // Riceve una risposta dal client
    receive_response(client);
    printf(KRED "[CLIENT OBSERVER]: " KBLU " Response received successfully. FIN\n" RESET); 
    // Invia un messaggio di FIN+ACK al client
    strcpy(client->buf, "FIN+ACK");
    send_response(client);
    sleep(1); // Attende un secondo per garantire che il messaggio sia ricevuto
    // Riceve un'ulteriore risposta dal client
    receive_response(client);
    printf(KRED "[CLIENT OBSERVER]: " KBLU " Response received successfully. ACK \n" RESET);
    // Invia un messaggio di chiusura al client
    strcpy(client->buf, "Closing session of this socket. Goodbye!\n");
    send_response(client);
    // Chiude la socket del client
    close(client->sock);
    // Rimuove le informazioni del client dagli array globali
    for (int i = 0; i < MAX_THREADS; i++) {
        if (connection_ports[i] == client->port && connection_socks[i] == client->sock) {
            connection_ports[i] = 0;
            connection_socks[i] = 0;
            connection_time[i] = 0;
            break;
        }
    }
    // Libera la memoria allocata per il client
    free(client);
    // Aggiorna i contatori globali dei client
    client_number--;
    client_number_for_port--;
    thread_no--;
    // Stampa il numero di client attivi
    printf(KRED "[CLIENT OBSERVER]: " KGRN " Currently active clients: %d\n" RESET, client_number);
}

void *handle_request(void *p) {
    struct timeval tv1;
    struct client_info* client = (struct client_info*)p;
    char *tokens[BUFLEN];
    char **bufferToSend;
    char client_size[BUFLEN];

    const char *directory = "server_files"; // Directory corrente
    char *file_name = "files_list.txt"; // Nome del file da aggiornare

    while(1) {
        memset(client->buf, 0, sizeof(client->buf));  // Clear buffer
        fflush(stdout);

        // Receive message from client
        int n = recvfrom(client->sock, client->buf, BUFLEN, 0, (struct sockaddr *)&client->from, &client->fromlen);
        if (n < 0) {
            perror(KRED "[SERVER]: " KWHT "recvfrom" RESET);
        }
        puts("\n");

        // Start measuring time
        gettimeofday(&tv1, NULL); 

        // Logging client info
        printf(KRED "[CLIENT OBSERVER]: " KGRN " Client port: %d, Socket: %d\n" RESET, client->port, client->sock);
        printf(KRED "[COMMAND OBSERVER]: " KBLU "Command received: %s\n" RESET, client->buf);

        // Handle 'list' command
        if (strncmp(client->buf, "list", 5) == 0) {
            update_file_with_list(file_name, directory);
            bufferToSend = alloc_memory(BUFLEN);
            int Number_file = read_file_list(bufferToSend, file_name, client->buf, client);
            printf(KRED "[LOGGER]: " KCYN "          Number of files: %d\n" RESET, Number_file);

            // Send file count to client
            if (sendto(client->sock, &Number_file, sizeof(Number_file), 0, (struct sockaddr *)&client->from, sizeof(client->from)) == -1) {
                die("sendto()");
            }

            // Send file names to client
            for (int i = 0; bufferToSend[i] != NULL && i < Number_file; i++) {
                size_t length = strlen(bufferToSend[i]) + 1;
                if (sendto(client->sock, bufferToSend[i], length, 0, (struct sockaddr *)&client->from, sizeof(client->from)) == -1) {
                    die("sendto()");
                }
                rebind_socket(client);
            }
            free(bufferToSend);
            printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" KWHT, print_total_time(tv1));
            rebind_socket(client);
        }

        // Handle other commands
        if (strncmp(client->buf, "list", 5) != 0) {
            //tokens = alloc_memory(BUFLEN);
            tokenize_string(client->buf, " \n", tokens);

            // Handle 'get' command
            if (strcmp(tokens[0], "get") == 0) {
                if (tokens[1] == NULL) {
                    // No file specified
                    memset(client->buf, '\0', BUFLEN);
                    sprintf(client->buf, KRED "WARNING: The requested file is empty, please try another file!" RESET);
                    send_response(client);
                    printf(KRED "[FILE HANDLER]: " KCYN "File name incomplete for server server_files folder\n" RESET);
                    printf(KRED "[TIME LOGGER]: " KYEL "executed in %lf s\n" RESET, print_total_time(tv1));
                    continue;
                }
                // Check if the provided token[1] is a file and not a directory
                struct stat path_stat;
                if (stat(tokens[1], &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                    // It's a directory, send an error response
                    memset(client->buf, '\0', BUFLEN);
                    sprintf(client->buf, KRED "WARNING: The requested file is empty, please try another file!" RESET);
                    send_response(client);
                    continue;
                }

                char *filename = strdup(tokens[1]);
                encoder_handler(directory, directory, filename);

                char filename_encoded[BUFLEN];
                snprintf(filename_encoded, sizeof(filename_encoded), "%s.txt", tokens[1]);

                char filepath[525]; // Buffer per il percorso completo del file
                snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename_encoded); // Costruisce il percorso completo

                // Usa access() per verificare l'esistenza del file
                if (access(filepath, F_OK) == 0) {
                    // File found, send it
                    int fd = openFile(filepath);
                    if(fd == -1) {
                        handle_empty_file(client);
                        continue;
                    }
                    long size = getFileSize(filepath);

                    if (size == 0) {
                        // File is empty
                        closeFile(fd);
                        handle_empty_file(client);
                        continue;
                    }

                    sendFile(client->sock, fd, client->from);
                    closeFile(fd);
                    printf(KRED "[FILE HANDLER]: " KCYN "Sent file: %s\n" RESET, tokens[1]);
                    printf(KRED "[TIME LOGGER]: " KYEL "executed in %lf s\n" RESET, print_total_time(tv1));
                    memset(client->filepath, '\0', BUFLEN);
                    rebind_socket(client);
                    delete_file(directory,filename_encoded);
                    continue;
                } else {
                    // File not found
                    handle_file_not_found(client);
                    printf(KRED "[TIME LOGGER]: " KYEL "executed in %lf s\n" RESET, print_total_time(tv1));
                }
            } 
            // Handle 'put' command
            else if (strncmp(tokens[0], "put", 4) == 0) {
                if (tokens[1] == NULL) {
                    // No file specified
                    memset(client->buf, '\0', BUFLEN);
                    sprintf(client->buf, KRED "WARNING: You must enter a file.\n" RESET);
                    send_response(client);
                    printf(KRED "[FILE HANDLER]: " KCYN "File name incomplete for Client Upload folder\n" RESET);
                    printf(KRED "[TIME LOGGER]: " KYEL "executed in %lf s\n" RESET, print_total_time(tv1));
                    continue;
                }

                // Prepare to receive the file
                char filename_encoded[BUFLEN];
                snprintf(filename_encoded, sizeof(filename_encoded), "%s.txt", tokens[1]);

                client->filepath = obtain_path(client->filepath, filename_encoded, tokens[0]);
                int fds = openFile(client->filepath);
                if (fds == -1) {
                    perror(KRED "[SERVER]: " KWHT "File already downloaded or in process...\n" RESET);
                    handle_empty_file(client);
                    continue;
                }

                // Receive file size
                memset(client_size, '\0', BUFLEN);
                recv(client->sock, client_size, sizeof(char) * 11, 0);
                if (strcmp(client_size, "Error_path") == 0) {
                    remove(client->filepath);
                    handle_error(client);
                    printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                    continue;   
                }

                // receive the file
                long size = strtol(client_size, NULL, 10);
                rcvFile(client->sock, fds, size, client->from);
                closeFile(fds);
                double total_time = print_total_time(tv1);
                decoder_handler(directory, directory, filename_encoded); // Decoder sul file ricevuto
                delete_file(directory, filename_encoded); 

                printf(KRED "[FILE HANDLER]: " KCYN "Received file: %s\n" RESET, tokens[1]);
                printf(KRED "[TIME LOGGER]: " KYEL "executed in %lf s\n" RESET, total_time);

                memset(client->filepath, '\0', BUFLEN);
                rebind_socket(client);
            }
            // Handle 'delete' command
            else if (strncmp(tokens[0], "delete", 7) == 0) {
                if (tokens[1] == NULL) {
                    // No file specified
                    memset(client->buf, '\0', BUFLEN);
                    sprintf(client->buf, KRED "WARNING: You must enter a file.\n" RESET);
                    send_response(client);
                    printf(KRED "[FILE HANDLER]: " KCYN "File name incomplete for server server_files folder\n" RESET);
                    printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                    continue;
                }
                struct stat path_stat;
                char filepath[525]; // Buffer per il percorso completo del file
                snprintf(filepath, sizeof(filepath), "%s/%s", directory, tokens[1]); // Costruisce il percorso completo

                // Usa stat() per verificare se il file esiste
                if (stat(filepath, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) {
                    // File non trovato o non è un file regolare
                    memset(client->buf, '\0', BUFLEN);
                    handle_file_not_found(client);
                    printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                    continue;
                }
                delete_file(directory, tokens[1]);
                printf(KRED "[FILE HANDLER]: " KCYN "Deleted file: %s\n" RESET, tokens[1]);
                printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                memset(client->filepath, '\0', BUFLEN);
                rebind_socket(client);
                continue;
            }
            // Handle 'quit' command
            else if (strncmp(tokens[0], "quit", 5) == 0) {
                handle_exit(client);
                printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                pthread_exit(NULL);
                break;
            }
            // Handle invalid command
            else {
                memset(client->buf, 0, strlen(client->buf));
                strcpy(client->buf, "Warning: invalid command!");
                send_response(client);
                rebind_socket(client);
                printf(KRED "[TIME LOGGER]: " KYEL "     executed in %lf s\n" RESET, print_total_time(tv1));
                continue;
            }
        }
    }
}

char* port_validation(char* s) {
    // Controllo di validità per il puntatore
    if (s == NULL) {
        fprintf(stderr, KRED "[SERVER]: " KWHT "Error: NULL input string\n" RESET);
        exit(EXIT_FAILURE);
    }
    // Conversione della stringa in numero intero
    errno = 0;
    int base_port = strtol(s, NULL, 10);
    if (errno != 0) {
        fprintf(stderr, KRED "[SERVER]: " KWHT "Error in strtol\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Trova una porta disponibile
    int port = base_port;
    struct sockaddr_in temp_addr;
    int temp_sock;
    int max_retries = 1000; // Numero massimo di tentativi

    while (max_retries > 0) {
        temp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (temp_sock < 0) {
            fprintf(stderr, KRED "[SERVER]: " KWHT "Error creating socket\n" RESET);
            exit(EXIT_FAILURE);
        }

        memset(&temp_addr, 0, sizeof(temp_addr));
        temp_addr.sin_family = AF_INET;
        temp_addr.sin_addr.s_addr = INADDR_ANY;
        temp_addr.sin_port = htons(port);

        if (bind(temp_sock, (struct sockaddr *)&temp_addr, sizeof(temp_addr)) == 0) {
            close(temp_sock);
            break; // Porta disponibile trovata
        }

        close(temp_sock);
        port++;
        max_retries--;
        if (port > 65535) { // Se la porta supera il limite massimo, ricomincia da una porta base
            port = base_port;
        }
    }

    if (max_retries == 0) {
        fprintf(stderr, KRED "[SERVER]: " KWHT "Error: Unable to find an available port after maximum retries\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Aggiorna la stringa con la porta valida finale
    sprintf(s, "%d", port);
    // Restituisce la stringa contenente la porta valida
    return s;
}

void kill_process_on_port(const char *port) {
    char command[BUFLEN];
    char buffer[BUFLEN];
    char process_name[BUFLEN];
    int pid = -1;
    FILE *fp;

    // Costruisce il comando ss per trovare i processi sulla porta specificata
    snprintf(command, sizeof(command), 
             "ss -lnp 'sport = :%s' | awk -F'[:,()]' '/users:/ {print $6, $7}'", port);

    // Esegue il comando e apre un pipe per leggere l'output
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, KRED "[SERVER]: " KWHT "Error executing ss command\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Legge tutti i processi trovati sulla porta specificata
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char pid_str[BUFLEN];
        sscanf(buffer, "%s pid=%s", process_name, pid_str); // Estrae il nome del processo e il PID come stringa
        pid = atoi(pid_str); // Converte il PID in un intero

        // Controlla se il processo si chiama "my_server"
        if (pid > 0 && strcmp(process_name, "\"my_server\"") == 0) {
            printf(KRED "[SERVER]: " KGRN "Found process '%s' with PID %d on port %s. Terminating...\n" RESET, process_name, pid, port);

            // Termina il processo con kill -9
            if (kill(pid, SIGKILL) == 0) {
            printf(KRED "[SERVER]: " KGRN "Process '%s' with PID %d terminated successfully.\n" RESET, process_name, pid);
            } else {
            fprintf(stderr, KRED "[SERVER]: " KWHT "Error terminating process '%s' with PID %d\n" RESET, process_name, pid);
            }
        } else if (pid > 0) {
            printf(KRED "[SERVER]: " KGRN "The process '%s' on port %s is not 'my_server'. No action taken.\n" RESET, process_name, port);
        }
    }
    pclose(fp);

    // Se nessun processo è stato trovato
    if (pid == -1) {
        printf(KRED "[SERVER]: " KGRN "No process active found on port %s.\n" RESET, port);
    }
}

void sig_handler(int signum){
    printf("Signal received: %d\n", signum);
    signal_flag++;
}

void *handle_client_requests() {
    struct timeval tv1;
    char buf[BUFLEN];
    int rc = 0;
    int sock_main_thread; // socket principale
    struct sockaddr_in server;

    // Set up signal handling
    struct sigaction act;
    sigset_t set;
    sigfillset(&set);
    act.sa_handler = sig_handler;
    act.sa_mask = set;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("Error function sigaction");
        exit(EXIT_FAILURE);
    } else if (sigaction(SIGQUIT, &act, NULL) == -1) {
        perror("Error function sigaction");
        exit(EXIT_FAILURE);
    }

    kill_process_on_port(server_port);

    while (1) {
        // Step 1: Create and bind socket to specified port
        sock_main_thread = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_main_thread < 0) {
            perror(KRED "[SERVER]: " KWHT "Error creating socket" RESET);
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        if (setsockopt(sock_main_thread, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error setting SO_REUSEADDR" RESET);
            close(sock_main_thread);
            exit(EXIT_FAILURE);
        }

        if (setsockopt(sock_main_thread, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error setting SO_REUSEPORT" RESET);
            close(sock_main_thread);
            exit(EXIT_FAILURE);
        }

        // Imposta il TOS (ad esempio, DSCP per priorità alta: 0x2E)
        int tos = 0x2E; // Expedited Forwarding (EF)
        if (setsockopt(sock_main_thread, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error setting TOS" RESET);
        }

        // Clear and set up server struct
        server = (struct sockaddr_in){0};
        server.sin_family = AF_INET;

        // Configura l'indirizzo IP specifico
        if (inet_aton(ip_address, &server.sin_addr) == 0) { // Sostituisci con il tuo IP desiderato
            perror(KRED "[SERVER]: " KWHT "Error setting specific IP address" RESET);
            close(sock_main_thread);
            exit(EXIT_FAILURE);
        }

        // Esempio: Convertire e gestire errori
        char *endptr;
        long serv_port = strtol(server_port, &endptr, 10); // Base 10

        if (*endptr != '\0') {
            fprintf(stderr, KRED "[SERVER]: " KWHT "Error: Port not valid '%s'\n" RESET, server_port);
            exit(EXIT_FAILURE);
        }

        // Imposta la porta nel socket
        server.sin_port = htons((uint16_t)serv_port);

        // Effettua il bind del socket all'indirizzo IP e alla porta specificata
        if (bind(sock_main_thread, (struct sockaddr *)&server, sizeof(server)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error binding socket" RESET);
            close(sock_main_thread);
            sleep(5);
            continue;
        }

        // Step 2: Allocate memory for client_info struct and set up client address
        struct client_info *client = malloc(sizeof(struct client_info));
        if (client == NULL) {
            perror(KRED "[SERVER]: " KWHT "Error allocating memory for client_info" RESET);
            close(sock_main_thread);
            exit(EXIT_FAILURE);
        }
        client->fromlen = sizeof(struct sockaddr_in);

        // Step 3: Receive data from the client
        struct timeval timeout;
        timeout.tv_sec = 5; // Timeout di 5 secondi
        timeout.tv_usec = 0;

        // Imposta il timeout sulla socket
        if (setsockopt(sock_main_thread, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error setting socket timeout" RESET);
            free(client);
            close(sock_main_thread);
            continue;
        }

        int n = recvfrom(sock_main_thread, buf, BUFLEN, 0, (struct sockaddr *)&client->from, &client->fromlen);
        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (process_on == 0) {
                    free(client);
                    break;
                }
            } else {
                perror(KRED "[SERVER]: " KWHT "recvfrom error" RESET);
            }
            free(client);
            close(sock_main_thread);
            continue;
        }
        buf[n] = '\0';  // Null-terminate received data for safety
        gettimeofday(&tv1, NULL);

        // Step 4: Process received data, allocate memory for port buffer, and send response
        char *buffer_port = port_validation(buf);
        printf("\n\n");
        printf(KRED "[SERVER]: " KGRN "Multiplexing new connection on port: %s\n" RESET, buffer_port);
        if (buffer_port == NULL) {
            perror(KRED "[SERVER]: " KWHT "port error" RESET);
            free(client);
            close(sock_main_thread);
            continue;
        }

        int port_final = atoi(buffer_port);
        int retry_count = 0;
        while (retry_count < 10) {
            if (sndMsg_sync(sock_main_thread, buffer_port, &client->from, client->fromlen)) {
            break; // Sync successful, exit retry loop
            }
            retry_count++;
            perror(KRED "[MSGSYNC]: " KGRN "Error of initial SYNC packet loss, retrying... " RESET);
        }

        if (retry_count == 10) {
            perror(KRED "[MSGSYNC]: " KGRN "Error of initial SYNC packet loss after 10 retries" RESET);
            free(client);
            close(sock_main_thread);
            continue;
        }

        // Step 5: Set up client socket and port information
        client->port = port_final;
        client->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (client->sock < 0) {
            perror(KRED "[SERVER]: " KWHT "Error creating client socket" RESET);
            free(client);
            close(sock_main_thread);
            continue;
        }

        // Imposta il TOS (ad esempio, DSCP per priorità alta: 0x2E)
        if (setsockopt(client->sock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error setting TOS" RESET);
        }

        client->server_addr = (struct sockaddr_in){0};  // Clear and set up client->server_addr struct
        client->server_addr.sin_family = AF_INET;

        // Configura l'indirizzo IP specifico
        if (inet_aton(ip_address, &server.sin_addr) == 0) { // Sostituisci con il tuo IP desiderato
            perror(KRED "[SERVER]: " KWHT "Error setting specific IP address" RESET);
            close(sock_main_thread);
            exit(EXIT_FAILURE);
        }

        client->server_addr.sin_port = htons(client->port);

        client->filepath = malloc(BUFLEN);
        if (client->filepath == NULL) {
            perror(KRED "[SERVER]: " KWHT "Error allocating memory for filepath" RESET);
            close(client->sock);
            free(client);
            continue;
        }

        if (bind(client->sock, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)) < 0) {
            perror(KRED "[SERVER]: " KWHT "Error binding client socket" RESET);
            free(client->filepath);
            close(client->sock);
            free(client);
            exit(EXIT_FAILURE);
        }

        client_number++;
        client_number_for_port++;
        
        printf(KRED "[CLIENT OBSERVER]: " KGRN "New Client-Server connection!\n" RESET);
        printf(KRED "[CLIENT OBSERVER]: " KGRN "Client port: %d, Socket: %d\n" RESET, client->port, client->sock);
        printf(KRED "[CLIENT OBSERVER]: " KGRN "Currently active clients: %d\n" RESET, client_number);

        // Step 6: Create a thread to handle the client request
        rc = pthread_create(&threads[thread_no], NULL, handle_request, (void *)client);
        if (rc) {
            fprintf(stderr, "A request could not be processed\n");
            free(client->filepath);
            close(client->sock);
            free(client);
        } else {
            thread_no++;

            connection_ports[connection_index] = client->port;
            connection_socks[connection_index] = client->sock;
            connection_time[connection_index] = time(NULL); // Save current date and time
            connection_index++;
        }

        // Step 7: Calculate total connection and setup time
        printf(KRED "[TIME LOGGER]: " KYEL "Connection time: %lf s | Server Thread Index: %d\n" RESET, print_total_time(tv1), thread_no);

        // Step 8: Clean up
        close(sock_main_thread);
    }
    return NULL;
}

void *monitor_user_input() {
    char *input = NULL; // Buffer dinamico
    size_t len = 0;     // Dimensione del buffer
    ssize_t read;       // Numero di caratteri letti

    while (1) {
        // Usa getline per leggere l'input dinamicamente
        read = getline(&input, &len, stdin);
        if (read != -1) {
            // Rimuove il newline finale, se presente
            input[strcspn(input, "\n")] = '\0';
            // Verifica il comando "/shutdown"
            if (strcmp(input, "/shutdown") == 0) {
                printf(KRED "[SERVER]: " RESET "Server shutting down...\n");
                fflush(stdout);
                break;
            }
            // Comando /delete file
            if (strncmp(input, "/delete", 7) == 0) {
                const char *directory = "server_files";  // Directory corrente
                char *file_name = input + 8;  // Poiché "/delete " ha 8 caratteri
                if (strlen(input) > 8) {
                    delete_file(directory, file_name);  // Passa il nome del file alla funzione delete_file
                } else {
                    printf(KRED "[SERVER]: " RESET "Invalid command: %s\n", input);
                    fflush(stdout);
                }
            }
            // Comando /list
            if (strcmp(input, "/list") == 0) {
                const char *directory = "server_files";  // Directory corrente
                char *list = list_files(directory);
                printf(
                    "\n╭───────────────────────────────────────────────────────╮\n"
                    "│              📂 LIST OF FILES 📂                      │\n"
                    "├───────────────────────────────────────────────────────┤\n"
                );
                char *file = strtok(list, "\n");
                while (file != NULL) {
                    printf("│ %-53s │\n", file); // Aggiunge la chiusura laterale destra
                    file = strtok(NULL, "\n");
                }
                printf("╰───────────────────────────────────────────────────────╯\n");
                free(list); // Libera la memoria allocata per la lista
            }
            // Comando /connections
            if (strcmp(input, "/connections") == 0) {
                printf(
                    "\n╭───────────────────────────────────────────────────────╮\n"
                    "│              🌐 ACTIVE CONNECTIONS 🌐                 │\n"
                    "├───────────────────────────────────────────────────────┤\n"
                );

                for (int i = 0; i < connection_index; i++) {
                    if (connection_ports[i] != 0 && connection_socks[i] != 0) {
                        char time_buffer[26];
                        struct tm *tm_info = localtime(&connection_time[i]);
                        strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
                        printf("│ Connection: Port %d, Socket %d, %s  │\n", connection_ports[i], connection_socks[i], time_buffer);
                    }
                }
                printf("╰───────────────────────────────────────────────────────╯\n");
            }

            // Comando /disconnect <port>
            if (strncmp(input, "/disconnect", 11) == 0) {
                int port_to_disconnect = atoi(input + 12); // Ottieni la porta dal comando
                int found = 0;

                for (int i = 0; i < connection_index; i++) {
                    if (connection_ports[i] == port_to_disconnect) {
                        close(connection_socks[i]);
                        printf(KRED "[SERVER]: " KWHT "Disconnected port %d with socket %d\n" RESET, port_to_disconnect,connection_socks[i]);
                        connection_ports[i] = 0;
                        connection_socks[i] = 0;
                        connection_time[i] = 0;
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    printf(KRED "[SERVER]: " KWHT "No active connection found on port %d\n" RESET, port_to_disconnect);
                }
            }
            // helper
            if (strcmp(input, "/help") == 0) {
                printf(
                    "\n╭───────────────────────────────────────────────────────╮\n"
                    "│              🌟 AVAILABLE COMMANDS 🌟                 │\n"
                    "├───────────────────────────────────────────────────────┤\n"
                    "│ 1) " KGRN "/shutdown" RESET "                                          │\n"
                    "│    ➤ Close the UDP socket of the server               │\n"
                    "│ 2) " KGRN "/delete <FileName>" RESET "                                 │\n"
                    "│    ➤ Delete a file from folder server_files           │\n"
                    "│ 3) " KGRN "/list" RESET "                                              │\n"
                    "│    ➤ List all files in the server_files directory     │\n"
                    "│ 4) " KGRN "/connections" RESET "                                       │\n"
                    "│    ➤ List all active connections                      │\n"
                    "│ 5) " KGRN "/disconnect <port>" RESET "                                 │\n"
                    "│    ➤ Disconnect a client on the specified port        │\n"
                    "│" KRED"    Warning: Use this command only to free" RESET "             │\n"
                    "│" KRED"    a port if there are issues with the client" RESET "         │\n"
                    "╰───────────────────────────────────────────────────────╯\n"
                );
            }
            // Comando non riconosciuto
            if (strcmp(input, "/shutdown") != 0 && strncmp(input, "/delete", 7) != 0 && strcmp(input, "/list") != 0 && strcmp(input, "/connections") != 0 && strncmp(input, "/disconnect", 11) != 0 && strcmp(input, "/help") != 0) {
                printf(KRED "[SERVER]: " RESET "Invalid command: %s\n", input);
                fflush(stdout);
            }
        }
    }
    // Libera la memoria allocata
    free(input);
    process_on = 0;
    return NULL;
}

// Funzione per verificare se l'IP è raggiungibile usando il comando 'ping'
int check_ip_availability(const char *ip) {
    char command[BUFLEN];

    // Costruisci il comando ping (il flag -c 1 invia 1 pacchetto ICMP)
    snprintf(command, sizeof(command), "ping -c 1 -w 1 %s > /dev/null 2>&1", ip);
    
    // Esegui il comando e controlla se è stato eseguito con successo
    int result = system(command);
    return (result == 0);  // Se il comando ha successo, il risultato sarà 0
}

int main(void) {
    char *endptr;
    long serv_port = 0;

    RST:
    printf(KRED "[SERVER]: " KWHT "Entering server configuration...\n" RESET);
    REBIND_IP:
    // Richiedi all'utente di inserire un indirizzo IP
    printf(KRED "[SERVER]: " KWHT "Please enter the server IP address: " RESET);
    if (fgets(ip_address, sizeof(ip_address), stdin) == NULL) {
        perror(KRED "[SERVER]: " KWHT "Error while inserting the IP" RESET);
        goto REBIND_IP;
    }

    // Rimuove il newline finale se presente
    ip_address[strcspn(ip_address, "\n")] = 0;

    // Verifica se l'IP è raggiungibile
    if (!check_ip_availability(ip_address)) {
        printf(KRED "[SERVER]: " KWHT "The IP address %s is not reachable.\n" RESET, ip_address);
        goto REBIND_IP;
    } else {
        printf(KRED "[SERVER]: " KWHT "The IP address %s is reachable.\n" RESET, ip_address);
    }
    
    REBIND_PORT:
    // Richiedi all'utente di inserire una porta
    printf(KRED "[SERVER]: " KWHT "Please enter the server Default Port: " RESET);
    if (fgets(server_port, sizeof(server_port), stdin) == NULL) {
        perror(KRED "[SERVER]: " KWHT "Error while inserting the Default port" RESET);
        goto REBIND_IP;
    }

    // Rimuove il newline finale se presente
    server_port[strcspn(server_port, "\n")] = 0;

    // Converte la porta in un numero
    serv_port = strtol(server_port, &endptr, 10); // Base 10 per numeri decimali

    // Controllo della validità della conversione e dei limiti
    if (*endptr != '\0') {
        fprintf(stderr, KRED "[SERVER]: " KWHT "Error: the port needs to be a valid number.\n" RESET);
        goto REBIND_PORT;
    }

    if(serv_port < 7000 || serv_port > 65535) {
        printf(KRED "[SERVER]: " KWHT "The default port %s is not reachable.\n" RESET, server_port);
        goto REBIND_PORT;
    } else {
        printf(KRED "[SERVER]: " KWHT "The default port %s is reachable.\n" RESET, server_port);
    }

    // Rimuove il newline finale se presente
    server_port[strcspn(server_port, "\n")] = 0;

    // Crea un thread per monitorare l'input dell'utente
    if (pthread_create(&input_thread, NULL, monitor_user_input, NULL) != 0) {
        perror(KRED "[SERVER]: " KWHT "Error thread creation" RESET);
        goto RST;
    }

    printf("\n\n\n");
    printf(KRED "[OPTION]: " KWHT "Loss probability (P): %.2f%%\n",P*100);
    printf(KRED "[OPTION]: " KWHT "Sliding window size (N): %d\n",N);
    printf(KRED "[OPTION]: " KWHT "Timeout value (T fixed): %d s\n",T/1000000);
    printf(KRED "[OPTION]: " KWHT "Fixed timeout mode: %s\n", FIXEDTIMEOUT ? "ON" : "OFF");
    printf(KRED "[OPTION]: " KWHT "Maximum number of retransmissions: %d times\n", MAXIMUM_ATTEMPT);
    printf(KRED "[OPTION]: " KWHT "Performance mode: %s\n",PERFORMANCE ? "ON" : "OFF");
    printf(KRED "[OPTION]: " KWHT "Max threads: %d", MAX_THREADS);


    printf("\n\n\n");
    printf(KGRN "───────────────────────────────────────────────────────────────────────────────\n" RESET);
    printf(KRED "[SERVER]: " KWHT "Server started successfully!\n" RESET);
    printf(KRED "[SERVER]: " KWHT "Waiting for any connection on IP: %s and default port: %s\n" RESET, ip_address,server_port);
    printf(KRED "[SERVER]: " KWHT "/help for more info...\n" RESET);
    printf(KGRN "───────────────────────────────────────────────────────────────────────────────\n" RESET);

    // Gestione delle richieste dei client
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int combination = (tm.tm_mon + 1) * 100 + (tm.tm_year + 1900);
    initialize_base64_alphabet_encode(combination);
    initialize_base64_alphabet_decode(combination);
    handle_client_requests();

    //shutdown procedure
    free(base64_chars);
    free(BASE64_CHARS);
    
    // Close all threads in threads array
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i]) {
            pthread_cancel(threads[i]);
            pthread_join(threads[i], NULL);
        }
    }
    pthread_cancel(input_thread);
    pthread_join(input_thread, NULL);
    printf(KRED "[SERVER]: " RESET "Server shutdown completed!\n");
    exit(EXIT_SUCCESS);

    return 0;
}