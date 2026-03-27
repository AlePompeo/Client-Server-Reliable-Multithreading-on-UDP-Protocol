#ifndef CLIENTFILEOPERATION_H
#define CLIENTFILEOPERATION_H

#include "macros.h"

int openFile(const char *path);
int openFileForSending(const char *path);
void closeFile(int fd);
int readFile(int fd, char *buf);
void writeFile(int fd, char *buf);
long getFileSize(const char *path);
char* readLine(char *buf, FILE *f);
bool checkFileName(char* filename, char* path_list);
char** alloc_memory();
char ** getContentDirectory(char *dirpathname);
int getNumberOfElementsInDir(char *dirpathname);
bool checkFileInDirectory(char *filename,char **dircontent,int dirsize);
char * obtain_path(char*filepath, char*token, char* cmd);
char* obtain_path2(char *filepath, char *cmd, char *assolute_path);
void delete_file(const char *directory, const char *filename);

#endif 
