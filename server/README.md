**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE SERVER.C**
## Struttura del file

## Variabili globali

### Thread
- **`pthread_t input_thread`**: Thread per monitorare l'input dell'utente.
- **`pthread_t threads[MAX_THREADS]`**: Array di thread per gestire le connessioni dei client.

### Stato del server
- **`volatile bool process_on`**: Flag per indicare se il server è attivo.
- **`int client_number`**: Numero di client attualmente connessi.
- **`int client_number_for_port`**: Numero di client connessi per porta.
- **`int thread_no`**: Numero di thread attivi.

### Configurazione
- **`char ip_address[BUFLEN]`**: Indirizzo IP del server.
- **`char server_port[BUFLEN]`**: Porta predefinita del server.

### Connessioni
- **`static int connection_index`**: Indice per tracciare le connessioni.
- **`static int connection_ports[MAX_THREADS]`**: Array delle porte utilizzate dai client.
- **`static int connection_socks[MAX_THREADS]`**: Array dei socket utilizzati dai client.
- **`static time_t connection_time[MAX_THREADS]`**: Timestamp delle connessioni attive.

---

## Strutture dati

### `struct client_info`
- **Descrizione**: Contiene informazioni relative a un client connesso.
- **Campi**:
  - `int sock`: Socket del client.
  - `int port`: Porta del client.
  - `socklen_t fromlen`: Lunghezza della struttura `sockaddr_in`.
  - `struct sockaddr_in from`: Indirizzo del client.
  - `char buf[BUFLEN]`: Buffer per i messaggi ricevuti.
  - `struct sockaddr_in server_addr`: Indirizzo del server.
  - `char *filepath`: Percorso del file associato al client.

---

## Funzioni principali

### `int main(void)`
- **Descrizione**: Punto di ingresso del programma. Configura il server, avvia il thread per monitorare l'input dell'utente e gestisce le richieste dei client.
- **Passaggi principali**:
  1. Richiede l'indirizzo IP e la porta del server.
  2. Verifica la raggiungibilità dell'IP e la validità della porta.
  3. Inizializza l'alfabeto Base64 per codifica e decodifica.
  4. Avvia il thread per monitorare l'input dell'utente.
  5. Gestisce le richieste dei client tramite la funzione `handle_client_requests`.

---

## Funzioni di gestione del server

### `void die(char *s)`
- **Descrizione**: Stampa un messaggio di errore e termina il programma.
- **Parametri**:
  - `s`: Messaggio di errore da stampare.

### `void rebind_socket(struct client_info *clinf)`
- **Descrizione**: Chiude e ricrea una socket per un client.
- **Parametri**:
  - `clinf`: Struttura `client_info` contenente le informazioni del client.

### `void kill_process_on_port(const char *port)`
- **Descrizione**: Termina i processi che utilizzano una porta specifica.
- **Parametri**:
  - `port`: Porta da liberare.

---

## Funzioni di gestione dei client

### `void *handle_request(void *p)`
- **Descrizione**: Gestisce le richieste di un client in un thread separato.
- **Parametri**:
  - `p`: Puntatore alla struttura `client_info` del client.
- **Funzionalità principali**:
  - Riceve comandi dal client.
  - Gestisce i comandi:
    - `list`: Elenca i file disponibili sul server.
    - `get <FileName>`: Invia un file al client.
    - `put <FileName>`: Riceve un file dal client.
    - `delete <FileName>`: Elimina un file sul server.
    - `quit`: Termina la connessione con il client.
  - Ritorna al punto di partenza (`START`) per gestire ulteriori richieste.

### `void send_response(struct client_info *client)`
- **Descrizione**: Invia una risposta al client.
- **Parametri**:
  - `client`: Struttura `client_info` del client.

### `void receive_response(struct client_info *client)`
- **Descrizione**: Riceve una risposta dal client.
- **Parametri**:
  - `client`: Struttura `client_info` del client.

### `void handle_empty_file(struct client_info *client)`
- **Descrizione**: Gestisce il caso in cui un file è vuoto.
- **Parametri**:
  - `client`: Struttura `client_info` del client.

### `void handle_file_not_found(struct client_info *client)`
- **Descrizione**: Gestisce il caso in cui un file non viene trovato.
- **Parametri**:
  - `client`: Struttura `client_info` del client.

### `void handle_exit(struct client_info *client)`
- **Descrizione**: Gestisce la chiusura della connessione con un client.
- **Parametri**:
  - `client`: Struttura `client_info` del client.

---

## Funzioni di supporto

### `int read_file_list(char** filelist, char* token, char* cmd, struct client_info* clinf)`
- **Descrizione**: Legge la lista dei file in una directory e la memorizza in un array.
- **Parametri**:
  - `filelist`: Array in cui memorizzare i nomi dei file.
  - `token`: Token del comando ricevuto.
  - `cmd`: Comando ricevuto.
  - `clinf`: Struttura `client_info` del client.
- **Ritorna**: Numero di file trovati.

### `char* port_validation(char* s)`
- **Descrizione**: Valida e trova una porta disponibile.
- **Parametri**:
  - `s`: Stringa contenente la porta di base.
- **Ritorna**: Porta valida come stringa.

---

## Funzioni per l'input dell'utente

### `void *monitor_user_input()`
- **Descrizione**: Monitora l'input dell'utente per comandi amministrativi.
- **Comandi supportati**:
  - `/shutdown`: Arresta il server.
  - `/delete <FileName>`: Elimina un file dalla directory `server_files`.
  - `/list`: Elenca i file nella directory `server_files`.
  - `/connections`: Mostra le connessioni attive.
  - `/disconnect <port>`: Disconnette un client su una porta specifica.
  - `/help`: Mostra l'elenco dei comandi disponibili.

---

## Funzioni di rete

### `void *handle_client_requests()`
- **Descrizione**: Gestisce le richieste dei client e crea thread per ogni connessione.
- **Funzionalità principali**:
  - Crea e configura il socket principale.
  - Riceve dati dai client.
  - Sincronizza i messaggi con i client utilizzando `sndMsg_sync`.
  - Crea un thread per ogni client connesso.

---
**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**

**FILE SERVERFILEOPERATION.C**
## Funzioni principali

### `int openFile(const char *path)`
- **Descrizione**: Apre un file in modalità lettura/scrittura. Se il file non esiste, lo crea.
- **Parametri**:
  - `path`: Percorso del file da aprire.
- **Ritorna**: Il file descriptor del file aperto, oppure `-1` in caso di errore.
- **Errori gestiti**:
  - Se il file non può essere aperto, viene stampato un messaggio di errore e la funzione restituisce `-1`.

---

### `void closeFile(int fd)`
- **Descrizione**: Chiude un file dato il suo file descriptor.
- **Parametri**:
  - `fd`: File descriptor del file da chiudere.
- **Errori gestiti**:
  - Se la chiusura fallisce, il programma termina con `EXIT_FAILURE`.

---

### `int readFile(int fd, char *buf)`
- **Descrizione**: Legge il contenuto di un file e lo memorizza in un buffer.
- **Parametri**:
  - `fd`: File descriptor del file da leggere.
  - `buf`: Buffer in cui memorizzare i dati letti.
- **Ritorna**: Il numero di byte letti.
- **Errori gestiti**:
  - Se la lettura fallisce, il programma termina con `EXIT_FAILURE`.

---

### `void writeFile(int fd, char *buf)`
- **Descrizione**: Scrive il contenuto di un buffer in un file.
- **Parametri**:
  - `fd`: File descriptor del file in cui scrivere.
  - `buf`: Buffer contenente i dati da scrivere.
- **Errori gestiti**:
  - Se la scrittura fallisce, il programma termina con `EXIT_FAILURE`.

---

### `long getFileSize(const char *path)`
- **Descrizione**: Ottiene la dimensione di un file in byte.
- **Parametri**:
  - `path`: Percorso del file di cui calcolare la dimensione.
- **Ritorna**: La dimensione del file in byte.
- **Errori gestiti**:
  - Se il file non può essere aperto o la dimensione non può essere calcolata, il programma termina con `EXIT_FAILURE`.

---

### `char* obtain_path(char* file_path, char* token, char* cmd)`
- **Descrizione**: Costruisce un percorso completo per un file basandosi sulla directory corrente e su un comando specifico.
- **Parametri**:
  - `file_path`: Buffer in cui memorizzare il percorso completo.
  - `token`: Nome del file o directory.
  - `cmd`: Comando ricevuto (es. `get`, `put`, `list`).
- **Ritorna**: Il percorso completo come stringa.

---

### `char** alloc_memory(size_t size)`
- **Descrizione**: Alloca memoria per un array di stringhe.
- **Parametri**:
  - `size`: Numero di elementi da allocare.
- **Ritorna**: Un puntatore all'array allocato.
- **Errori gestiti**:
  - Se l'allocazione fallisce, il programma termina con `EXIT_FAILURE`.

---

### `void tokenize_string(char *buffer, const char *delimiter, char **tokens)`
- **Descrizione**: Divide una stringa in token utilizzando un delimitatore specifico.
- **Parametri**:
  - `buffer`: Stringa da dividere.
  - `delimiter`: Delimitatore da utilizzare.
  - `tokens`: Array in cui memorizzare i token.

---

### `double print_total_time(struct timeval tv1)`
- **Descrizione**: Calcola e restituisce il tempo totale trascorso in secondi.
- **Parametri**:
  - `tv1`: Tempo iniziale.
- **Ritorna**: Il tempo totale trascorso in secondi.

---

### `char* list_files(const char *dir_name)`
- **Descrizione**: Elenca i file presenti in una directory.
- **Parametri**:
  - `dir_name`: Nome della directory da analizzare.
- **Ritorna**: Una stringa contenente i nomi dei file separati da newline.
- **Errori gestiti**:
  - Se la directory non può essere aperta o la memoria non può essere allocata, il programma termina con `EXIT_FAILURE`.

---

### `void update_file_with_list(const char *file_name, const char *dir_name)`
- **Descrizione**: Scrive in un file la lista dei file presenti in una directory.
- **Parametri**:
  - `file_name`: Nome del file in cui scrivere la lista.
  - `dir_name`: Nome della directory da analizzare.
- **Errori gestiti**:
  - Se il file non può essere aperto o la lista non può essere ottenuta, viene stampato un messaggio di errore.

---

### `void delete_file(const char *directory, const char *filename)`
- **Descrizione**: Elimina un file specifico da una directory.
- **Parametri**:
  - `directory`: Nome della directory contenente il file.
  - `filename`: Nome del file da eliminare.
- **Errori gestiti**:
  - Se il file non può essere eliminato, viene stampato un messaggio di errore.

---

## Costanti e macro

### `KRED`, `KWHT`, `RESET`
- **Descrizione**: Macro utilizzate per colorare l'output del terminale.
  - `KRED`: Colore rosso.
  - `KWHT`: Colore bianco.
  - `RESET`: Resetta il colore.

### `BUFLEN`
- **Descrizione**: Lunghezza massima del buffer utilizzato per i percorsi e i file.

  
**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------**
**FILE SERVERSYNCMSG.C**
## Funzioni principali

### `bool sndMsg_sync(int sock, char *snd_msg, struct sockaddr_in *snd_addr, socklen_t msg)`
- **Descrizione**: Invia un messaggio al client e attende una conferma (ACK) per garantire la sincronizzazione. La funzione gestisce la perdita di pacchetti simulata e riprova fino a un massimo di 3 tentativi.
- **Parametri**:
  - `sock`: Socket utilizzato per la comunicazione.
  - `snd_msg`: Messaggio da inviare.
  - `snd_addr`: Struttura contenente l'indirizzo del destinatario.
  - `msg`: Lunghezza del messaggio da inviare.
- **Ritorna**: `true` se la sincronizzazione è completata con successo, `false` in caso di errore o dopo 3 tentativi falliti.
- **Dettagli del funzionamento**:
  1. **Timeout**: Imposta un timeout per la ricezione dei messaggi utilizzando la funzione `setRcvTimeout`.
  2. **Simulazione perdita pacchetti**: Utilizza un valore casuale per simulare la perdita di pacchetti con probabilità `P`.
  3. **Invio messaggio**: Utilizza `sendto` per inviare il messaggio al client.
  4. **Ricezione ACK**: Utilizza `recvfrom` per ricevere un messaggio di conferma (ACK) dal client.
  5. **Verifica indirizzo**: Controlla che l'ACK provenga dall'indirizzo corretto.
  6. **Gestione messaggi ricevuti**:
     - Se riceve "ACK", prepara un messaggio di fine sincronizzazione ("SYN").
     - Se riceve "SYNACK", la sincronizzazione è completata con successo.
     - In caso di errore, termina il programma.
  7. **Riprova**: Se non riceve un ACK valido, riprova fino a un massimo di 3 tentativi.
- **Errori gestiti**:
  - Errore in `sendto`: Stampa un messaggio di errore e restituisce `false`.
  - Errore in `recvfrom`: Stampa un messaggio di errore e restituisce `false`.

---

## Funzioni di supporto

### `void setRcvTimeout(int sock, int timeout)`
- **Descrizione**: Imposta un timeout per la ricezione dei messaggi su un socket.
- **Parametri**:
  - `sock`: Socket su cui impostare il timeout.
  - `timeout`: Timeout in secondi.
- **Dettagli**:
  - Utilizza la struttura `timeval` per configurare il timeout.
  - Chiama `setsockopt` per applicare il timeout al socket.
- **Errori gestiti**:
  - Se `setsockopt` fallisce, stampa un messaggio di errore e termina il programma.

---

## Costanti e macro

### `SND_MSG_TIMEOUT`
- **Descrizione**: Timeout predefinito per la ricezione dei messaggi in secondi.
- **Utilizzo**: Utilizzato nella funzione `setRcvTimeout` per configurare il timeout del socket.

### `P`
- **Descrizione**: Probabilità di perdita di pacchetti simulata.
- **Utilizzo**: Utilizzata nella funzione `sndMsg_sync` per simulare la perdita di pacchetti durante l'invio.

### `KRED`, `KWHT`, `KGRN`, `RESET`
- **Descrizione**: Macro utilizzate per colorare l'output del terminale.
  - `KRED`: Colore rosso.
  - `KWHT`: Colore bianco.
  - `KGRN`: Colore verde.
  - `RESET`: Resetta il colore.

---

## Dettagli sull'implementazione

### Simulazione della perdita di pacchetti
La funzione `sndMsg_sync` utilizza un valore casuale generato con `rand()` per simulare la perdita di pacchetti. La probabilità di perdita è definita dalla costante `P`. Se il valore generato è inferiore a `1 - P`, il pacchetto viene inviato; altrimenti, viene scartato.

### Sincronizzazione dei messaggi
La sincronizzazione avviene attraverso un protocollo semplice:
1. Il server invia un messaggio al client.
2. Il client risponde con un messaggio di conferma (ACK o SYNACK).
3. Il server verifica che il messaggio ricevuto provenga dall'indirizzo corretto.
4. Se la sincronizzazione è completata con successo, il server restituisce `true`.

### Gestione dei tentativi
La funzione riprova fino a un massimo di 3 tentativi per inviare il messaggio e ricevere un ACK. Se tutti i tentativi falliscono, la funzione restituisce `false`.

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
**FILE SERVERRECEIVER.C**
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
**FILE SERVERSENDER.C**
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

