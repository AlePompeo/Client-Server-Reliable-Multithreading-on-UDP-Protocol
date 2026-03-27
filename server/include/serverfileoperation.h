#ifndef SERVERFILEOPERATION_H
#define SERVERFILEOPERATION_H

#include "macros.h"

int openFile(const char *path);
void closeFile(int fd);
int readFile(int fd, char *buf);
void writeFile(int fd, char *buf);
long getFileSize(const char *path);
char* obtain_path(char*file_path,char*token,char* cmd);
char** alloc_memory();
void tokenize_string(char *buffer, const char *delimiter, char **tokens);
double print_total_time(struct timeval tv1);
char* list_files(const char *dir_name);
void update_file_with_list(const char *file_name, const char *dir_name);
void delete_file(const char *directory, const char *filename);

#endif
