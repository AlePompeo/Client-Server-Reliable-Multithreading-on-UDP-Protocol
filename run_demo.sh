#!/bin/bash
# Progetto IIW - Script di avvio per la demo
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

# Directory di base = directory dove si trova questo script
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="$BASE_DIR/server"
CLIENT_DIR="$BASE_DIR/Client"

echo "📁 Directory base: $BASE_DIR"

# Controllo cartelle
if [ ! -d "$SERVER_DIR" ]; then
    echo "❌ La cartella $SERVER_DIR non esiste."
    exit 1
fi

if [ ! -d "$CLIENT_DIR" ]; then
    echo "❌ La cartella $CLIENT_DIR non esiste."
    exit 1
fi

# Controllo script
if [ ! -f "$SERVER_DIR/run_server.sh" ]; then
    echo "❌ Script run_server.sh non trovato in $SERVER_DIR."
    exit 1
fi

if [ ! -f "$CLIENT_DIR/run_client.sh" ]; then
    echo "❌ Script run_client.sh non trovato in $CLIENT_DIR."
    exit 1
fi

# Avvia in due terminali separati
gnome-terminal -- bash -c "
    cd \"$SERVER_DIR\"
    chmod +x run_server.sh
    ./run_server.sh
    exec bash
" &

gnome-terminal -- bash -c "
    cd \"$CLIENT_DIR\"
    chmod +x run_client.sh
    ./run_client.sh
    exec bash
" &

wait

echo "✅ Server e client avviati."
