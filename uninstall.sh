#!/bin/bash
# Script di disinstallazione per il progetto IIW
# Rimuove la directory local_bin
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

TARGET_DIR="$HOME/local_bin"

echo "⚠️  ATTENZIONE: Questo script rimuoverà completamente la cartella: $TARGET_DIR"
echo

# Verifica se la directory esiste
if [ -d "$TARGET_DIR" ]; then
    read -p "Sei sicuro di voler procedere con la disinstallazione? [s/N]: " confirm

    if [[ "$confirm" =~ ^[sS]$ ]]; then
        echo "🗑️  Rimozione della cartella $TARGET_DIR in corso..."
        rm -rf "$TARGET_DIR"

        if [ $? -eq 0 ]; then
            echo "✅ Disinstallazione completata con successo."
        else
            echo "❌ Errore durante la disinstallazione."
            exit 1
        fi
    else
        echo "❎ Disinstallazione annullata."
        exit 0
    fi
else
    echo "ℹ️  La cartella $TARGET_DIR non esiste. Nulla da disinstallare."
    exit 0
fi
