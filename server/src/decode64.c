#include "macros.h"
#include "decode64.h"
#include "reliableUDP.h"

//const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *base64_chars = NULL;

// Funzione per inizializzare l'alfabeto Base64 shiftato
void initialize_base64_alphabet_decode(int shift) {
    base64_chars = generate_shifted_base64_alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", shift);
    if (base64_chars == NULL) {
        fprintf(stderr, "Errore: impossibile generare l'alfabeto Base64.\n");
        exit(EXIT_FAILURE);
    }
}

// Funzione per ottenere l'indice di un carattere nell'alfabeto Base64
int base64_char_index(char c) {
    // Cerca il carattere nell'alfabeto Base64
    const char *pos = strchr(base64_chars, c);

    // Se il carattere è trovato, restituisce l'indice, altrimenti -1
    return pos ? pos - base64_chars : -1;
}

void base64_decode(FILE *input, FILE *output) {
    unsigned char input_buffer[INPUT_BUFFER_SIZE];
    unsigned char output_buffer[(INPUT_BUFFER_SIZE / DECODE_BLOCK_SIZE) * OUTPUT_BLOCK_SIZE];
    size_t bytes_read;

    // Legge il file di input in blocchi
    while ((bytes_read = fread(input_buffer, 1, INPUT_BUFFER_SIZE, input)) > 0) {
        size_t output_index = 0;

        // Processa i dati letti in blocchi di 4 caratteri Base64
        for (size_t i = 0; i < bytes_read; i += DECODE_BLOCK_SIZE) {
            int indices[4] = { -1, -1, -1, -1 };

            // Riempi il buffer di input e calcola gli indici Base64
            for (size_t j = 0; j < DECODE_BLOCK_SIZE && (i + j) < bytes_read; j++) {
                indices[j] = base64_char_index(input_buffer[i + j]);
                // Controlla se il carattere è valido, altrimenti segnala errore
                if (indices[j] == -1 && input_buffer[i + j] != '=') {
                    fprintf(stderr,KRED "[CODEBASE64]: " KWHT "Carattere Base64 non valido: %c\n" RESET, input_buffer[i + j]);
                    exit(EXIT_FAILURE);
                }
            }

            // Decodifica il blocco di 4 caratteri in un massimo di 3 byte
            if (indices[0] >= 0 && indices[1] >= 0) {
                // Primo byte decodificato
                output_buffer[output_index++] = (indices[0] << 2) | (indices[1] >> 4);

                if (indices[2] >= 0) {
                    // Secondo byte decodificato
                    output_buffer[output_index++] = ((indices[1] & 0xF) << 4) | (indices[2] >> 2);

                    if (indices[3] >= 0) {
                        // Terzo byte decodificato
                        output_buffer[output_index++] = ((indices[2] & 0x3) << 6) | indices[3];
                    }
                }
            }
        }

        // Scrive il buffer decodificato nel file di output
        fwrite(output_buffer, 1, output_index, output);
    }
}

int is_regular_file_decode(const char *path) {
    struct stat path_stat;
    return (stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode));
}

int decode_base64_file(const char *input_filename, const char *output_filename_base) {
    FILE *input = fopen(input_filename, "r");
    if (!input) {
        perror(KRED "[CODEBASE64]: " KWHT "Errore nell'apertura del file di input" RESET);
        return -1;
    }

    char extension[64];
    if (fscanf(input, "ext:%63s\n", extension) != 1) {
        fprintf(stderr,KRED "[CODEBASE64]: " KWHT "Errore: estensione non trovata\n" RESET);
        fclose(input);
        return -1;
    }

    char output_filename[256];
    if (strstr(output_filename_base, extension) == NULL) {
        snprintf(output_filename, sizeof(output_filename), "%s.%s", output_filename_base, extension);
    } else {
        snprintf(output_filename, sizeof(output_filename), "%s", output_filename_base);
    }

    FILE *output = fopen(output_filename, "wb");
    if (!output) {
        perror(KRED "[CODEBASE64]: " KWHT "Errore nell'apertura del file di output" RESET);
        fclose(input);
        return -1;
    }

    base64_decode(input, output);

    fclose(input);
    fclose(output);
    if(!PERFORMANCE) {
        printf(KRED "[CODEBASE64]: " KWHT "File %s decodificato in %s\n" RESET, input_filename, output_filename);
    }
    return 0;
}

int decoder_handler(const char *input_directory, const char *output_directory, const char *filename) {
    char input_filepath[512];
    snprintf(input_filepath, sizeof(input_filepath), "%s/%s", input_directory, filename);

    if (!is_regular_file_decode(input_filepath)) {
        fprintf(stderr,KRED "[CODEBASE64]: " KWHT "Errore: %s non è un file valido.\n" RESET, input_filepath);
        return -1;
    }

    char output_filename_base[512];
    snprintf(output_filename_base, sizeof(output_filename_base), "%s/%s", output_directory, filename);

    char *ext_pos = strstr(output_filename_base, ".txt");
    if (ext_pos) *ext_pos = '\0';

    return decode_base64_file(input_filepath, output_filename_base);
}