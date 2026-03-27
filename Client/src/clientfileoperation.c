#include "macros.h"
#include "clientfileoperation.h"

int openFile(const char *path) {
    int fd;
    errno = 0;
    fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0666);  //O_RDWR = lettura e scrittura, O_CREAT = se non esiste lo creo
    return fd;
}

int openFileForSending(const char *path) {
    int fd;
    errno = 0;
    fd = open(path, O_RDWR, 0666);  //O_RDWR = lettura e scrittura, O_CREAT = se non esiste lo creo
    if(fd == -1) {
        printf("Errore in openFile()\n");
        exit(EXIT_FAILURE);
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
        if(v == 0)          
            break;
        dim -= v;
        buf += v;
    }
}
 
long getFileSize(const char *path) {
    FILE *file = NULL;
    long file_size = 0;
    file = fopen(path, "r+");
    if(file == NULL){
        exit(EXIT_FAILURE);
    }
    if(fseek(file, 0L, SEEK_END) == -1){        
          exit(EXIT_FAILURE);
    }
    file_size = ftell(file);            
    if(file_size == -1){

        exit(EXIT_FAILURE);
    }
    fseek(file, 0L, SEEK_SET);
    if(fclose(file) != 0){
        exit(EXIT_FAILURE);
    }
    return file_size;
}
 
char* readLine(char *buf, FILE *f) {
    if(fgets(buf, MAX_LINE_SIZE, f) == NULL){
        if(!feof(f)){                   
            exit(EXIT_FAILURE);
        }
        else
            return NULL;                
    }
    if(buf[strlen(buf) - 1] == '\n'){          
        buf[strlen(buf) - 1] = '\0';
    }
    return buf;
}
 
bool checkFileName(char* filename, char* path_list){
    FILE *list = NULL;
    list = fopen(path_list, "r");
    if(list == NULL){
        exit(EXIT_FAILURE);
    }
    bool cond = false;
    char *buf;
    buf = malloc(MAX_LINE_SIZE);
    if(!buf) {
        exit(EXIT_FAILURE);
    }
    for(;;) {
        buf = readLine(buf, list);
        if(!buf)
            break;
        if(strncmp(filename, buf, strlen(filename) ) == 0){
            cond=true;
            break;
        }
    }
    free(buf);
    if(fclose(list) != 0){
        exit(EXIT_FAILURE);
    }
    return cond;
}

char** alloc_memory(){
    char **buf = malloc(BUFLEN * sizeof(char*));
    if (buf == NULL){
        perror("Error function malloc");
        exit(-1);
    }
    return buf;
}

char ** getContentDirectory(char *dirpathname){
    DIR *directory;
    char **content;
    struct dirent *de;
    int j=0;
    directory = opendir(dirpathname);
    if(directory == NULL ){
        printf("Error in opening directory\n");
        return NULL;
    }
    content = alloc_memory();
    if(content == NULL){
        printf("Error in malloc\n");
        closedir(directory);
        return NULL;
    }
    while((de = readdir(directory)) != NULL) {
        content[j] = de->d_name;
        j++;
    }
    if(closedir(directory) == -1) {
        perror("Error closing directory");
        free(content);
        return NULL;
    }
    return content;
}

int getNumberOfElementsInDir(char *dirpathname){
    DIR *directory;
    struct dirent *de;
    int i = 0;
    directory = opendir(dirpathname);
    if(directory == NULL ){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    /* Controllo numero di elementi presenti all'interno della cartella Upload*/
    while((de = readdir(directory)) != NULL){
        i = i + 1;
    }
    if(closedir(directory) == -1) {
        perror("Error closing directory");
        exit(EXIT_FAILURE);
    }
    return i;
}

bool checkFileInDirectory(char *filename,char **dircontent,int dirsize){
    bool trovato = false;
    int i;
    for(i=0;i<dirsize;i++){
        if(strcmp(filename,dircontent[i]) == 0){
            trovato = true;
            break;
        }
        else{
            trovato = false;
        }
    }
    return trovato;
}

char* obtain_path(char*filepath, char*token, char* cmd){
    if (getcwd(filepath, sizeof(char)*BUFLEN) != NULL) {
    } else {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }
    
    if (strncmp(cmd, "get", 4) == 0)
        strcat(filepath, "/Download/");
    else if (strncmp(cmd, "put", 4) == 0)
        strcat(filepath, "/Upload/");
    else if (strncmp(cmd, "list", 5) == 0) 
        strcat(filepath, "/");
    strcat(filepath,token);

    return filepath;
}

char* obtain_path2(char *filepath, char *cmd, char *assolute_path) {
    if (getcwd(filepath, sizeof(char) * BUFLEN) == NULL) {
        perror("getcwd() error");
        exit(EXIT_FAILURE);
    }

    if (strlen(assolute_path) > 1) {
        memset(assolute_path, '\0', BUFLEN);  
    }
    if (strncmp(cmd, "get", 4) == 0)
        strcat(filepath, "/Download/");
    else if (strncmp(cmd, "put", 4) == 0)
        strcat(filepath, "/Upload/");
    else if (strncmp(cmd, "list", 5) == 0) 
        strcat(filepath, "/");
    strcpy(assolute_path, filepath);

    return assolute_path;
}

void delete_file(const char *directory, const char *filename) {
    char *full_path;
    size_t dir_len = strlen(directory);
    size_t filename_len = strlen(filename);
    int needs_slash = (directory[dir_len - 1] != '/');
    full_path = malloc(dir_len + filename_len + 2); 
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

