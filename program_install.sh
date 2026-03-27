#!/bin/bash
# Progetto IIW - Installazione e configurazione
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

# Variabili
ZIP_FILE="Progetto_IIW.zip"
EXTRACTED_DIR="Progetto_IIW"
TARGET_DIR="$HOME/local_bin"
ZLIB_URL="https://zlib.net/zlib-1.3.1.tar.gz"
ZLIB_DIR="$TARGET_DIR/zlib"

# Verifica connessione a Internet
check_internet_connection() {
    echo "🌐 Verifica della connessione a Internet..."
    if ping -q -c 1 -W 2 google.com > /dev/null; then
        echo "✅ Connessione a Internet rilevata."
    else
        echo "❌ Nessuna connessione a Internet. Impossibile procedere con il download."
        exit 1
    fi
}

# Verifica esistenza local_bin
if [ -d "$TARGET_DIR" ]; then
    echo "⚠️  La directory $TARGET_DIR esiste già."
    read -p "Vuoi rimuoverla per procedere con una nuova installazione? (s/n): " risposta
    case "$risposta" in
        [sS])
            echo "🧹 Rimozione della directory $TARGET_DIR..."
            rm -rf "$TARGET_DIR" || { echo "❌ Impossibile rimuovere $TARGET_DIR"; exit 1; }
            ;;
        *)
            echo "ℹ️ Installazione annullata dall'utente."
            exit 0
            ;;
    esac
fi

mkdir -p "$TARGET_DIR"

# Estrazione e gestione zip
extract_and_move_zip() {
    echo "🔄 Estrazione e spostamento della cartella $ZIP_FILE..."

    if [ ! -f "$ZIP_FILE" ]; then
        echo "❌ File $ZIP_FILE non trovato."
        exit 1
    fi

    unzip "$ZIP_FILE" -d "$EXTRACTED_DIR"
    if [ $? -ne 0 ]; then
        echo "❌ Errore durante l'estrazione."
        exit 1
    fi

    mv "$EXTRACTED_DIR" "$TARGET_DIR/" || { echo "❌ Errore nello spostamento della cartella."; exit 1; }

    echo "✅ Cartella spostata in $TARGET_DIR/$EXTRACTED_DIR"

    # Gestione sampleFiles
    SAMPLE_SOURCE="$TARGET_DIR/$EXTRACTED_DIR/sampleFiles"
    if [ -d "$SAMPLE_SOURCE" ]; then
        echo "📁 Cartella sampleFiles trovata. Procedo con la copia..."

        for dir in "Client" "server"; do
            DEST="$TARGET_DIR/$EXTRACTED_DIR/$dir/sampleFiles"
            mkdir -p "$DEST"
            cp -r "$SAMPLE_SOURCE/"* "$DEST/"
            echo "✅ sampleFiles copiato in $dir/"
        done

        # Rimozione da root del progetto
        rm -rf "$SAMPLE_SOURCE"
        echo "🧹 sampleFiles rimosso dalla root del progetto."
    else
        echo "ℹ️ Nessuna cartella sampleFiles trovata durante l'estrazione."
    fi
}

# Installazione zlib
install_zlib() {
    echo "➡️  Inizio installazione di zlib..."
    cd "$TARGET_DIR" || exit 1

    echo "⬇️  Scaricamento zlib..."
    wget "$ZLIB_URL" -O zlib.tar.gz || { echo "❌ Errore nel download"; exit 1; }

    echo "📦 Estrazione zlib..."
    tar -xzf zlib.tar.gz
    cd zlib-* || { echo "❌ Errore nell'estrazione"; exit 1; }

    echo "⚙️  Configurazione e compilazione zlib..."
    ./configure --prefix="$ZLIB_DIR" && make && make install || { echo "❌ Errore nella compilazione"; exit 1; }

    echo "✅ zlib installato in: $ZLIB_DIR"
}

# Esecuzione dello script
check_internet_connection
extract_and_move_zip
install_zlib
