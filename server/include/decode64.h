#ifndef DECODE64_H
#define DECODE64_H

extern char *base64_chars;

int base64_char_index(char c);
void base64_decode(FILE *input, FILE *output);
int is_regular_file_decode(const char *path);
int decode_base64_file(const char *input_filename, const char *output_filename_base);
int decoder_handler(const char *input_directory, const char *output_directory, const char *filename);
void initialize_base64_alphabet_decode(int shift);

#endif // DECODE64_H