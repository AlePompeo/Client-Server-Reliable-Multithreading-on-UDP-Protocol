// Progetto IIW
// Versione: 11.0
// Autore: Alessio Pompeo
// Autore: Davide Edoardo Stoica
// Autore: Consuelo Caporaso
// Anno: 2025

#include "macros.h"
#include "clientfileoperation.h"
#include "ClientSender.h"
#include "ClientReceiver.h"
#include "clientsyncmsg.h"
#include "decode64.h"
#include "encode64.h"
#include "reliableUDP.h"

int global_var = 0;
pthread_mutex_t next;
int Number_file; // variabile in cui memorizzo il numero di file presenti nel File Lista.txt -> cosi al variare del numero di file nella lista varia anche questa variabile globale
int signal_flag = 0; // Variabile globale per il segnale
char ip_address[BUFLEN]; // Variabile globale per l'indirizzo IP
char server_port[BUFLEN]; // Variabile globale per la porta predefinita

struct thread_list {
    char *buffer;
    pthread_t tid;
    pthread_mutex_t mutex_thread;
};

void die(char *s) {
    perror(s);
    exit(1);
}

void* thread_function(void*p) {
    struct thread_list *tinfo = (struct thread_list *) p;
    pthread_mutex_lock(&tinfo->mutex_thread);
    printf("%s\n",tinfo->buffer);
    pthread_mutex_unlock(&tinfo->mutex_thread);
    pthread_exit(NULL);
}

char ** tokenize_string(char * buffer,char * delimiter) {
    int i = 0;
    char **token_vector = malloc(BUFLEN * sizeof(char*));
    token_vector[i] = strtok(buffer,delimiter);
    while(token_vector[i]!= NULL) {
        i++;
        token_vector[i] =strtok(NULL,delimiter);
    }
    return token_vector;
}

void handler(int signum) {
    printf("Signal received: %d\n", signum);
    signal_flag++;
}

char* list_files(const char *dir_name) {
    // Alloca un buffer iniziale per memorizzare i nomi dei file
    char *buffer = malloc(1024);
    if (buffer == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0'; // Inizializza il buffer come stringa vuota

    // Apre la directory specificata
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("Error opening the directory");
        free(buffer); // Libera la memoria allocata in caso di errore
        exit(EXIT_FAILURE);
    }

    struct dirent *entry; // Struttura per leggere le voci della directory
    size_t buffer_len = 0; // Lunghezza corrente del contenuto del buffer

    // Itera su ogni voce della directory
    while ((entry = readdir(dir)) != NULL) {
        // Ignora le voci "." e ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            size_t entry_len = strlen(entry->d_name) + 1; // Calcola la lunghezza del nome del file + '\n'

            // Controlla se il buffer ha spazio sufficiente, altrimenti rialloca
            if (buffer_len + entry_len + 1 >= 1024) {
                char *new_buffer = realloc(buffer, buffer_len + entry_len + 1);
                if (new_buffer == NULL) {
                    perror("Error in realloc");
                    free(buffer); // Libera la memoria allocata in caso di errore
                    closedir(dir); // Chiude la directory
                    exit(EXIT_FAILURE);
                }
                buffer = new_buffer; // Aggiorna il puntatore al nuovo buffer
            }

            // Aggiunge il nome del file al buffer
            strcat(buffer, entry->d_name);
            strcat(buffer, "\n"); // Aggiunge un newline dopo il nome del file
            buffer_len += entry_len; // Aggiorna la lunghezza del buffer
        }
    }

    closedir(dir); // Chiude la directory
    return buffer; // Ritorna il buffer contenente i nomi dei file
}

void open_with_vim(const char *file_path) {
    char command[MAX_PATH + 50]; // Include spazio extra per opzioni
    // Usa 'vim -Z' per avviare Vim in modalità sandbox
    snprintf(command, sizeof(command), "vim -Z -c 'set nomodeline | set secure' %s", file_path);

    int ret = system(command);
    if (ret == -1) {
        perror("Error opening Vim");
        exit(EXIT_FAILURE);
    }
}

void modify_file_interactive() {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting the current directory");
        exit(EXIT_FAILURE);
    }
    // Costruisce percorsi relativi basati sulla directory corrente
    char download_dir[MAX_PATH], upload_dir[MAX_PATH];
    if (snprintf(download_dir, MAX_PATH, "%s/Download", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for download_dir\n");
        exit(EXIT_FAILURE);
    }
    if (snprintf(upload_dir, MAX_PATH, "%s/Upload", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for upload_dir\n");
        exit(EXIT_FAILURE);
    }

    CHOICE:
    printf(KRED"[CLIENT OBSERVER]"RESET ": Select a directory by digiting a number: [1.Download] [2.Upload]\n");
    printf(KRED"[CLIENT OBSERVER]"RESET ": Number Selected: ");

    int choice;
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": Invalid choice. Please try again.\n");
        goto CHOICE;
    }
    getchar(); // Consuma il carattere newline rimasto dopo scanf
    const char *selected_dir = (choice == 1) ? download_dir : upload_dir;
    printf(KRED"[CLIENT OBSERVER]"RESET ": Directory selected: %s\n", selected_dir);
    // Verifica che la directory selezionata esista
    struct stat st;
    if (stat(selected_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Errore: The directory %s does not exist.\n", selected_dir);
        exit(EXIT_FAILURE);
    }
    char *files_list = list_files(selected_dir);
    // Controlla se la cartella è vuota senza usare list_files
    DIR *dir = opendir(selected_dir);
    if (dir == NULL) {
        perror("Error opening the directory");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry;
    int is_empty = 1; // Assume che la directory sia vuota
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            is_empty = 0; // La directory non è vuota
            break;
        }
    }
    closedir(dir);
    if (is_empty) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": The directory is empty. Returning to 'File editing complete'.\n");
        free(files_list);
        return;
    }
    
    char *line = strtok(files_list, "\n");
    while (line != NULL) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": %s\n", line);
        line = strtok(NULL, "\n");
    }
    free(files_list);
    printf(KRED"[CLIENT OBSERVER]"RESET ": Insert the file name to modify or to be created if it does not exist: ");
    char file_name[MAX_PATH];
    if (fgets(file_name, MAX_PATH, stdin) == NULL) {
        fprintf(stderr, "Error while reading the file.\n");
        exit(EXIT_FAILURE);
    }
    file_name[strcspn(file_name, "\n")] = '\0';
    char file_path[MAX_PATH];
    if (snprintf(file_path, MAX_PATH, "%s/%s", selected_dir, file_name) >= MAX_PATH) {
        fprintf(stderr, "Error: File path too long.\n");
        exit(EXIT_FAILURE);
    }
    // Controlla se il file esiste, altrimenti lo crea
    if (access(file_path, F_OK) != 0) {
        FILE *file = fopen(file_path, "w");
        if (!file) {
            perror("Error during file creation");
            exit(EXIT_FAILURE);
        }
        fclose(file);
        printf(KRED"[CLIENT OBSERVER]"RESET ": File '%s' created.\n", file_path);
    }
    open_with_vim(file_path);
    printf(KRED"[CLIENT OBSERVER]"RESET ": File editing complete.\n");
    return;
}

int compressFile(const char *filePath) {
    char outputPath[MAX_PATH];
    snprintf(outputPath, sizeof(outputPath), "%s.gz", filePath);
    FILE *source = fopen(filePath, "rb");
    if (!source) {
        perror("fopen");
        return -1;
    }
    gzFile dest = gzopen(outputPath, "wb");
    if (!dest) {
        perror("gzopen");
        fclose(source);
        return -1;
    }
    char buffer[CHUNK];
    int bytesRead;
    while ((bytesRead = fread(buffer, 1, CHUNK, source)) > 0) {
        if (gzwrite(dest, buffer, bytesRead) != bytesRead) {
            perror("gzwrite");
            gzclose(dest);
            fclose(source);
            return -1;
        }
    }
    gzclose(dest);
    fclose(source);
    return 0;
}

int decompressFile(const char *filePath) {
    char outputPath[MAX_PATH];
    snprintf(outputPath, sizeof(outputPath), "%s", filePath);
    outputPath[strlen(outputPath) - 3] = '\0'; // Remove .gz extension
    gzFile source = gzopen(filePath, "rb");
    if (!source) {
        perror("gzopen");
        return -1;
    }
    FILE *dest = fopen(outputPath, "wb");
    if (!dest) {
        perror("fopen");
        gzclose(source);
        return -1;
    }
    char buffer[CHUNK];
    int bytesRead;
    while ((bytesRead = gzread(source, buffer, CHUNK)) > 0) {
        if (fwrite(buffer, 1, (size_t)bytesRead, dest) != (size_t)bytesRead) {
            perror("fwrite");
            gzclose(source);
            fclose(dest);
            return -1;
        }
    }
    gzclose(source);
    fclose(dest);
    return 0;
}

void compress_interactive_file() {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting the current directory");
        exit(EXIT_FAILURE);
    }
    char download_dir[MAX_PATH], upload_dir[MAX_PATH];
    if (snprintf(download_dir, MAX_PATH, "%s/Download", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for download_dir\n");
        exit(EXIT_FAILURE);
    }
    if (snprintf(upload_dir, MAX_PATH, "%s/Upload", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for upload_dir\n");
        exit(EXIT_FAILURE);
    }
    CHOICE:
    printf(KRED"[CLIENT OBSERVER]"RESET ": Select a directory by typing a number: [1.Download] [2.Upload]\n");
    printf(KRED"[CLIENT OBSERVER]"RESET ": Number Selected: ");
    int choice;
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": Invalid choice. Please try again.\n");
        goto CHOICE;
    }
    getchar(); // Consuma il carattere newline rimasto dopo scanf
    const char *selected_dir = (choice == 1) ? download_dir : upload_dir;
    printf(KRED"[CLIENT OBSERVER]"RESET ": Directory selected: %s\n", selected_dir);
    struct stat st;
    if (stat(selected_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: The directory %s does not exist.\n", selected_dir);
        exit(EXIT_FAILURE);
    }
    char *files_list = list_files(selected_dir);
    // Controlla se la cartella è vuota
    if (files_list[0] == '\0') {
        printf(KRED"[CLIENT OBSERVER]"RESET ": The directory is empty. Returning to 'File editing complete'.\n");
        free(files_list);
        goto FINISH;
    }
    if (files_list) {
        char *line = strtok(files_list, "\n");
        while (line != NULL) {
            printf(KRED"[CLIENT OBSERVER]"RESET ": %s\n", line);
            line = strtok(NULL, "\n");
        }
        free(files_list);
    }
    printf(KRED"[CLIENT OBSERVER]"RESET ": Insert the file name to be compressed: ");
    char file_name[MAX_PATH];
    if (fgets(file_name, MAX_PATH, stdin) == NULL) {
        fprintf(stderr, "Error while reading the file.\n");
        exit(EXIT_FAILURE);
    }
    file_name[strcspn(file_name, "\n")] = '\0';
    char file_path[MAX_PATH];
    if (snprintf(file_path, MAX_PATH, "%s/%s", selected_dir, file_name) >= MAX_PATH) {
        fprintf(stderr, "Error: File path too long.\n");
        exit(EXIT_FAILURE);
    }
    if (access(file_path, F_OK) != 0) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": File '%s' does not exist.\n", file_path);
    }
    if (compressFile(file_path) != 0) {
        FINISH:
        fprintf(stderr, "Failed to compress file\n");
    }else {
        printf(KRED"[CLIENT OBSERVER]"RESET ": File '%s' compressed successfully.\n", file_path);
        // Remove the decompressed file
        if (remove(file_path) != 0) {
            perror("Error deleting the decompressed file");
        } else {
            printf(KRED"[CLIENT OBSERVER]"RESET ": Decompressed file '%s' deleted successfully.\n", file_path);
        }   
    }    
}

void decompress_interactive_file() {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting the current directory");
        exit(EXIT_FAILURE);
    }
    char download_dir[MAX_PATH], upload_dir[MAX_PATH];
    if (snprintf(download_dir, MAX_PATH, "%s/Download", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for download_dir\n");
        exit(EXIT_FAILURE);
    }
    if (snprintf(upload_dir, MAX_PATH, "%s/Upload", cwd) >= MAX_PATH) {
        fprintf(stderr, "Error: Path too long for upload_dir\n");
        exit(EXIT_FAILURE);
    }
    CHOICE:
    printf(KRED"[CLIENT OBSERVER]"RESET ": Select a directory by typing a number: [1.Download] [2.Upload]\n");
    printf(KRED"[CLIENT OBSERVER]"RESET ": Number Selected: ");
    int choice;
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": Invalid choice. Please try again.\n");
        goto CHOICE;
    }
    getchar(); // Consuma il carattere newline rimasto dopo scanf
    const char *selected_dir = (choice == 1) ? download_dir : upload_dir;
    printf(KRED"[CLIENT OBSERVER]"RESET ": Directory selected: %s\n", selected_dir);
    struct stat st;
    if (stat(selected_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: The directory %s does not exist.\n", selected_dir);
        exit(EXIT_FAILURE);
    }
    char *files_list = list_files(selected_dir);
    if (files_list[0] == '\0') {
        printf(KRED"[CLIENT OBSERVER]"RESET ": The directory is empty. Returning to 'File editing complete'.\n");
        free(files_list);
        goto FINISH;
    }
    if (files_list) {
        char *line = strtok(files_list, "\n");
        while (line != NULL) {
            printf(KRED"[CLIENT OBSERVER]"RESET ": %s\n", line);
            line = strtok(NULL, "\n");
        }
        free(files_list);
    }
    printf(KRED"[CLIENT OBSERVER]"RESET ": Insert the file name to be decompressed: ");
    char file_name[MAX_PATH];
    if (fgets(file_name, MAX_PATH, stdin) == NULL) {
        fprintf(stderr, "Error while reading the file.\n");
        exit(EXIT_FAILURE);
    }
    file_name[strcspn(file_name, "\n")] = '\0';
    char file_path[MAX_PATH];
    if (snprintf(file_path, MAX_PATH, "%s/%s", selected_dir, file_name) >= MAX_PATH) {
        fprintf(stderr, "Error: File path too long.\n");
        exit(EXIT_FAILURE);
    }
    if (access(file_path, F_OK) != 0) {
        printf(KRED"[CLIENT OBSERVER]"RESET ": File '%s' does not exist.\n", file_path);
    }
    if (decompressFile(file_path) != 0) {
        FINISH:
        fprintf(stderr, "Failed to decompress file\n");
    } else {
        printf(KRED"[CLIENT OBSERVER]"RESET ": File '%s' decompressed successfully.\n", file_path);
        // Remove the compressed file
        if (remove(file_path) != 0) {
            perror("Error deleting the compressed file");
        } else {
            printf(KRED"[CLIENT OBSERVER]"RESET ": Compressed file '%s' deleted successfully.\n", file_path);
        }
    }    
}

void handle_client_organization() {
    int sent;
    int s;
    struct sockaddr_in si_other, servAddr;
    socklen_t slen;
    char message[BUFLEN] = {0}; // Inizializza il buffer a zero
    char buf[BUFLEN] = {0};     // Inizializza il buffer a zero    
    char* buffer_size;
    char *file_path = malloc(sizeof(char) * BUFLEN);

    // setup signal handler
    struct sigaction act;
    sigset_t set;
    sigfillset(&set);
    act.sa_handler = handler;
    act.sa_mask = set;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("Error function sigaction");
        exit(EXIT_FAILURE);
    } else if (sigaction(SIGQUIT, &act, NULL) == -1) {
        perror("Error function sigaction");
        exit(EXIT_FAILURE);
    }
    
    if (file_path == NULL) {
        perror("Error function malloc");
        exit(1);
    }
    char **token_vector;
    // Creazione del socket 
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
       die("Error creating the socket");
    }  
    // Imposta il TOS (ad esempio, DSCP per priorità alta: 0x2E)
    int tos = 0x2E; // Expedited Forwarding (EF)
    if (setsockopt(s, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
        perror("Error setting TOS");
    } 
    // Permette il riutilizzo dell'indirizzo/porta
    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
       perror("Error configuring the socket");
       close(s);
       exit(EXIT_FAILURE);
    }
    // Configurazione timeout per il socket
    struct timeval timeout;
    timeout.tv_sec = 2;  // Timeout di 2 secondi
    timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
       perror("Error setting up the timeout");
       close(s);
       exit(EXIT_FAILURE);
    }

    // Esempio: Convertire e gestire errori
    char *endptr;
    long serv_port = strtol(server_port, &endptr, 10); // Base 10

    if (*endptr != '\0') {
        fprintf(stderr, KRED "[CLIENT]: " KWHT "Error: Port not valid '%s'\n" RESET, server_port);
        exit(EXIT_FAILURE);
    }

    // Inizializzazione della struttura si_other
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons((uint16_t)serv_port);
    slen = sizeof(si_other);
    if (inet_aton(ip_address, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        close(s);
        exit(1);
    }
    // Inizializzazione della struttura servAddr
    memset((char *) &servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons((uint16_t)serv_port);
    servAddr.sin_addr.s_addr = inet_addr(ip_address);
    char* porta_buf = malloc(sizeof(char) * 6);
    if (porta_buf == NULL) {
        perror("Error in malloc\n");
    }
    sprintf(message, "%ld", serv_port);
    sent = (int)sendto(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, slen);
    if (sent == -1) {
       perror("Error sending to the server");
       fprintf(stderr, KRED"Warning: No active server! Make sure the server is running.\n"RESET);
       close(s);
       exit(EXIT_FAILURE);
    }
    if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
       if (errno == EAGAIN || errno == EWOULDBLOCK) {
           fprintf(stderr, KRED"Warning: No active server! Make sure the server is running.\n"RESET);
       } else {
           perror("Error receiving from the server");
       }
       close(s);
       exit(EXIT_FAILURE);
    }  
    memset(message, 0, strlen(message));
    int retry_count = 0;
    while (retry_count < 10) {
        // Uso di rcvMsg con servAddr corretto
        porta_buf = rcvMsg_sync(s, message, BUFLEN, (struct sockaddr_in *) &servAddr);
        if (porta_buf) {
            break; // Esci dal ciclo se la ricezione è riuscita
        }
        perror(KRED "[CLIENT]: " KWHT "Error receiving SYNC message, retrying..."RESET);
        retry_count++;
        sleep(1); // Attendi 1 secondo prima di riprovare
    }
    if (retry_count == 10) {
        perror(KRED "[CLIENT]: " KWHT "Error receiving SYNC message after 10 retries"RESET);
        close(s);
        exit(EXIT_FAILURE);
    }
    int porta = atoi(porta_buf);
    memset(message, 0, strlen(message));
    memset(porta_buf, 0, strlen(porta_buf));
    printf(KRED "[CLIENT]: " KWHT "Connecting to default server IP: %s and default port: %d\n"RESET, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    //printf(KRED "[CLIENT]: " KWHT "Waiting for any connection on IP: %s and default port: %s\n" RESET, ip_address,server_port);
    close(s);  	   
    while(1) {
        if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            die("socket");
        }
        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(porta);
        slen = sizeof(si_other);
        if (inet_aton(ip_address, &si_other.sin_addr) == 0)
            //serve per convertire in una struttura di indirizzi network(IP) da una struttura di indirizzi dot address
        {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
        }
        printf(
            "\n╭───────────────────────────────────────────────────────╮\n"
            "│              🌟 SERVER CONNECTION DETAILS 🌟          │\n"
            "├───────────────────────────────────────────────────────┤\n"
            "│ "KGRN"IP Address:"RESET" %-39s   │\n"
            "│ "KGRN"Port:"RESET" %-43d     │\n"
            "╰───────────────────────────────────────────────────────╯\n",
            inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port)
        );
        printf(
            "\n╭───────────────────────────────────────────────────────╮\n"
            "│              🌟 AVAILABLE COMMANDS 🌟                 │\n"
            "├───────────────────────────────────────────────────────┤\n"
            "│ 1) "KGRN"list"RESET"                                               │\n"
            "│    ➤ Displays the files available on the server       │\n"
            "│ 2) "KGRN"get <FileName>"RESET"                                     │\n"
            "│    ➤ Downloads the file <FileName> from the server    │\n"
            "│ 3) "KGRN"put <FileName>"RESET"                                     │\n"
            "│    ➤ Uploads the file <FileName> to the server        │\n"
            "│ 4) "KGRN"modify <FileName>"RESET"                                  │\n"
            "│    ➤ Edits the file <FileName> locally using Vim      │\n"
            "│ 5) "KGRN"delete <FileName>"RESET"                                  │\n"
            "│    ➤ Delete the file <FileName> from the server       │\n"
            "│ 6) "KGRN"compress <FileName>"RESET"                                │\n"
            "│    ➤ Compress the file <FileName> locally             │\n"
            "│ 7) "KGRN"decompress <FileName>"RESET"                              │\n"
            "│    ➤ Decompress the file <FileName> locally           │\n"
            "│ 8) "KGRN"quit"RESET"                                               │\n"
            "│    ➤ Ends the UDP connection with the server          │\n"
            "╰───────────────────────────────────────────────────────╯\n"
            KYEL"ENTER YOUR DESIRED COMMAND: "RESET
        );
        if (fgets(message, BUFLEN, stdin) == NULL && errno != EINTR) {
            perror("Error in fgets");
            exit(1);
        }
        int len = (int)strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
          message[len - 1] = '\0';
        }
        // Se l'input è troppo corto, chiedi di nuovo l'input
        if (len < 3) {
          fprintf(stderr, KRED "WARNING: You have entered an invalid command. Please try again.\n"RESET);
          continue;  // Torna all'inizio del ciclo per chiedere di nuovo l'input
        }
        char string[BUFLEN];
        strcpy(string, message);
        token_vector = tokenize_string(string, " \n");
        sent = (int)sendto(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, slen);
        if (strncmp(message, "list", 5) == 0) {
            if (sent == -1) {
            die("sendto()");
            }
            // Il client riceve dal server il numero di file presenti nella Lista.txt
            if (recvfrom(s, &Number_file, sizeof(Number_file), 0, (struct sockaddr *) &si_other, &slen) == -1) {
            die("recvfrom()");
            }
            memset(buf, '\0', BUFLEN);
            if (Number_file == 0) {
            printf(
             "\n╭───────────────────────────────────────────────────────╮\n"
             "│                📂 AVAILABLE FILES 📂                  │\n"
             "├───────────────────────────────────────────────────────┤\n"
             "│ "KRED"No files available on the server."RESET"                     │\n"
             "╰───────────────────────────────────────────────────────╯\n"
            );
            } else {
            printf(
             "\n╭───────────────────────────────────────────────────────╮\n"
             "│                📂 AVAILABLE FILES 📂                  │\n"
             "├─────────────┬─────────────────────────────────────────┤\n"
             "│ "KGRN"%d Files"RESET"     │ "KGRN"File Name"RESET"                               │\n"
             "├─────────────┼─────────────────────────────────────────┤\n"
            , Number_file);
            for (int i = 0; i < Number_file; i++) { 
                // Inizializza il buffer a zero
                memset(buf, 0, BUFLEN);
                // Alloca e inizializza la struttura thread_list
                struct thread_list *thread_list = calloc(1, sizeof(struct thread_list));
                if (thread_list == NULL) {
                perror("Error in malloc");
                exit(EXIT_FAILURE);
                }
                // Ricevi dati tramite recvfrom
                if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) {
                perror("recvfrom()");
                free(thread_list); // Libera la memoria in caso di errore
                exit(EXIT_FAILURE);
                }
                // Stampa l'indice e il buffer ricevuto              
                printf("│ %-11d │ %-39s │\n", i + 1, buf);
                // Imposta i valori nella struttura thread_list
                thread_list->buffer = strdup(buf); // Usa strdup, dato che buf è già limitato a BUFLEN
                if (thread_list->buffer == NULL) {
                perror("Error in strdup");
                free(thread_list);
                exit(EXIT_FAILURE);
                }
                // Inizializza il mutex
                if (pthread_mutex_init(&thread_list->mutex_thread, NULL) != 0) {
                perror("pthread_mutex_init");
                free(thread_list->buffer);
                free(thread_list);
                exit(EXIT_FAILURE);
                }
                free(thread_list->buffer);
                free(thread_list);
            }
            printf(
                "╰─────────────┴─────────────────────────────────────────╯\n"
            );
            }
        }
        if (strncmp(token_vector[0], "get", 4) == 0) {
            if (sent == -1) {
                die("sendto()");
            }
            file_path = malloc(sizeof(char) * BUFLEN);
            memset(file_path, 0, BUFLEN);
            memset(message, 0, BUFLEN);
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            if (strcmp(message, "WARNING: The requested file is empty, please try another file!") == 0) {
                printf( KRED"%s\n"RESET, message);
            }
            else if (strcmp(message, "FAIL") == 0) {
                fprintf(stderr, "%s\n", KRED "The requested file is not in the list of available files on the server or is empty.\n"RESET);
            }
            else if (token_vector[1]==NULL)  {
                fprintf(stdout,KRED "WARNING: You must enter a file.\n"RESET);
            }                
            else {
                 char *filename_encoded = malloc(strlen(token_vector[1]) + 5); // Spazio per il nome del file e ".txt"
                 if (filename_encoded) {
                     sprintf(filename_encoded, "%s.txt", token_vector[1]); // Concatenazione del nome del file con ".txt"
                 }
                 file_path = obtain_path(file_path, filename_encoded, token_vector[0]); // Ottengo path assoluto del file
                 int fd = openFile(file_path);
                 rcvFile(s, fd, 0, si_other);
                 close(fd);
                 memset(file_path, '\0', strlen(file_path));
                 const char *directory = "Download";
                 decoder_handler(directory, directory, filename_encoded); // Decoder sul file ricevuto
                 delete_file(directory, filename_encoded); 
                 printf(KRED "[FILE HANDLER]: " KCYN "I have received the file %s\n"RESET, token_vector[1]);
                 free(filename_encoded);
            }
        }
        if (strncmp(token_vector[0], "put", 4) == 0) {
            char *absolute_path = malloc(sizeof(char) * BUFLEN);
            if (absolute_path == NULL) {
                perror("Error function malloc");
                exit(EXIT_FAILURE);
            }
            else if (token_vector[1]==NULL)  {
                fprintf(stdout,KRED "WARNING: You must enter a file.\n"RESET);

            } 
            else{
                obtain_path2(file_path, token_vector[0], absolute_path); // ottengo path_assoluto file
                memset(file_path, '\0', BUFLEN);
                strcpy(file_path, absolute_path);
                free(absolute_path);
                /*rapido controllo per vedere se il file che sto scegliendo di postare ce l'ho davvero in upload
                * perchè altrimenti non posso inviarglielo*/
                char **directoryContent = malloc(sizeof(char*) * getNumberOfElementsInDir(file_path));
                if (directoryContent == NULL) {
                    perror("Error allocating memory for directoryContent");
                    exit(EXIT_FAILURE);
                }
                int dirsize = getNumberOfElementsInDir(file_path);
                directoryContent = getContentDirectory(file_path);
                bool exists = checkFileInDirectory(token_vector[1], directoryContent, dirsize);
                if (exists == true) {
                    /*il file esiste e quindi posso inviarglielo*/
                    char *filename = strdup(token_vector[1]);
                    const char *directory = "Upload";
                    char full_file_path[MAX_PATH];
                    snprintf(full_file_path, sizeof(full_file_path), "%s%s", file_path, filename);
                    // Controlla se il percorso è una cartella normale non compressa
                    struct stat path_stat;
                    if (stat(full_file_path, &path_stat) != 0) {
                        perror("Error checking file path");
                        goto ERROR;
                    }
                    if (!S_ISREG(path_stat.st_mode)) {
                        fprintf(stderr, KRED "Error: '%s' is not a regular file.\n" RESET, filename);
                        goto ERROR;
                    }
                    encoder_handler(directory, directory, filename);
                    char *filename_encoded = malloc(strlen(token_vector[1]) + 5); // Spazio per il nome del file e ".txt"
                    if (filename_encoded) {
                        sprintf(filename_encoded, "%s.txt", token_vector[1]); // Concatenazione del nome del file con ".txt"
                    }
                    strcat(file_path, filename_encoded);
                    int fds = openFileForSending(file_path);
                    long size = getFileSize(file_path);
                    if (size == 0) {
                        goto ERROR;
                    }
                    buffer_size = malloc(BUFLEN * sizeof(char));
                    if (buffer_size == NULL) {
                        perror("Error function malloc");
                        exit(1);
                    }
                    sprintf(buffer_size, "%ld", size);
                    sent = (int) sendto(s, buffer_size, sizeof(buffer_size), 0, (struct sockaddr *) &si_other, slen);
                    sendFile(s, fds, si_other);
                    close(fds);
                    close(s);
                    delete_file(directory, filename_encoded); 
                    free(directoryContent);
                    free(buffer_size);
                    printf(KRED "[FILE HANDLER]: " KCYN "File sent: %s\n"RESET, token_vector[1]);

                } else {
                    ERROR:
                    /*il file non esiste, non lo possiedo*/
                    printf(KRED"The file is not in the Upload folder or is empty.\n"RESET);
                    sendto(s, "Error_path", sizeof(char) * 11, 0, (struct sockaddr *) &si_other, slen);
                }
                memset(file_path, '\0', BUFLEN * sizeof(char));
            }
        }
        //Gestione del comando modify
        if (strncmp(message, "modify", 7) == 0) {
            if (strlen(message) == 6) { 
               // Caso: comando esattamente "modify" (senza argomenti)
               modify_file_interactive();
            } else {
                printf("Error: The 'modify' command does not accept arguments.\n");
            }
        }
       //Gestione del comando delete
        if (strncmp(token_vector[0], "delete", 7) == 0) { 
            if (sent == -1) {
                die("sendto()");
            }
            if (token_vector[1]==NULL)  {
                fprintf(stdout,KRED "WARNING: You must enter a file.\n"RESET);
            }
            else {
                struct timeval timeout1;
                timeout1.tv_sec = 1.5;
                timeout1.tv_usec = 0;
                if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout1, sizeof(timeout1)) < 0) {
                    perror("Error setting up the timeout");
                    close(s);
                    exit(EXIT_FAILURE);
                }
                if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                    //die("recvfrom()");
                }
                if (strcmp(message, "FAIL") == 0) {
                    fprintf(stderr, "%s\n", KRED "The requested file is not in the list of available files on the server.\n"RESET);
                }
                else{
                    printf(KRED "[CLIENT]: " KWHT "File %s deleted successfully.\n" RESET, token_vector[1]);   
                }
            }              
        }
        //Gestione del comando compress
        if (strncmp(token_vector[0], "compress", 9) == 0) {
            if (sent == -1) {
                die("sendto()");
            }
            compress_interactive_file();
        }
        //Gestione del comando decompress
        if (strncmp(token_vector[0], "decompress", 11) == 0) {
            if (sent == -1) {
                die("sendto()");
            }
            decompress_interactive_file();
        }  

        //Gestione del comando quit
        if (strncmp(token_vector[0], "quit", 5) == 0) {
            printf(KRED "[CLIENT]: " KWHT "Notifying the server of system exit\n" RESET);           
            memset(message, 0, strlen(message));
            // Imposta un timeout per la ricezione del messaggio di conferma          
            timeout.tv_sec = 2;  // Timeout of 2 seconds
            timeout.tv_usec = 0;            
            if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror(KRED "[CLIENT]: " KWHT  "Error setting up the timeout" RESET);        
            // Libera file_path
            free(file_path);

            // Libera porta_buf
            free(porta_buf);

            // Libera la memoria allocata per token_vector
            for (int i = 0; token_vector[i] != NULL; i++) {
                free(token_vector[i]);
            }
            free(token_vector);

            close(s);
            exit(EXIT_FAILURE);            
            }
            const char *ack_message = "FIN";
            if (sendto(s, ack_message, strlen(ack_message), 0, (struct sockaddr *) &si_other, slen) == -1) {
            perror("Error while sending ACK");
            }
            // Receives a message with content: FIN+ACK
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {                
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf(KRED "[CLIENT]: " KWHT "Timeout: no response from the server. Closing the client.\n" RESET);                
            } else {
                die("recvfrom()");               
            }
            } else {                
            printf(KRED "[CLIENT]: " KWHT "Received successfully %s\n" RESET, message);
            // Sends an ACK to the server
            ack_message = "ACK";
            if (sendto(s, ack_message, strlen(ack_message), 0, (struct sockaddr *) &si_other, slen) == -1) {
                perror("Error while sending ACK");
            }
            }
            // Receives a message with content: Closing session of this socket.\nGoodbye!
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {                
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf(KRED "[CLIENT]: " KWHT "Timeout: no response from the server. Closing the client.\n" RESET);                
            } else {
                die("recvfrom()");               
             }
            } else {                
            printf(KRED "[CLIENT]: " KWHT "%s\n" RESET, message);
            }        
            close(s);
            free(base64_chars);
            free(BASE64_CHARS);
            // Libera file_path
            free(file_path);
           
            free(token_vector);            
            exit(0);
        }
        //Casistica in cui il comando mandato dall'utente non viene riconosciuto tra i comandi(GET,PUT,LIST,QUIT,DELETE)
        if (strncmp(token_vector[0], "list", 5) != 0 && strncmp(token_vector[0], "get", 4) != 0 &&
            strncmp(token_vector[0], "quit", 5) && strncmp(token_vector[0], "put", 4) != 0 &&
            strncmp(token_vector[0], "modify", 7) != 0 && strncmp(token_vector[0], "delete", 7) != 0 &&
            strncmp(token_vector[0], "compress", 9) != 0 && strncmp(token_vector[0], "decompress", 11) != 0) {
            memset(message, 0, strlen(message));
            if (recvfrom(s, message, sizeof(message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
                die("recvfrom()");
            }
            printf(KRED"%s\n"RESET, message);
        }
        free(token_vector);
        
    }
        close(s);
}

// Funzione per verificare se l'IP è raggiungibile usando il comando 'ping'
int check_ip_availability(const char *ip) {
    char command[256];
    // Costruisci il comando ping (il flag -c 1 invia 1 pacchetto ICMP)
    snprintf(command, sizeof(command), "ping -c 1 -w 1 %s > /dev/null 2>&1", ip);
    // Esegui il comando e controlla se è stato eseguito con successo
    int result = system(command);
    return (result == 0);  // Se il comando ha successo, il risultato sarà 0
}

int main(void) {
    char *endptr;
    long serv_port = 0;
    printf(KRED "[CLIENT]: " KWHT "Entering server configuration...\n" RESET);
    REBIND_IP:
    // Richiedi all'utente di inserire un indirizzo IP
    printf(KRED "[CLIENT]: " KWHT "Please enter the server IP address: " RESET);
    if (fgets(ip_address, sizeof(ip_address), stdin) == NULL) {
        perror("Error while inserting the IP");
        return 1;
    }
    // Rimuove il newline finale se presente
    ip_address[strcspn(ip_address, "\n")] = 0;
    // Verifica se l'IP è raggiungibile
    if (!check_ip_availability(ip_address)) {
        printf(KRED "[CLIENT]: " KWHT "The IP address %s is not reachable.\n" RESET, ip_address);
        goto REBIND_IP;
    } else {
        printf(KRED "[CLIENT]: " KWHT "The IP address %s is reachable.\n" RESET, ip_address);
    }
    REBIND_PORT:
    // Richiedi all'utente di inserire una porta
    printf(KRED "[CLIENT]: " KWHT "Please enter the server Default Port: " RESET);
    if (fgets(server_port, sizeof(server_port), stdin) == NULL) {
        perror(KRED "[CLIENT]: " KWHT "Error while inserting the Default port" RESET);
        goto REBIND_IP;
    }

    // Rimuove il newline finale se presente
    server_port[strcspn(server_port, "\n")] = 0;

    // Converte la porta in un numero
    serv_port = strtol(server_port, &endptr, 10); // Base 10 per numeri decimali

    // Controllo della validità della conversione e dei limiti
    if (*endptr != '\0') {
        fprintf(stderr, KRED "[CLIENT]: " KWHT "Error: the port needs to be a valid number.\n" RESET);
        goto REBIND_PORT;
    }

    if(serv_port < 7000 || serv_port > 65535) {
        printf(KRED "[CLIENT]: " KWHT "The default port %s is not reachable.\n" RESET, server_port);
        goto REBIND_PORT;
    } else {
        printf(KRED "[CLIENT]: " KWHT "The default port %s is reachable.\n" RESET, server_port);
    }

    printf("\n\n\n");
    printf(KRED "[OPTION]: " KWHT "Loss probability (P): %.2f%%\n",P*100);
    printf(KRED "[OPTION]: " KWHT "Sliding window size (N): %d\n",N);
    printf(KRED "[OPTION]: " KWHT "Timeout value (T fixed): %d s\n", T/1000000);
    printf(KRED "[OPTION]: " KWHT "Fixed timeout mode: %s\n", FIXEDTIMEOUT ? "ON" : "OFF");
    printf(KRED "[OPTION]: " KWHT "Maximum number of retransmissions: %d times\n", MAXIMUM_ATTEMPT);
    printf(KRED "[OPTION]: " KWHT "Performance mode: %s\n",PERFORMANCE ? "ON" : "OFF");
    printf(KRED "[OPTION]: " KWHT "Max threads: %d", MAX_THREADS);


    // Rimuove il newline finale se presente
    server_port[strcspn(server_port, "\n")] = 0;
    printf("\n\n\n");
    printf(KGRN "───────────────────────────────────────────────────────────────────────────────\n" RESET);
    printf(KRED "[CLIENT]: " KWHT "Waiting for any connection on IP: %s and default port: %s\n" RESET, ip_address,server_port);
    printf(KGRN "───────────────────────────────────────────────────────────────────────────────\n" RESET);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int combination = (tm.tm_mon + 1) * 100 + (tm.tm_year + 1900);
    initialize_base64_alphabet_encode(combination);
    initialize_base64_alphabet_decode(combination);
    
    // Gestione delle richieste dei client
    handle_client_organization();
    return 0;
}