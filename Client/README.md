**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE CLIENT.C**
### `int main(void)`
- **Descrizione**: Punto di ingresso del programma. Configura l'indirizzo IP e la porta del server, verifica la loro validità e avvia la gestione delle richieste del client.
- **Passaggi principali**:
  1. Richiede l'indirizzo IP del server e verifica la sua raggiungibilità.
  2. Richiede la porta del server e verifica che sia valida.
  3. Inizializza l'alfabeto Base64 per la codifica e decodifica.
  4. Avvia la funzione `handle_client_organization` per gestire le richieste.

---

## Funzioni di gestione del client

### `void handle_client_organization()`
- **Descrizione**: Gestisce la comunicazione con il server e l'esecuzione dei comandi inviati dal client.
- **Funzionalità principali**:
  - Configura il socket UDP e imposta opzioni come il timeout e il riutilizzo dell'indirizzo.
  - Gestisce i comandi disponibili:
    - `list`: Elenca i file disponibili sul server.
    - `get <FileName>`: Scarica un file dal server.
    - `put <FileName>`: Carica un file sul server.
    - `modify`: Modifica un file localmente.
    - `delete <FileName>`: Elimina un file dal server.
    - `compress <FileName>`: Comprimi un file localmente.
    - `decompress <FileName>`: Decomprimi un file localmente.
    - `quit`: Termina la connessione con il server.

---

## Funzioni di utilità

### `void die(char *s)`
- **Descrizione**: Stampa un messaggio di errore e termina il programma.
- **Parametri**:
  - `s`: Messaggio di errore da stampare.

### `void handler(int signum)`
- **Descrizione**: Gestisce i segnali ricevuti dal programma (es. `SIGINT` o `SIGQUIT`).
- **Parametri**:
  - `signum`: Numero del segnale ricevuto.

### `int check_ip_availability(const char *ip)`
- **Descrizione**: Verifica se un indirizzo IP è raggiungibile utilizzando il comando `ping`.
- **Parametri**:
  - `ip`: Indirizzo IP da verificare.
- **Ritorna**: `1` se l'IP è raggiungibile, `0` altrimenti.

---

## Funzioni per la gestione dei file

### `char* list_files(const char *dir_name)`
- **Descrizione**: Elenca i file presenti in una directory.
- **Parametri**:
  - `dir_name`: Nome della directory da analizzare.
- **Ritorna**: Una stringa contenente i nomi dei file separati da newline.

### `void open_with_vim(const char *file_path)`
- **Descrizione**: Apre un file in modalità sandbox utilizzando Vim.
- **Parametri**:
  - `file_path`: Percorso del file da aprire.

### `void modify_file_interactive()`
- **Descrizione**: Permette all'utente di selezionare una directory (`Download` o `Upload`), elencare i file e modificare un file selezionato o crearne uno nuovo.

### `int compressFile(const char *filePath)`
- **Descrizione**: Comprimi un file utilizzando la libreria `zlib`.
- **Parametri**:
  - `filePath`: Percorso del file da comprimere.
- **Ritorna**: `0` in caso di successo, `-1` in caso di errore.

### `int decompressFile(const char *filePath)`
- **Descrizione**: Decomprimi un file `.gz` utilizzando la libreria `zlib`.
- **Parametri**:
  - `filePath`: Percorso del file da decomprimere.
- **Ritorna**: `0` in caso di successo, `-1` in caso di errore.

### `void compress_interactive_file()`
- **Descrizione**: Permette all'utente di selezionare una directory (`Download` o `Upload`), elencare i file e comprimere un file selezionato.

### `void decompress_interactive_file()`
- **Descrizione**: Permette all'utente di selezionare una directory (`Download` o `Upload`), elencare i file e decomprimere un file selezionato.

---

## Funzioni per la gestione della codifica Base64

### `void initialize_base64_alphabet_encode(int combination)`
- **Descrizione**: Inizializza l'alfabeto Base64 per la codifica.

### `void initialize_base64_alphabet_decode(int combination)`
- **Descrizione**: Inizializza l'alfabeto Base64 per la decodifica.

---

## Funzioni per la gestione dei thread

### `void* thread_function(void *p)`
- **Descrizione**: Funzione eseguita dai thread per stampare un messaggio.
- **Parametri**:
  - `p`: Puntatore alla struttura `thread_list`.

### `char** tokenize_string(char *buffer, char *delimiter)`
- **Descrizione**: Divide una stringa in token utilizzando un delimitatore.
- **Parametri**:
  - `buffer`: Stringa da dividere.
  - `delimiter`: Delimitatore da utilizzare.
- **Ritorna**: Un array di stringhe contenente i token.

---

## Funzioni di rete

### `char* rcvMsg_sync(int sock, char *buffer, size_t buffer_len, struct sockaddr_in *addr)`
- **Descrizione**: Riceve un messaggio dal server in modo sincrono.
- **Parametri**:
  - `sock`: Socket utilizzato per la comunicazione.
  - `buffer`: Buffer per memorizzare il messaggio ricevuto.
  - `buffer_len`: Lunghezza del buffer.
  - `addr`: Struttura dell'indirizzo del server.
- **Ritorna**: Il messaggio ricevuto.

---

## Costanti e variabili globali

### Variabili globali
- `int global_var`: Variabile globale generica.
- `int Number_file`: Numero di file presenti sul server.
- `int signal_flag`: Flag per i segnali ricevuti.
- `char ip_address[128]`: Indirizzo IP del server.
- `char server_port[128]`: Porta predefinita del server.

### Costanti
- `MAX_PATH`: Lunghezza massima del percorso di un file.
- `BUFLEN`: Lunghezza massima del buffer per i messaggi.

---
**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------** **FILE CLIENTFILEOPERATION.C** 
## Funzioni per la Gestione dei File

### `int openFile(const char *path)`
- **Scopo:** 
  Apre (o crea, se non esiste) un file in modalità lettura e scrittura, troncandolo se esiste già.
- **Dettagli di implementazione:** 
  - Utilizza la funzione `open()` con le flag `O_RDWR`, `O_CREAT` e `O_TRUNC`.
  - Viene specificato il permesso `0666`, che assegna diritti di lettura e scrittura a tutti gli utenti.
  - L’errore viene gestito impostando `errno` a 0 prima della chiamata, mentre il valore di ritorno (il file descriptor) viene restituito direttamente.

### `int openFileForSending(const char *path)`
- **Scopo:** 
  Apre un file in modalità lettura e scrittura, utilizzato principalmente per operazioni di invio (sending).
- **Dettagli di implementazione:** 
  - Usa `open()` con la flag `O_RDWR` senza creare o troncare il file.
  - Se il file non viene aperto (ritorno -1), il programma stampa un messaggio d’errore e termina l’esecuzione.
  - Garantisce che l’operazione di apertura vada a buon fine prima di procedere.

### `void closeFile(int fd)`
- **Scopo:** 
  Chiude il file associato al file descriptor passato.
- **Dettagli di implementazione:** 
  - Chiama `close(fd)` e, in caso di errore (ritorno -1), termina il programma.
  - Viene impostato `errno` a 0 prima di effettuare la chiamata.

### `int readFile(int fd, char *buf)`
- **Scopo:** 
  Legge il contenuto di un file in un buffer, fino a raggiungere la dimensione definita da `DATA_SIZE`.
- **Dettagli di implementazione:** 
  - Legge in un ciclo, aggiornando il puntatore del buffer e la variabile contatore `r` per tenere traccia dei byte letti.
  - Se `read()` restituisce -1 (errore), il programma termina.
  - Se `read()` restituisce 0 (fine file), la funzione restituisce il numero di byte letti fino a quel momento.

### `void writeFile(int fd, char *buf)`
- **Scopo:** 
  Scrive il contenuto di un buffer in un file.
- **Dettagli di implementazione:** 
  - Calcola la lunghezza del buffer tramite `strlen()` e scrive in un ciclo fino a quando tutti i dati sono stati scritti.
  - Gestisce eventuali errori in scrittura (se `write()` restituisce -1, termina il programma).

### `long getFileSize(const char *path)`
- **Scopo:** 
  Determina e restituisce la dimensione di un file.
- **Dettagli di implementazione:** 
  - Apre il file in modalità "r+" (lettura e scrittura) con `fopen()`.
  - Utilizza `fseek()` per spostarsi alla fine del file e `ftell()` per ottenere la dimensione.
  - Dopo aver ottenuto la dimensione, il file viene riposizionato all'inizio e chiuso.
  - In caso di errori in qualsiasi passaggio, il programma termina.

### `char* readLine(char *buf, FILE *f)`
- **Scopo:** 
  Legge una singola linea da un file in un buffer.
- **Dettagli di implementazione:** 
  - Usa `fgets()` per leggere una linea fino a `MAX_LINE_SIZE` caratteri.
  - Se la linea termina con il carattere newline (`\n`), questo viene sostituito con il terminatore di stringa (`\0`).
  - Se la fine del file è raggiunta, la funzione restituisce `NULL`; in caso di errori (diversi dalla fine del file) termina il programma.

### `bool checkFileName(char* filename, char* path_list)`
- **Scopo:** 
  Verifica se il nome di un file esiste all'interno di una lista di file salvata in un file di testo.
- **Dettagli di implementazione:** 
  - Apre il file contenente la lista dei nomi (specificato da `path_list`) in modalità lettura.
  - Legge riga per riga e confronta il nome specificato (`filename`) con ciascuna riga usando `strncmp()`.
  - Se viene trovato il nome, la funzione restituisce `true`.
  - Libera la memoria allocata per il buffer e chiude il file.

---

## Funzioni per la Gestione della Memoria e delle Directory

### `char** alloc_memory()`
- **Scopo:** 
  Alloca dinamicamente memoria per un array di stringhe.
- **Dettagli di implementazione:** 
  - Utilizza `malloc()` per allocare memoria per un array di puntatori a carattere della lunghezza definita da `BUFLEN`.
  - In caso di fallimento nell’allocazione, viene stampato un messaggio d’errore e il programma termina.

### `char ** getContentDirectory(char *dirpathname)`
- **Scopo:** 
  Ottiene il contenuto di una directory e lo memorizza in un array di stringhe.
- **Dettagli di implementazione:** 
  - Apre la directory con `opendir()`. Se l’apertura fallisce, viene restituito `NULL` con un messaggio di errore.
  - Alloca la memoria per l’array di contenuto tramite la funzione `alloc_memory()`.
  - Itera su ogni elemento della directory con `readdir()` e memorizza il nome di ogni file (o directory) nell’array.
  - Dopo aver completato la lettura, la directory viene chiusa con `closedir()`.
  - Restituisce l’array contenente i nomi degli elementi.

### `int getNumberOfElementsInDir(char *dirpathname)`
- **Scopo:** 
  Conta il numero di elementi presenti all’interno di una directory.
- **Dettagli di implementazione:** 
  - Apre la directory con `opendir()`.
  - Itera su ogni elemento con `readdir()` incrementando un contatore.
  - Chiude la directory con `closedir()` e restituisce il conteggio.
  - Se l’apertura o la chiusura della directory falliscono, il programma termina.

### `bool checkFileInDirectory(char *filename, char **dircontent, int dirsize)`
- **Scopo:** 
  Verifica se un file specifico è presente in un array di contenuti di una directory.
- **Dettagli di implementazione:** 
  - Itera attraverso l’array `dircontent` (dimensione `dirsize`) confrontando ogni elemento con il nome fornito (`filename`) mediante `strcmp()`.
  - Se viene trovata una corrispondenza, restituisce `true`, altrimenti `false`.

---

## Funzioni per la Costruzione dei Percorsi

### `char * obtain_path(char* filepath, char* token, char* cmd)`
- **Scopo:** 
  Costruisce un percorso completo basato sul comando ricevuto (ad es. `get`, `put` o `list`) e su un token (che rappresenta il nome del file).
- **Dettagli di implementazione:** 
  - Recupera la directory corrente con `getcwd()`.
  - A seconda del comando (`get`, `put` o `list`), concatena il percorso con la directory appropriata (`Download/`, `Upload/` o la directory radice).
  - Infine, concatena il token al percorso ottenuto e restituisce il percorso completo.

### `void obtain_path2(char *filepath, char *cmd, char *assolute_path)`
- **Scopo:** 
  Funzione simile a `obtain_path` ma costruisce il percorso completo e lo copia in un buffer esterno (`assolute_path`).
- **Dettagli di implementazione:** 
  - Ottiene la directory corrente con `getcwd()`.
  - Concatena la directory corretta in base al comando (`get`, `put` o `list`).
  - Copia il percorso risultante nel buffer `assolute_path`, garantendo così la disponibilità del percorso assoluto.

---

## Funzione per la Cancellazione di File

### `void delete_file(const char *directory, const char *filename)`
- **Scopo:** 
  Elimina un file specificato, costruendo dinamicamente il percorso completo.
- **Dettagli di implementazione:** 
  - Calcola la lunghezza della directory e del nome del file.
  - Verifica se il percorso della directory termina con uno slash (`/`) e, se necessario, lo aggiunge al percorso completo.
  - Alloca la memoria per contenere il percorso completo e lo costruisce usando `sprintf()`.
  - Utilizza `remove()` per cancellare il file. Se l’operazione ha successo, stampa un messaggio di conferma; in caso di fallimento, stampa un messaggio d’errore.
  - Libera la memoria allocata per il percorso completo.
  
**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE CLIENTSYNCMSG.C**
## Funzioni Implementate

### 1. `void setRcvTimeout(int sock, int timeout_ms);`
- **Scopo:** 
  Impostare un timeout per le operazioni di ricezione sul socket specificato. 
- **Dettagli:** 
  Questa funzione (la cui implementazione è presumibilmente definita altrove) configura il socket `sock` in modo che le chiamate a `recvfrom()` o altre funzioni di I/O non blocchino indefinitamente, ma restituiscano un errore se il tempo di attesa supera `timeout_ms` millisecondi. 
- **Utilizzo nel modulo:** 
  Viene invocata all'inizio della funzione `rcvMsg_sync()` per garantire che la ricezione di un messaggio non rimanga bloccata oltre il tempo desiderato, rendendo l’operazione sincrona e resiliente in presenza di ritardi o perdite di pacchetti.

---

### 2. `char* rcvMsg_sync(int sock, char *rcv, unsigned int size, struct sockaddr_in *servaddr)`
- **Scopo:** 
  Ricevere in maniera sincrona un messaggio tramite un socket, implementando una logica di handshake per la sincronizzazione e la gestione degli errori di trasmissione.
- **Parametri:** 
  - `sock`: il file descriptor del socket da cui ricevere i dati.
  - `rcv`: buffer in cui verrà copiato il messaggio ricevuto.
  - `size`: dimensione del buffer e del messaggio.
  - `servaddr`: puntatore a una struttura `sockaddr_in` che conterrà le informazioni dell'indirizzo del mittente.
- **Logica di Funzionamento:**
  1. **Inizializzazione e Timeout:** 
     - La funzione definisce una variabile booleana `SYN` inizializzata a `false` (0) per tracciare se è stato ricevuto un messaggio di sincronizzazione.
     - Vengono inizializzati due parametri di tipo `socklen_t`: uno (`addr_len`) per la lunghezza generica di un indirizzo e l’altro (`len`) basato sulla struttura specifica `servaddr`.
     - Viene chiamata `setRcvTimeout(sock, RCV_MSG_TIMEOUT)` per impostare un timeout alla ricezione, utilizzando una costante `RCV_MSG_TIMEOUT` definita in "macros.h".

  2. **Ciclo di Ricezione:** 
     - La funzione entra in un ciclo infinito (`for(;;)`) per gestire in maniera iterativa i pacchetti ricevuti.
     - Viene dichiarato un array locale `pack_rcv` di dimensione `size` che fungerà da buffer temporaneo per ogni pacchetto in arrivo.
     - La variabile `errno` viene azzerata prima di chiamare `recvfrom()` per una corretta gestione degli errori.
  
  3. **Gestione degli Errori di Ricezione:** 
     - Se `recvfrom()` restituisce un valore negativo (indicando un errore), si controlla se l’errore è dovuto a un timeout (EAGAIN o EWOULDBLOCK). 
       - Se il flag `SYN` è stato impostato a `true`, significa che almeno un messaggio di sincronizzazione (SYN) era già stato ricevuto; in questo caso, la funzione restituisce il contenuto del buffer `rcv`.
       - Se non è stato ricevuto nessun SYN, la funzione restituisce `NULL` per indicare che non è stato ricevuto alcun messaggio valido.
     - Per altri errori, la funzione restituisce immediatamente `NULL`.

  4. **Controllo dei Messaggi di Errore:** 
     - Se il pacchetto ricevuto corrisponde a stringhe specifiche ("Errore 4" o "Errore 5"), il messaggio viene copiato nel buffer `rcv` tramite `strcpy()` e viene restituito, interrompendo ulteriori elaborazioni.

  5. **Gestione del Messaggio di Sincronizzazione (SYN):** 
     - Se il pacchetto contiene la stringa "SYN", il protocollo di sincronizzazione viene attivato:
       - Viene preparato un messaggio "SYNACK" per confermare la ricezione del SYN.
       - Viene calcolato un valore casuale tramite `random()/RAND_MAX`. 
       - Il flag `SYN` viene impostato a `true` per segnalare la ricezione del messaggio di sincronizzazione.
       - Se il valore casuale è inferiore a `(1 - P)` (dove `P` è una probabilità definita in "macros.h"), il messaggio "SYNACK" viene inviato tramite `sendto()`. Se l'invio fallisce, il programma stampa un errore e termina.

  6. **Gestione dei Messaggi Normali:** 
     - Se il pacchetto ricevuto non corrisponde né a messaggi di errore né a "SYN", il messaggio viene copiato nel buffer `rcv`.
     - Viene preparato un messaggio "ACK" per confermare la ricezione.
     - Analogamente al caso SYN, viene generato un valore casuale e, se questo soddisfa la condizione `(ran < (1 - P))`, il messaggio "ACK" viene inviato utilizzando `sendto()`.
  
  7. **Ciclo Continuo:** 
     - Il ciclo continua fino a quando non si verifica una condizione di uscita (timeout senza SYN, ricezione di un messaggio di errore, o completamento di una trasmissione corretta).
  
- **Ritorno:** 
  - Se viene ricevuto un messaggio valido, questo viene copiato nel buffer `rcv` e la funzione restituisce il puntatore a `rcv`.
  - In caso di errori o di timeout senza aver ricevuto alcun SYN, la funzione restituisce `NULL`.

**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE DECODE64.C**
## Funzioni Relative all'Alfabeto Base64

### 1. `void initialize_base64_alphabet_decode(int shift)`
- **Scopo:** 
  Inizializzare l'alfabeto Base64 con uno shift personalizzato.
- **Funzionamento:** 
  - Chiama la funzione `generate_shifted_base64_alphabet` (definita altrove) passando l'alfabeto Base64 standard e il valore di shift.
  - Se la generazione fallisce (cioè, se il risultato è `NULL`), stampa un messaggio d'errore e termina il programma.
- **Utilizzo:** 
  Prima di decodificare un file Base64, l'alfabeto deve essere inizializzato per interpretare correttamente i caratteri.

### 2. `int base64_char_index(char c)`
- **Scopo:** 
  Restituire l'indice del carattere `c` all'interno dell'alfabeto Base64 shiftato.
- **Funzionamento:** 
  - Utilizza `strchr` per trovare la posizione del carattere `c` nell'alfabeto memorizzato in `base64_chars`.
  - Se il carattere non viene trovato, restituisce `-1`; altrimenti, restituisce la differenza tra la posizione trovata e l'inizio dell'alfabeto.
- **Utilizzo:** 
  Questa funzione è fondamentale per convertire ogni carattere codificato in un indice numerico per la decodifica.

---

## Funzioni di Decodifica Base64

### 3. `void base64_decode(FILE *input, FILE *output)`
- **Scopo:** 
  Leggere da un file codificato in Base64 e scrivere il contenuto decodificato in un altro file.
- **Funzionamento:** 
  - Viene utilizzato un buffer di input (dimensione `INPUT_BUFFER_SIZE`) per leggere porzioni del file codificato.
  - Per ogni blocco letto, il codice processa i dati in gruppi di 4 caratteri (dimensione definita da `DECODE_BLOCK_SIZE`):
    - Per ogni gruppo, vengono ottenuti gli indici di ciascun carattere utilizzando `base64_char_index`.
    - Se un carattere non appartiene all'alfabeto Base64 (eccetto il carattere '=' usato per il padding), viene generato un errore.
    - I valori ottenuti vengono utilizzati per ricostruire i byte originali, memorizzati in un buffer di output.
  - Il buffer decodificato viene poi scritto sul file di output.
- **Utilizzo:** 
  Questa funzione è il cuore della decodifica e consente di trasformare un file codificato in Base64 (con eventuale padding) nel file originale.

---

## Funzioni per la Gestione dei File

### 4. `int is_regular_file_decode(const char *path)`
- **Scopo:** 
  Verificare se il percorso specificato punta a un file regolare.
- **Funzionamento:** 
  - Utilizza `stat()` per ottenere le informazioni sul file e verifica con `S_ISREG` se il percorso rappresenta un file regolare.
  - Restituisce `1` se il file è valido, altrimenti `0`.
- **Utilizzo:** 
  Funzione di controllo per assicurarsi che l'input fornito per la decodifica sia effettivamente un file.

### 5. `int decode_base64_file(const char *input_filename, const char *output_filename_base)`
- **Scopo:** 
  Decodificare un file codificato in Base64 e scrivere il file decodificato con un nome di output appropriato.
- **Funzionamento:** 
  - Apre il file di input in modalità lettura.
  - Legge la prima riga per ottenere l'estensione del file originale (attesa nel formato "ext:xxx").
  - Costruisce il nome del file di output:
    - Se l'estensione non è già presente nel nome di output base, viene aggiunta.
  - Apre il file di output in modalità scrittura binaria.
  - Chiama `base64_decode` per eseguire la decodifica vera e propria.
  - Chiude entrambi i file e, se la modalità performance non è attiva, stampa un messaggio di conferma.
- **Utilizzo:** 
  Funzione principale per l'interfaccia utente per la decodifica di un file Base64.

### 6. `int decoder_handler(const char *input_directory, const char *output_directory, const char *filename)`
- **Scopo:** 
  Gestire l'intero processo di decodifica, integrando i controlli di validità del file e la costruzione dei percorsi.
- **Funzionamento:** 
  - Costruisce il percorso completo del file di input combinando la directory di input e il nome del file.
  - Verifica che il file di input sia un file regolare chiamando `is_regular_file_decode`.
  - Costruisce il percorso base per il file di output combinando la directory di output e il nome del file.
  - Rimuove (se presente) l'estensione `.txt` dal nome base dell'output per evitare duplicazioni di estensione.
  - Invoca `decode_base64_file` per eseguire la decodifica.
- **Utilizzo:** 
  Funzione di interfaccia che coordina il processo di decodifica a partire dai percorsi delle directory e dal nome del file, rendendo il modulo facilmente integrabile in  un'applicazione più ampia.

**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE ENCODE64.C**
## Funzioni Relative all'Alfabeto Base64

### 1. `void initialize_base64_alphabet_encode(int shift)`
- **Scopo:** 
  Inizializzare l'alfabeto Base64 utilizzato per la codifica, applicando uno shift personalizzato.
- **Funzionamento:** 
  - Chiama la funzione `generate_shifted_base64_alphabet` passando l'alfabeto Base64 standard (`"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"`) e il valore di shift.
  - Se la generazione fallisce (ovvero, se il risultato è `NULL`), stampa un messaggio d'errore e termina il programma.
- **Utilizzo:** 
  Questa funzione deve essere chiamata prima di effettuare qualsiasi operazione di codifica in modo che l'alfabeto Base64 shiftato sia correttamente inizializzato.

---

## Funzioni di Codifica Base64

### 2. `void base64_encode(FILE *input, FILE *output)`
- **Scopo:** 
  Leggere dati da un file di input e codificarli in formato Base64, scrivendo il risultato in un file di output.
- **Funzionamento:** 
  - Legge dati dal file di input in blocchi di dimensione definita da `ENCODE_BLOCK_SIZE`.
  - Per ogni blocco, divide i dati in gruppi di 3 byte.
  - Converte ciascun gruppo di 3 byte in 4 caratteri Base64:
    - Utilizza operazioni di bit shifting per ottenere gli indici dei caratteri, usando l'alfabeto shiftato memorizzato in `BASE64_CHARS`.
    - Se non sono presenti 3 byte (per esempio, alla fine del file), aggiunge il carattere di padding '='.
  - Scrive il contenuto codificato nel file di output.
- **Utilizzo:** 
  Viene utilizzata per convertire i file binari o di testo in una rappresentazione Base64, utile per trasmissioni o archiviazioni in formato testuale.

---

## Funzioni per la Gestione dei File

### 3. `int is_regular_file_encode(const char *path)`
- **Scopo:** 
  Verificare se il percorso specificato punta a un file regolare.
- **Funzionamento:** 
  - Utilizza la funzione `stat()` per ottenere informazioni sul file e controlla, tramite la macro `S_ISREG`, se il file è regolare.
  - Restituisce `1` (vero) se il file è valido, altrimenti `0`.
- **Utilizzo:** 
  Questa funzione è utilizzata come controllo preliminare per assicurarsi che l'input passato per la codifica sia effettivamente un file.

### 4. `int convert_file_to_base64(const char *input_filepath, const char *output_filepath)`
- **Scopo:** 
  Codificare un file in formato Base64 e salvare il risultato in un file di output.
- **Funzionamento:** 
  - Apre il file di input in modalità binaria (`"rb"`) e il file di output in modalità scrittura (`"w"`).
  - Se uno dei file non viene aperto, stampa un messaggio di errore e chiude i file eventualmente aperti.
  - All'inizio del file di output, scrive l'estensione del file originale nel formato `ext:xxx`, dove `xxx` rappresenta l'estensione del file.
  - Chiama la funzione `base64_encode` per eseguire la codifica vera e propria.
  - Chiude entrambi i file e, se non è attiva la modalità performance, stampa un messaggio di conferma.
- **Utilizzo:** 
  Funzione centrale che coordina l'intero processo di codifica, dalla gestione dei file all'applicazione dell'algoritmo Base64.

---

## Funzione di Gestione dell'Encoder

### 5. `int encoder_handler(const char *input_directory, const char *output_directory, const char *filename)`
- **Scopo:** 
  Gestire il processo di codifica per un file specifico, integrando la costruzione dei percorsi e la verifica dei file.
- **Funzionamento:** 
  - Costruisce il percorso completo per il file di input combinando la directory di input e il nome del file.
  - Costruisce il percorso per il file di output, aggiungendo l'estensione `.txt`.
  - Verifica che il file di input sia un file regolare chiamando `is_regular_file_encode`.
  - Se il file non è valido, stampa un messaggio d'errore e restituisce `-1`.
  - Invoca `convert_file_to_base64` per eseguire la codifica.
  - Se la codifica avviene correttamente, stampa un messaggio di conferma (a meno che non sia attiva la modalità performance) e restituisce `0`.
- **Utilizzo:** 
  Questa funzione funge da interfaccia principale per l'utente o per altre parti del sistema, permettendo di codificare un file in Base64 passando le directory e il nome del file.

**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE RELIABLEUDP.C**
## 1. Calcolo del Checksum

### `unsigned long calculateChecksum(const char *data, size_t length)`
- **Scopo:** 
  Calcola il checksum dei dati forniti utilizzando l'algoritmo CRC32.
  
- **Funzionamento:** 
  - Viene chiamata la funzione `crc32()` (proveniente da una libreria esterna, ad esempio zlib) per calcolare il checksum sul blocco di dati indicato.
  - Il valore iniziale viene impostato a `0L` e i dati sono passati come array di byte.
  - Se la modalità "performance" non è attiva (cioè, se `PERFORMANCE` è falso), viene stampato il checksum calcolato.
  - Il checksum risultante viene poi restituito.

- **Utilizzo:** 
  Questa funzione viene usata per garantire l'integrità dei dati, permettendo di verificare che i dati trasmessi o ricevuti non siano stati alterati.

---

## 2. Congestion Control ed Estimazione del Timeout

### `long unsigned estimateTimeout(long unsigned *EstimatedRTT, long unsigned *DevRTT, long unsigned SampleRTT)`
- **Scopo:** 
  Calcolare il timeout dinamico per la trasmissione basato sulla stima del Round-Trip Time (RTT) e sulla sua deviazione.
  
- **Funzionamento:** 
  - **Aggiornamento di EstimatedRTT:** 
    L'EstimatedRTT viene aggiornato come media pesata, dove il 87.5% del valore precedente è mantenuto e il 12.5% è dato dal nuovo SampleRTT:

    *EstimatedRTT = 0.875 * (*EstimatedRTT) + 0.125 * SampleRTT;
  
  - **Aggiornamento di DevRTT:** 
    La deviazione (DevRTT) viene anch'essa aggiornata come media pesata del valore precedente e della differenza assoluta tra il SampleRTT e l'EstimatedRTT:
    
    *DevRTT = 0.75 * (*DevRTT) + 0.25 * (SampleRTT > *EstimatedRTT ? SampleRTT - *EstimatedRTT : *EstimatedRTT - SampleRTT);
  
  - **Calcolo del Timeout Interval:** 
    Se non è abilitato un RTT fisso (`FIXEDRTT` falso), il timeout viene calcolato come:

    timeoutInterval = *EstimatedRTT + 4 * (*DevRTT);

    Altrimenti, se `FIXEDRTT` è vero, viene restituito un valore costante definito da `RTT` (ad esempio, 1 secondo).
  
- **Utilizzo:** 
  Questa funzione è essenziale in sistemi di trasmissione affidabili, dove il timeout per le operazioni di rete deve adattarsi dinamicamente alle variazioni delle condizioni di rete, riducendo la possibilità di timeout prematuri o troppo lunghi.

---

## 3. Generazione di un Alfabeto Base64 Shiftato

### `char *generate_shifted_base64_alphabet(const char *base64_chars, int shift)`
- **Scopo:** 
  Generare un nuovo alfabeto Base64 applicando uno shift ciclico all'alfabeto standard.

- **Funzionamento:** 
  - **Verifica dell'Input:** 
    Se il puntatore all'alfabeto originale (`base64_chars`) è `NULL`, la funzione stampa un messaggio di errore e restituisce `NULL`.
  
  - **Allocazione della Memoria:** 
    Viene allocato un buffer per contenere l'alfabeto shiftato. La dimensione del buffer è la lunghezza dell'alfabeto originale più uno per il carattere di terminazione (`'\0'`).
  
  - **Normalizzazione dello Shift:** 
    Lo shift viene normalizzato per evitare valori fuori dall'intervallo. Se lo shift è negativo, viene convertito in un corrispondente valore positivo; se è positivo, viene ridotto al modulo della lunghezza dell'alfabeto.
  
  - **Applicazione dello Shift Ciclico:** 
    Per ogni posizione nell'alfabeto originale, il nuovo carattere viene scelto spostando l'indice di `shift` posizioni, utilizzando l'operatore modulo per garantire la ciclicità:

    shifted_alphabet[i] = base64_chars[(i + shift) % length];
  
  - **Terminazione della Stringa:** 
    Viene aggiunto il terminatore `'\0'` alla fine della stringa shiftata.
  
  - **Restituzione:** 
    La funzione restituisce il puntatore all'alfabeto shiftato, che può essere utilizzato nelle operazioni di codifica/decodifica Base64.

- **Utilizzo:** 
  La funzione consente di applicare una trasformazione all'alfabeto Base64 standard, permettendo di creare varianti personalizzate per la codifica e la decodifica dei dati, aumentando la flessibilità e la sicurezza nelle operazioni di crittografia o steganografia.

**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE CLIENTRECEIVER.C**
## 1. Inizializzazione del Buffer di Ricezione

### `void rcvBufInit(struct rcv_info *rcv_inf)`
- **Scopo:** 
  Inizializza il buffer di ricezione impostando ogni posizione a `NULL`.
- **Funzionamento:** 
  - Itera attraverso il buffer (dimensione definita dalla costante `N`) e assegna `NULL` a ciascun elemento.
- **Utilizzo:** 
  Questa funzione prepara la struttura dati che memorizzerà i pacchetti ricevuti, garantendo che all'avvio nessuna posizione contenga dati residui.

---

## 2. Calcolo della Differenza di Tempo

### `long calculateTimeDifference(struct timeval start, struct timeval end)`
- **Scopo:** 
  Calcolare la differenza in microsecondi tra due istanti temporali.
- **Funzionamento:** 
  - Utilizza i campi `tv_sec` e `tv_usec` delle strutture `timeval` per calcolare il tempo trascorso.
  - La formula computa la differenza in microsecondi: 
    (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec)
- **Utilizzo:** 
  È usata per misurare il SampleRTT (Round Trip Time) di ogni pacchetto ricevuto, utile per l'aggiornamento dinamico del timeout.

---

## 3. Invio degli Acknowledgement (ACK)

### `void sendAcknowledgement(int sock_fd, int seq_num, struct sockaddr_in si_other)`
- **Scopo:** 
  Invia un ACK per il pacchetto con il numero di sequenza specificato.
- **Funzionamento:** 
  - Genera un valore casuale per simulare la perdita di ACK in base alla probabilità definita da `P`.
  - Se il valore casuale è inferiore a `1 - P`, prepara un buffer contenente il numero di sequenza convertito in stringa.
  - Utilizza `sendto()` per inviare l'ACK al mittente, gestendo eventuali errori.
  - Se la modalità non è "performance", stampa un messaggio di conferma.
- **Utilizzo:** 
  Garantisce che il mittente riceva una conferma per ogni pacchetto correttamente ricevuto, permettendo la gestione della finestra scorrevole e il ritrasmissione dei pacchetti mancanti.

---

## 4. Impostazione del Timeout per la Ricezione

### `void setTimeoutRcv(int sockfd, long timeout)`
- **Scopo:** 
  Configurare il timeout per le operazioni di ricezione sul socket.
- **Funzionamento:** 
  - Crea una struttura `timeval` e imposta `tv_sec` e `tv_usec` in base al valore del timeout in microsecondi.
  - Usa `setsockopt()` per impostare l'opzione `SO_RCVTIMEO` sul socket.
  - Gestisce eventuali errori nella configurazione del timeout.
- **Utilizzo:** 
  Assicura che le chiamate a `recvfrom()` non blocchino indefinitamente, permettendo di rilevare timeout e gestire la ritrasmissione in caso di mancata ricezione di pacchetti.

---

## 5. Parsing del Pacchetto Ricevuto

### `unsigned int parseLine(char *buffer, char *data)`
- **Scopo:** 
  Estrae il numero di sequenza e i dati da un pacchetto ricevuto.
- **Funzionamento:** 
  - Utilizza `strtoul()` per convertire la parte iniziale della stringa (contenente il numero di sequenza) in un valore numerico.
  - Verifica che dopo il numero ci sia uno spazio; altrimenti, genera un errore.
  - Incrementa il puntatore e copia la parte restante della stringa (i dati) nel buffer `data`.
  - Restituisce il numero di sequenza.
- **Utilizzo:** 
  Permette di separare l'intestazione del pacchetto (numero di sequenza) dal contenuto dati, fondamentale per ordinare e scrivere correttamente i pacchetti ricevuti.

---

## 6. Ricezione del File

### `void rcvFile(int sock_fd, int fd, long size, struct sockaddr_in si_other)`
- **Scopo:** 
  Gestire la ricezione di un file intero, utilizzando una finestra scorrevole e meccanismi di controllo per garantire l'ordinamento e l'integrità dei dati.
- **Funzionamento:**
  - **Inizializzazione:** 
    - Alloca e inizializza una struttura `rcv_info` contenente il buffer di ricezione, il file descriptor di output, le dimensioni del file e le informazioni sul mittente.
    - Imposta i valori iniziali per la gestione della finestra (ad esempio, `rcv_base`) e i parametri per la stima del timeout (EstimatedRTT, DevRTT e TIMEOUT).
    - Imposta il timeout sul socket tramite `setTimeoutRcv()`.
  - **Ciclo Principale di Ricezione:** 
    - Avvia un ciclo in cui si riceve un pacchetto tramite `recvfrom()`. Se la ricezione scade (timeout) o si verificano errori specifici (come ECONNREFUSED), gestisce la condizione riprovando o terminando la ricezione.
    - Inizia il timer per il calcolo del SampleRTT.
    - Alloca dinamicamente una struttura `rcv_info_pack` per memorizzare il pacchetto e utilizza `parseLine()` per estrarre il numero di sequenza e i dati.
    - Calcola il checksum dei dati ricevuti per verificarne l'integrità.
    - Aggiorna il numero di sequenza da inviare in eventuali ritrasmissioni.
    - Stampa, se non in modalità performance, informazioni di debug sul pacchetto e sui parametri RTT.
  - **Gestione della Finestra Scorrevole (Sliding Window):** 
    - La logica distingue vari casi in base alla posizione del pacchetto nella finestra:
      1. **Finestra Non Spezzata:** 
         - Se il pacchetto è nella finestra e non è già stato ricevuto, viene salvato nel buffer.
         - Se il pacchetto è il prossimo atteso (`rcv_base`), scrive i dati sul file e sposta la finestra in avanti.
      2. **Finestra Spezzata (Pacchetto alla Fine):** 
         - Simile al caso precedente, ma gestisce i pacchetti che si trovano nella parte alta dell'indice, con il wrap-around del buffer.
      3. **Pacchetto Fuori Finestra:** 
         - Se il pacchetto è fuori dalla finestra attuale, viene comunque inviato un ACK per confermare la ricezione, senza modificarne l'ordine.
  - **Aggiornamento del Timeout:** 
    - Dopo la ricezione di ogni pacchetto, il SampleRTT viene calcolato e usato per aggiornare il timeout dinamico tramite la funzione `estimateTimeout()`.
  - **Terminazione della Ricezione:** 
    - Se il numero di sequenza del pacchetto ricevuto supera o raggiunge il numero totale di pacchetti attesi (`N`), viene inviato un ACK finale, il socket viene chiuso e, se il file è stato completamente ricevuto, la funzione termina con successo.
- **Utilizzo:** 
  Questa funzione coordina l'intero processo di ricezione, integrando la logica del protocollo di trasmissione (ACK, timeout, sliding window) e garantendo che i dati vengano scritti correttamente su file in ordine.

**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE CLIENTSENDER.C**
## Strutture e Variabili Globali

- **`nodo* phead`** 
  Puntatore alla testa della lista collegata (queue) utilizzata per tenere traccia dei pacchetti inviati e gestire i timeout.

---

## Funzioni di Allocazione e Gestione della Lista

### `nodo* alloc_node(void)`
- **Scopo:** 
  Alloca dinamicamente un nuovo nodo della lista.
- **Funzionamento:** 
  - Utilizza `malloc()` per allocare memoria per una struttura di tipo `nodo`.
  - Se l'allocazione fallisce, stampa un messaggio d'errore e termina il programma.
- **Utilizzo:** 
  Utilizzata per creare un nuovo nodo da inserire nella lista di timeout.

---

### `unsigned long calculateSampleRTT(struct timeval *start, struct timeval *end)`
- **Scopo:** 
  Calcolare il SampleRTT (Round Trip Time) in microsecondi tra due istanti temporali.
- **Funzionamento:** 
  - Calcola la differenza tra i secondi e microsecondi dei due timestamp.
  - Gestisce il caso in cui i microsecondi finali siano inferiori a quelli iniziali.
  - Se il valore calcolato supera un certo limite (ad esempio 1.000.000 microsecondi), lo limita a un valore predefinito (500 microsecondi in questo caso).
- **Utilizzo:** 
  Fondamentale per adattare dinamicamente il timeout per la trasmissione in base alle condizioni di rete.

---

### `nodo* insert_in_queue(nodo* head, unsigned int seq_num)`
- **Scopo:** 
  Inserire un nuovo nodo (con un numero di sequenza) alla fine della lista di timeout.
- **Funzionamento:** 
  - Alloca un nuovo nodo con `alloc_node()`.
  - Inizializza il campo `seq_num` e segna il nodo come non completato (`finished = 0`).
  - Registra il tempo corrente nel campo `timer` del nodo usando `gettimeofday()`.
  - Se la lista è vuota, restituisce il nuovo nodo come testa; altrimenti, itera fino alla fine della lista e collega il nuovo nodo.
- **Utilizzo:** 
  Mantiene un registro dei pacchetti inviati e dei loro tempi di invio per poter gestire i timeout e le eventuali ritrasmissioni.

---

### `nodo* delete_node_in_head(nodo* head)`
- **Scopo:** 
  Rimuovere il nodo in testa alla lista.
- **Funzionamento:** 
  - Se la lista non è vuota, salva il nodo di testa in una variabile temporanea, aggiorna la testa e libera la memoria del nodo rimosso.
- **Utilizzo:** 
  Usata per rimuovere il pacchetto che è stato già ritrasmesso o confermato, spostando la finestra scorrevole.

---

### `nodo* remove_nodo(nodo* head, unsigned int seq_num, struct timeval* t)`
- **Scopo:** 
  Rimuovere dalla lista il nodo che corrisponde a un dato numero di sequenza e restituire il suo timestamp.
- **Funzionamento:** 
  - Scorre la lista per trovare il nodo con il numero di sequenza `seq_num`.
  - Se trovato, aggiorna i puntatori per rimuoverlo dalla lista, copia il valore del timer nel parametro `t` e libera la memoria.
- **Utilizzo:** 
  Permette di recuperare il tempo di invio del pacchetto che viene confermato tramite ACK.

---

## Funzioni per la Gestione del Buffer di Invio

### `void sndBufInit(struct snd_thread_info* snd)`
- **Scopo:** 
  Inizializzare l'array di buffer di invio impostando ogni elemento a `NULL`.
- **Funzionamento:** 
  - Itera da 0 a `N - 1` (dove `N` è la dimensione del buffer) e assegna `NULL` ad ogni posizione.
- **Utilizzo:** 
  Prepara il buffer di invio per l'inserimento dei pacchetti da trasmettere.

---

### `char* addSeqNum(char* data, unsigned int seqnum)`
- **Scopo:** 
  Formattare una stringa contenente il numero di sequenza e i dati del pacchetto.
- **Funzionamento:** 
  - Determina la dimensione necessaria per il buffer in base alla lunghezza dei dati e al numero di sequenza.
  - Alloca memoria per la nuova stringa.
  - Utilizza `snprintf()` per formattare il numero di sequenza (e, se applicabile, i dati) in una stringa.
- **Utilizzo:** 
  Garantisce che ogni pacchetto inviato contenga un'intestazione con il numero di sequenza, essenziale per il corretto ordinamento e conferma dei pacchetti.

---

## Thread per l'Invio e Gestione dei Timeout

### `void* thread_send_job(void* p)`
- **Scopo:** 
  Leggere dati dal file e inviarli in pacchetti UDP, gestendo la finestra scorrevole.
- **Funzionamento:** 
  - Inizializza la lista di timeout e accoda i pacchetti inviati nella struttura `snd_thread_info`.
  - Legge dal file tramite `readFile()` e prepara il pacchetto con il numero di sequenza.
  - Se la finestra scorrevole è piena, attende (usando `usleep()`) fino a quando non c'è spazio.
  - Invio del pacchetto tramite `sendto()` (incluso il calcolo del checksum e la formattazione con `addSeqNum()`).
  - Aggiorna la variabile `next_tosend` per la posizione successiva e inserisce il pacchetto nella coda per il timeout.
- **Utilizzo:** 
  È il thread responsabile dell'invio dei pacchetti e della gestione della coda dei pacchetti in attesa di ACK.

---

### `void* thread_timeout_job(void* p)`
- **Scopo:** 
  Gestire i timeout dei pacchetti inviati e ritrasmettere quelli per i quali il timeout è scaduto.
- **Funzionamento:** 
  - Controlla continuamente la lista di timeout (`phead`).
  - Confronta il tempo corrente con il timer del pacchetto in testa alla lista.
  - Se il timeout è scaduto, incrementa il timeout, segnala il pacchetto come ritrasmesso e lo invia nuovamente.
  - Rimuove il pacchetto ritrasmesso dalla testa della lista e lo reinserisce in coda.
- **Utilizzo:** 
  Garantisce la ritrasmissione dei pacchetti che non hanno ricevuto ACK in tempo, adattando dinamicamente il timeout.

---

### `int parseAck(char* ack)`
- **Scopo:** 
  Interpretare il valore numerico contenuto nella stringa ACK ricevuta.
- **Funzionamento:** 
  - Usa `strtoul()` per convertire la stringa in un intero.
  - Verifica eventuali errori di conversione.
- **Utilizzo:**
  Fondamentale per interpretare gli ACK ricevuti e aggiornare la finestra scorrevole.

---

### `void setRcvTimeout(int sockfd, long unsigned timeout)`
- **Scopo:** 
  Impostare il timeout per le operazioni di ricezione sul socket.
- **Funzionamento:** 
  - Configura una struttura `timeval` in base al timeout specificato (dividendo in secondi e microsecondi).
  - Applica il timeout al socket usando `setsockopt()` con l'opzione `SO_RCVTIMEO`.
- **Utilizzo:** 
  Assicura che le operazioni di ricezione non blocchino indefinitamente e facilitano la gestione dei ritardi nella rete.

---

### `void final_job(struct snd_thread_info* snd_ti, struct snd_pack* pkt, char* ACK_torcv)`
- **Scopo:** 
  Gestire la fase finale dell'invio, in cui viene ritrasmesso l'ultimo pacchetto e si attende il relativo ACK finale.
- **Funzionamento:** 
  - Invia il pacchetto finale, marcandolo con un numero di sequenza fittizio.
  - Aggiorna il timeout e attende di ricevere l'ACK finale.
  - Cancella il thread di timeout una volta ricevuto l'ACK finale e libera le risorse.
- **Utilizzo:** 
  Chiude in modo ordinato il processo di invio, garantendo che l'ultima parte del file sia stata trasmessa correttamente.

---

### `void thread_ack_job(void* p)`
- **Scopo:** 
  Ricevere e gestire gli ACK dai pacchetti inviati, aggiornando la finestra scorrevole e il timeout.
- **Funzionamento:** 
  - Esegue un ciclo continuo in cui attende la ricezione di ACK utilizzando `recvfrom()`.
  - Quando un ACK viene ricevuto, lo analizza con `parseAck()` e, se valido, aggiorna il campo `acked` del pacchetto corrispondente.
  - Calcola il SampleRTT e aggiorna il timeout usando la funzione di stima.
  - Se il pacchetto corrispondente all'ACK è il più vecchio (send_base), sposta la finestra scorrevole liberando la memoria dei pacchetti confermati.
- **Utilizzo:** 
  Responsabile del controllo della conferma di ricezione dei pacchetti, essenziale per mantenere la sincronizzazione tra il mittente e il ricevitore.

---

## Funzione Principale di Invio File

### `void sendFile(int sockfd, int fd, struct sockaddr_in si_other)`
- **Scopo:** 
  Coordinare l'intero processo di invio del file tramite UDP.
- **Funzionamento:** 
  - Alloca e inizializza una struttura `snd_thread_info` che contiene tutte le informazioni necessarie per l'invio (buffer, socket, timeout, indirizzo remoto, ecc.).
  - Inizializza il buffer di invio tramite `sndBufInit()`.
  - Imposta i valori iniziali per `send_base`, `next_tosend`, e i parametri per il timeout.
  - Crea due thread: uno per l'invio dei pacchetti (`thread_send_job`) e uno per la gestione dei timeout (`thread_timeout_job`).
  - Infine, chiama la funzione `thread_ack_job()` nel thread principale per gestire la ricezione degli ACK.
- **Utilizzo:** 
  Funzione di ingresso per il modulo di invio file, che coordina tutti gli aspetti dell'invio affidabile (trasmissione, ritrasmissione, controllo del timeout e sliding window).

