#include "macros.h"
#include "serverfileoperation.h"

int openFile(const char *path) {
    int fd;
    // Prova ad aprire il file in lettura/scrittura. Se non esiste, lo crea.
    fd = open(path, O_RDWR | O_CREAT, 0666); // O_CREAT aggiunto per creare il file se non esiste
    if (fd == -1) {
        perror(KRED "[FILEOP]: " KWHT "Error in openFile()" RESET);
        return -1;
    }
    return fd;
}

void closeFile(int fd) {
	errno = 0;
	if(close(fd) == -1) {
			exit(EXIT_FAILURE);
	}
}

int readFile(int fd, char *buf) {
	unsigned long r;
	ssize_t v;
		
	r = 0;
	while(r < DATA_SIZE) {
		errno = 0;
		v = read(fd, buf, DATA_SIZE - r);
		if(v == -1) {
			exit(EXIT_FAILURE);
		}
		if(v == 0)
			return r;

		r += v;
		buf += v;
	}

	return r;
}

void writeFile(int fd, char *buf) {
	ssize_t v, dim;
	dim = strlen(buf);
	
	while(dim > 0) {
		errno = 0;
		v = write(fd, buf, dim);

		if(v == -1) {
			exit(EXIT_FAILURE);
		}
		if(v == 0)   		//zero indica che sono stati scritti 0 byte 
			break;  
    
		dim -= v;		
		buf += v;
	}
}

long getFileSize(const char *path) {
	FILE *file = NULL;
	long file_size;

	file = fopen(path, "r");
	if(file == NULL){
		exit(EXIT_FAILURE);
	}

	if(fseek(file, 0L, SEEK_END) == -1){		//fseek() permette di modificare l'indicatore di posizone del file
		exit(EXIT_FAILURE);
	}

	file_size = ftell(file); 			//ftell() restituisce la dimensione del file in byte
	if(file_size == -1){
		exit(EXIT_FAILURE);
	}

	fseek(file, 0L, SEEK_SET);

	if(fclose(file) != 0){
		exit(EXIT_FAILURE);
	}

	return file_size;
}

char* obtain_path(char* file_path, char* token, char* cmd) {
    // Ottieni la directory corrente
    if (getcwd(file_path, BUFLEN) == NULL) {
        perror(KRED "[FILEOP]: " KWHT "Error during getcwd" RESET);
        file_path[0] = '\0'; // Imposta il buffer come stringa vuota in caso di errore
        return file_path;
    }

    // Configura il percorso base per l'esecuzione
    strcat(file_path, "/server_files/");

    // Modifica il percorso in base al comando ricevuto
    if (strncmp(cmd, "get", 4) == 0 || strncmp(cmd, "put", 4) == 0) {
        strcat(file_path, ""); // Percorso invariato per "get" e "put"
    } else if (strncmp(cmd, "list", 5) == 0) {
        file_path[strlen(file_path) - strlen("server_files/")] = '\0'; // Rimuove "server_files/"
    }

    // Aggiungi il token al percorso finale
    strcat(file_path, token);

    return file_path; // Restituisce il buffer passato come argomento
}

char** alloc_memory(size_t size) {
    if (size == 0) {
        fprintf(stderr,KRED "[FILEOP]: " KWHT "Error: Requested allocation size is 0\n" RESET);
        exit(EXIT_FAILURE);
    }

    char **buf = calloc(size, sizeof(char*));
    if (!buf) {
        perror(KRED "[FILEOP]: " KWHT "Error in calloc" RESET);
        exit(EXIT_FAILURE);
    }

    return buf;
}

void tokenize_string(char *buffer, const char *delimiter, char **tokens) {
    char *token;  // Dichiarazione della variabile token
    int i = 0;    // Inizializzazione dell'indice a 0 per il primo token

    // Inizializza il primo token
    token = strtok(buffer, delimiter);
    while (token != NULL) {
        tokens[i++] = token;  // Assegna il token attuale all'array di tokens e incrementa l'indice
        token = strtok(NULL, delimiter);  // Ottieni il prossimo token
    }
    tokens[i] = NULL;  // Aggiungi un NULL alla fine dell'array per indicare la fine
}


double print_total_time(struct timeval tv1) {
    struct timeval tv2;
    gettimeofday(&tv2, NULL); // Prende il tempo corrente

    // Calcola il tempo totale in secondi
    double total_time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec) / 1000000.0;

    return total_time;
}

char* list_files(const char *dir_name) {
    // Alloca un buffer dinamico
    char *buffer = malloc(1024);
    if (buffer == NULL) {
        perror(KRED "[FILEOP]: " KWHT "Memory allocation error" RESET);
        exit(EXIT_FAILURE);
    }
    
    buffer[0] = '\0';  // Inizializza il buffer come stringa vuota

    // Apri la directory
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror(KRED "[FILEOP]: " KWHT "Error opening the directory" RESET);
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Leggi la directory
    struct dirent *entry;
    size_t buffer_len = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Escludere le voci '.' e '..'
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            size_t entry_len = strlen(entry->d_name) + 1;

            // Verifica se serve più spazio e rialloca se necessario
            if (buffer_len + entry_len + 1 >= 1024) {
                char *new_buffer = realloc(buffer, buffer_len + entry_len + 1);
                if (new_buffer == NULL) {
                    perror(KRED "[FILEOP]: " KWHT "Reallocation error" RESET);
                    free(buffer);
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }
                buffer = new_buffer;
            }

            strcat(buffer, entry->d_name);
            strcat(buffer, "\n");
            buffer_len += entry_len;
        }
    }

    // Chiudi la directory
    closedir(dir);

    return buffer;  // Restituisce il buffer dinamico
}

void update_file_with_list(const char *file_name, const char *dir_name) {
    // Chiama la funzione list_files per ottenere la lista dei file nella directory
    char *file_list = list_files(dir_name);
    if (file_list == NULL) {
        fprintf(stderr, KRED "[FILEOP]: " KWHT "Error in obtaining the file list.\n" RESET);
        return;
    }

    // Apri il file in modalità "w" per sovrascrivere il contenuto
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        perror(KRED "[FILEOP]: " KWHT "Error opening the file" RESET);
        free(file_list);
        return;
    }

    // Verifica se la lista è vuota e scrive un messaggio se non ci sono file
    if (file_list[0] == '\0') {
        fprintf(file, KRED "[FILEOP]: " KWHT "No files found\n" RESET);
    } else {
        fprintf(file, "%s", file_list);
    }

    // Chiudi il file e libera la memoria
    fclose(file);
    free(file_list);
}

void delete_file(const char *directory, const char *filename) {
    char *full_path;
    size_t dir_len = strlen(directory);
    size_t filename_len = strlen(filename);

    // Controlla se il percorso termina con una barra, aggiungila se necessario
    int needs_slash = (directory[dir_len - 1] != '/');
    full_path = malloc(dir_len + filename_len + 2); // +1 per '/' e +1 per '\0'

    if (!full_path) {
        perror(KRED "[FILEOP]: " KWHT "Memory allocation error for full_path" RESET);
        return;
    }

    // Crea il percorso completo
    if (needs_slash) {
        sprintf(full_path, "%s/%s", directory, filename);
    } else {
        sprintf(full_path, "%s%s", directory, filename);
    }

    // Elimina il file
    if (remove(full_path) == 0) {
        if(!PERFORMANCE) {
            printf(KRED "[SERVER]: " KWHT "File %s successfully deleted.\n" RESET, full_path);
        }
    } else {
        perror(KRED "[SERVER]: " KWHT "Error while deleting the file" RESET);
    }

    free(full_path);
}