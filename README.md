# 📡 Trasferimento File su UDP — Reliable Client-Server su Linux

Progetto universitario per il corso di **Ingegneria di Internet e Web** — implementazione di un'applicazione **Client-Server multithreading** in C che realizza un trasferimento file affidabile su **protocollo UDP**, emulando le principali caratteristiche di TCP a livello applicativo: controllo di flusso, rilevamento degli errori, ordinamento dei pacchetti e ritrasmissione selettiva.

> Università degli Studi di Roma **Tor Vergata** — A.A. 2023/2024
> A cura di: **Pompeo Alessio**, **Stoica Davide Edoardo**, **Caporaso Consuelo**

---

## 📌 Descrizione

Il sistema realizza un protocollo di trasferimento dati affidabile costruito sopra UDP, senza dipendere dai meccanismi nativi di TCP. Ogni sessione client è gestita da un **thread dedicato** sul server, con multiplazione e demultiplazione dinamica delle porte. Il trasferimento sfrutta **Selective Repeat** con buffer circolare, **Three-Way Handshake** personalizzato, **checksum CRC32**, **codifica Base64 con Cifrario di Cesare** e **calcolo adattivo del Timeout tramite RTT**.

---

## ✨ Funzionalità

### Comandi disponibili lato Client

| Comando | Descrizione |
|---------|-------------|
| `list` | Elenca i file disponibili sul server |
| `get <filename>` | Scarica un file dal server nella cartella `Download` |
| `put <filename>` | Carica un file dalla cartella `Upload` al server |
| `modify <filename>` | Modifica un file con editor Vim (locale) |
| `delete <filename>` | Elimina un file dal server |
| `compress <filename>` | Comprime un file in formato `.gz` |
| `decompress <filename.gz>` | Decomprime un file `.gz` |
| `quit` | Termina la sessione con il server |

### Comandi disponibili lato Server

| Comando | Descrizione |
|---------|-------------|
| `/list` | Lista i file nella directory `server_files` |
| `/delete <filename>` | Elimina un file dal server |
| `/connections` | Mostra tutte le connessioni attive (porta, socket, data/ora) |
| `/disconnect <port>` | Disconnette forzatamente un client su una porta specifica |
| `/shutdown` | Chiude ordinatamente il socket UDP del server |

---

## 🧠 Protocollo di Affidabilità

### Selective Repeat con Buffer Circolare
Il trasferimento dei file utilizza **Selective Repeat** con finestra di dimensione configurabile `N`. Ogni pacchetto è identificato da un numero di sequenza e bufferizzato in un **ring buffer** di dimensione `N`. Solo i pacchetti effettivamente persi vengono ritrasmessi, evitando le ritrasmissioni inutili del Go-Back-N.

Il mittente opera con **3 thread concorrenti**:
- `Thread_1` — lettura file, composizione e invio pacchetti
- `Thread_2` — gestione ACK e scorrimento della finestra
- `Thread_3` — gestione timeout tramite Linked List

Il destinatario opera con un **singolo flusso** (ricezione, invio ACK, scrittura su file).

### Three-Way Handshake (SYN / SYNACK / ACK — FIN / FINACK)
Ogni trasferimento è incapsulato in un handshake personalizzato ispirato a TCP, che garantisce affidabilità anche nello scambio di comandi semplici. Fino a **10 tentativi** di ritrasmissione prima di dichiarare la connessione fallita.

### Checksum CRC32
A differenza del checksum tradizionale UDP (16 bit, complemento a 1), viene utilizzato **CRC32** (*Cyclic Redundancy Check* a 32 bit) per una rilevazione degli errori di trasmissione significativamente più robusta.

### CodeBase64 + Cifrario di Cesare
I file vengono codificati in **Base64** prima della trasmissione. L'alfabeto Base64 è stato **shiftato tramite il Cifrario di Cesare** per aggiungere un livello di offuscamento dei dati trasmessi.

### Timeout Adattivo (Estimated RTT)
Il timeout è calcolato dinamicamente tramite la formula TCP standard:

```
SampleRTT     = t_arrivo - t_invio
EstimatedRTT  = 0.875 × EstimatedRTT + 0.125 × SampleRTT
DevRTT        = 0.75 × DevRTT + 0.25 × |SampleRTT - EstimatedRTT|
Timeout       = EstimatedRTT + 4 × DevRTT
```

### TOS — Type Of Service
Il campo `IP_TOS` è impostato tramite `setsockopt()` per assegnare **alta priorità** ai pacchetti UDP sia lato client che lato server.

---

## ⚙️ Architettura del Sistema

```
┌───────────────────────────────────────────────────────────────────┐
│                         SERVER                                    │
│                                                                   │
│  Main Thread (porta default 7000)                                 │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │  accept connessione → port_validation → sync → bind porta i │  │
│  └───────────────────────────┬─────────────────────────────────┘  │
│                              │ pthread_create                      │
│           ┌──────────────────┼──────────────────┐                 │
│   Thread_i │  handle_request │  Thread_j        │  Thread_k       │
│   porta i  │  porta j        │  porta k  ...    │                 │
└────────────┴─────────────────┴──────────────────┴─────────────────┘
                    ▲                    ▲
                    │  UDP / Selective   │
                    │  Repeat            │
                    ▼                    ▼
┌──────────────────────────────────────────────────────────────────┐
│                         CLIENT                                   │
│  Upload/  ← put          get → Download/                         │
│  Thread_1 (packet) | Thread_2 (ACK) | Thread_3 (timeout)        │
└──────────────────────────────────────────────────────────────────┘
```

### Struttura del segmento UDP personalizzato

```
┌─────────────┬──────────────────┐
│ Source Port │ Destination Port │
├─────────────┴──────────────────┤
│              ACK               │
├──────────────┬─────────────────┤
│ Header Len   │   Window Size   │
├──────────────┬────┬────┬───────┤
│   Checksum   │SYN │FIN │Timeout│
├──────────────┴────┴────┴───────┤
│              Data              │
└────────────────────────────────┘
```

---

## 🛠️ Compilazione ed Esecuzione

### Requisiti

- Sistema Operativo **Linux** (Ubuntu / Fedora / Arch)
- GCC ≥ 14
- `make`
- `zlib` (`libz-dev` o equivalente)
- `pthread`

### Installazione automatica

```bash
chmod +x *.sh
./installation.sh
```

Lo script guida l'installazione, installa le dipendenze necessarie e avvia la demo aprendo due terminali per Client e Server.

### Compilazione manuale

```bash
# Server
cd Server && make

# Client
cd Client && make
```

### Avvio

```bash
./run_server.sh   # inserire IP e porta (default: 127.0.0.1, 7000–65535)
./run_client.sh   # inserire stesso IP e porta del server
```

### Configurazione parametri (opzionale)

```bash
./config_macros.sh
```

Parametri configurabili:

| Parametro | Descrizione |
|-----------|-------------|
| `N` | Dimensione della sliding window |
| `P` | Probabilità di perdita simulata dei pacchetti |
| `T` | Valore timeout fisso |
| `FIXEDTIMEOUT` | Abilita timeout fisso (disabilita RTT adattivo) |
| `MAX_THREADS` | Numero massimo di client connessi contemporaneamente (default: 50) |
| `PERFORMANCE` | Disabilita output diagnostico per massimizzare il throughput |
| `MAXIMUM_ATTEMPT` | Numero massimo di tentativi di connessione |

---

## 📁 Struttura del Repository

```
.
├── Server/
│   ├── server_files/          # File disponibili per il download
│   ├── src/
│   │   ├── server.c           # Entry point e gestione richieste
│   │   ├── serversender.c     # Invio file verso il client
│   │   ├── serverreceiver.c   # Ricezione file dal client
│   │   ├── serverfileoperation.c
│   │   ├── serversyncmsg.c    # Handshake iniziale
│   │   ├── encode64.c / decode64.c
│   │   └── reliableUDP.c      # Implementazione CRC32
│   ├── include/
│   │   └── macros.h           # Parametri globali configurabili
│   ├── makefile
│   └── run_server.sh
│
├── Client/
│   ├── Download/              # File ricevuti con get
│   ├── Upload/                # File da inviare con put
│   ├── src/
│   │   ├── client.c           # Entry point e menu interattivo
│   │   ├── clientsender.c     # Invio file verso il server
│   │   ├── clientreceiver.c   # Ricezione file dal server
│   │   ├── clientfileoperation.c
│   │   ├── clientsyncmsg.c
│   │   ├── encode64.c / decode64.c
│   │   └── reliableUDP.c
│   ├── include/
│   │   └── macros.h
│   ├── makefile
│   └── run_client.sh
│
├── installation.sh
├── config_macros.sh
└── uninstall.sh
```

---

## 📊 Prestazioni (campione — localhost, P=20%, T adattivo)

| File | Dimensione | `get` (N=50) | `put` (N=50) |
|------|------------|--------------|--------------|
| BancaItalia.txt | 62.3 KB | 1.064 s | 0.034 s |
| file-sample2.pdf | 1.3 MB | 1.372 s | 0.570 s |
| file-sample3.mp3 | 3.9 MB | 2.188 s | 2.079 s |
| 20mb.zip | 20 MB | 7.754 s | 7.960 s |
| file-sample4.mp4 | 38.2 MB | 15.056 s | 15.066 s |
| 100mb-fake.zip | 104.7 MB | 38.678 s | 42.554 s |

Il trasferimento di un file da 100 MB con probabilità di perdita del 90% viene completato in circa **53 s** (get) e **50 s** (put) grazie alla ritrasmissione selettiva e al timeout adattivo.

---

## ⚠️ Limitazioni Note

- Possibile **memory leak** in sessioni prolungate o con trasferimenti multipli
- Il comportamento può variare tra versioni di GCC/glibc differenti — si raccomanda di standardizzare l'ambiente
- Assenza di **meccanismo di autenticazione** tra mittente e destinatario
- Assenza di **ripresa automatica** del trasferimento in caso di crash

---

## 🔧 Ambiente di Sviluppo

- **OS**: Linux Fedora 41 / Ubuntu 24.04 LTS
- **Compilatore**: GCC 14.2
- **Librerie**: `zlib`, `pthread`, Berkeley Sockets API
- **Tool**: VSCode, GDB, Valgrind, Make
