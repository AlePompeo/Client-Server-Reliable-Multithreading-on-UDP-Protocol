#include "macros.h"
#include "reliableUDP.h"

//checksum
unsigned long calculateChecksum(const char *data, size_t length) {
    unsigned long checksum = crc32(0L, (const Bytef *)data, length);
    if (!PERFORMANCE) {
        printf(KRED "[CHECKSUM]: " KWHT "Calculated checksum: %lu\n" RESET, checksum);
    }
    return checksum;
}

//congestion control
long unsigned estimateTimeout(long unsigned *EstimatedRTT, long unsigned *DevRTT, long unsigned SampleRTT) {
    *EstimatedRTT = 0.875 * (*EstimatedRTT) + 0.125 * SampleRTT;
    //*DevRTT = 0.75 * (*DevRTT) + 0.25 * abs(SampleRTT - *EstimatedRTT);
    *DevRTT = 0.75 * (*DevRTT) + 0.25 * (SampleRTT > *EstimatedRTT ? SampleRTT - *EstimatedRTT : *EstimatedRTT - SampleRTT);
    if (!FIXEDRTT) {
        long unsigned timeoutInterval = (*EstimatedRTT + 4 * (*DevRTT));
        return timeoutInterval;
    } else {
        return RTT; // Fixed RTT value (1 second)
    }
}

//encryption
char *generate_shifted_base64_alphabet(const char *base64_chars, int shift) {
    size_t length = strlen(base64_chars);

    // Controlla che l'alfabeto originale non sia NULL
    if (base64_chars == NULL) {
        fprintf(stderr, "Errore: l'alfabeto originale è NULL.\n");
        return NULL;
    }

    // Alloca memoria per l'alfabeto shiftato
    char *shifted_alphabet = malloc(length + 1); // +1 per il terminatore '\0'
    if (shifted_alphabet == NULL) {
        fprintf(stderr, "Errore: impossibile allocare memoria per l'alfabeto shiftato.\n");
        return NULL;
    }

    // Normalizza lo shift per evitare valori fuori dal range
    if (shift < 0) {
        shift = (shift % length + length) % length; // Gestisce shift negativi
    } else {
        shift = shift % length; // Riduce lo shift al range valido
    }

    // Applica lo shift ciclico
    for (size_t i = 0; i < length; i++) {
        shifted_alphabet[i] = base64_chars[(i + shift) % length];
    }
    shifted_alphabet[length] = '\0'; // Assicurati che la stringa sia terminata

    return shifted_alphabet; // Ritorna l'alfabeto shiftato
}