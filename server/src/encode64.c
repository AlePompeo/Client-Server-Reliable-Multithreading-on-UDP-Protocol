#include "macros.h"
#include "encode64.h"
#include "reliableUDP.h"

//const char *BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char *BASE64_CHARS = NULL;

// Funzione per inizializzare l'alfabeto Base64 shiftato
void initialize_base64_alphabet_encode(int shift) {
    BASE64_CHARS = generate_shifted_base64_alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", shift);
    if (BASE64_CHARS == NULL) {
        fprintf(stderr, "Errore: impossibile generare l'alfabeto Base64.\n");
        exit(EXIT_FAILURE);
    }
}

void base64_encode(FILE *input, FILE *output) {
    unsigned char input_buffer[ENCODE_BLOCK_SIZE]; // Buffer per leggere i dati dal file di input
    char output_buffer[OUTPUT_BUFFER_SIZE];       // Buffer per scrivere i dati codificati in Base64
    size_t bytes_read;

    // Legge i dati dal file di input in blocchi
    while ((bytes_read = fread(input_buffer, 1, ENCODE_BLOCK_SIZE, input)) > 0) {
        size_t output_index = 0;

        // Processa i dati letti in blocchi di 3 byte
        for (size_t i = 0; i < bytes_read; i += 3) {
            unsigned char b1 = input_buffer[i];                          // Primo byte
            unsigned char b2 = (i + 1 < bytes_read) ? input_buffer[i + 1] : 0; // Secondo byte (se presente)
            unsigned char b3 = (i + 2 < bytes_read) ? input_buffer[i + 2] : 0; // Terzo byte (se presente)

            // Codifica i 3 byte in 4 caratteri Base64
            output_buffer[output_index++] = BASE64_CHARS[(b1 >> 2) & 0x3F]; // Primi 6 bit del primo byte
            output_buffer[output_index++] = BASE64_CHARS[((b1 & 0x3) << 4) | (b2 >> 4)]; // Ultimi 2 bit del primo byte + primi 4 bit del secondo byte
            output_buffer[output_index++] = (i + 1 < bytes_read) ? BASE64_CHARS[((b2 & 0xF) << 2) | (b3 >> 6)] : '='; // Ultimi 4 bit del secondo byte + primi 2 bit del terzo byte (o '=' se mancante)
            output_buffer[output_index++] = (i + 2 < bytes_read) ? BASE64_CHARS[b3 & 0x3F] : '='; // Ultimi 6 bit del terzo byte (o '=' se mancante)
        }

        // Scrive i caratteri codificati nel file di output
        fwrite(output_buffer, 1, output_index, output);
    }
}

int is_regular_file_encode(const char *path) {
    struct stat path_stat;
    return stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode);
}

int convert_file_to_base64(const char *input_filepath, const char *output_filepath) {
    FILE *input = fopen(input_filepath, "rb");
    FILE *output = fopen(output_filepath, "w");
    if (!input || !output) {
        perror(KRED "[CODEBASE64]: " KWHT "Errore nell'apertura del file" RESET);
        if (input) fclose(input);
        if (output) fclose(output);
        return -1;
    }
    // Scrive l'estensione all'inizio del file di output
    const char *extension = strrchr(input_filepath, '.');
    fprintf(output, "ext:%s\n", extension ? extension + 1 : "");
    // Codifica il file in Base64
    base64_encode(input, output);
    fclose(input);
    fclose(output);
    if(!PERFORMANCE) {
        printf(KRED "[CODEBASE64]: " KWHT "File %s convertito in %s\n" RESET, input_filepath, output_filepath);
    }
    return 0;
}

int encoder_handler(const char *input_directory, const char *output_directory, const char *filename) {
    char input_filepath[512];
    char output_filepath[512];
    // Costruisce i percorsi di input e output
    snprintf(input_filepath, sizeof(input_filepath), "%s/%s", input_directory, filename);
    snprintf(output_filepath, sizeof(output_filepath), "%s/%s.txt", output_directory, filename);
    // Verifica se il file di input è valido
    if (!is_regular_file_encode(input_filepath)) {
        fprintf(stderr,KRED "[CODEBASE64]: " KWHT "Errore: %s non è un file valido.\n" RESET, input_filepath);
        return -1;
    }
    // Converte il file in Base64
    if (convert_file_to_base64(input_filepath, output_filepath) == -1) {
        fprintf(stderr,KRED "[CODEBASE64]: " KWHT "Errore durante la conversione di %s.\n" RESET, input_filepath);
        return -1;
    }
    if(!PERFORMANCE) {
        printf(KRED "[CODEBASE64]: " KWHT "File %s codificato con successo in %s.\n" RESET, input_filepath, output_filepath);
    }
    return 0;
}