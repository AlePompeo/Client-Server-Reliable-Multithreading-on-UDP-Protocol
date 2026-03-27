#ifndef ENCODE64_H
#define ENCODE64_H

#include "macros.h"

extern char *BASE64_CHARS;

void base64_encode(FILE *input, FILE *output);
int is_regular_file_encode(const char *path);
int convert_file_to_base64(const char *input_filepath, const char *output_filepath);
int encoder_handler(const char *input_directory, const char *output_directory, const char *filename);
void initialize_base64_alphabet_encode(int shift);

#endif