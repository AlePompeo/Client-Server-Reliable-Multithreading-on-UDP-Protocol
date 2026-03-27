#!/bin/bash
# Progetto IIW - Installazione e configurazione
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

# Funzione per determinare il gestore di pacchetti in base alla distribuzione
get_package_manager() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        if [[ "$ID" == "ubuntu" || "$ID" == "debian" ]]; then
            echo "apt"
        elif [[ "$ID" == "fedora" || "$ID" == "rhel" || "$ID" == "centos" ]]; then
            echo "dnf"
        else
            echo "unknown"
        fi
    else
        echo "unknown"
    fi
}

# Funzione per installare un pacchetto
install_package() {
    PACKAGE=$1
    PACKAGE_MANAGER=$(get_package_manager)

    echo "➡️ Verifica se $PACKAGE è installato..."

    # Verifica se il pacchetto è già installato
    if dpkg -s "$PACKAGE" &> /dev/null || rpm -q "$PACKAGE" &> /dev/null || command -v "$PACKAGE" &> /dev/null; then
        echo "✅ $PACKAGE è già installato."
    else
        echo "❌ $PACKAGE non trovato. Procedo con l'installazione..."

        # Installa il pacchetto in base al gestore di pacchetti
        if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
            sudo apt update
            sudo apt install -y "$PACKAGE" || { echo "❌ Errore nell'installazione di $PACKAGE"; exit 1; }
        elif [[ "$PACKAGE_MANAGER" == "dnf" ]]; then
            sudo dnf install -y "$PACKAGE" || { echo "❌ Errore nell'installazione di $PACKAGE"; exit 1; }
        else
            echo "❌ Gestore di pacchetti non supportato per questa distribuzione."
            exit 1
        fi

        echo "✅ $PACKAGE installato correttamente."
    fi
}

# commentato perchè richiede permessi di root,
# e non è necessario per il funzionamento del programma (funzione modify)
# se si vuole un'esperienza più completa si può decommentare
# install_package vim 

echo "➡️ Verifica e installazione di pthread..."

# Verifica se pthread è presente
if gcc -pthread -v &> /dev/null; then
    echo "✅ pthread è già disponibile tramite il compilatore."
else
    echo "❌ pthread non disponibile. Procedo con l'installazione..."
    PACKAGE_MANAGER=$(get_package_manager)

    if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
        sudo apt update
        sudo apt install -y build-essential || { echo "❌ Errore nell'installazione di build-essential"; exit 1; }
    elif [[ "$PACKAGE_MANAGER" == "dnf" ]]; then
        sudo dnf install -y gcc gcc-c++ make || { echo "❌ Errore nell'installazione di build tools"; exit 1; }
    fi
    echo "✅ pthread disponibile dopo l'installazione di build tools."
fi

echo "➡️ Verifica presenza delle librerie C standard (glibc)..."

PACKAGE_MANAGER=$(get_package_manager)

if [[ "$PACKAGE_MANAGER" == "apt" ]]; then
    install_package libc6-dev
elif [[ "$PACKAGE_MANAGER" == "dnf" ]]; then
    install_package glibc-devel
else
    echo "⚠️ Impossibile verificare/installare le librerie C su questa distribuzione."
fi

# Verifica la versione di gcc
current_gcc_version=$(gcc --version | head -n 1 | awk '{print $3}')
required_gcc_version="13.1.0"
# Il programma è stato testato con GCC 14.2.1, GCC 14.2.0 e GCC 13.2.0
# Siccome al momento è una delle più recenti se necessario si installa la 14.2.0

echo "➡️ Verifica versione di GCC..."

# Confronta la versione corrente con quella richiesta
if [[ "$current_gcc_version" < "$required_gcc_version" ]]; then
    echo "❌ La versione di GCC ($current_gcc_version) è inferiore alla versione richiesta ($required_gcc_version). Procedo con l'installazione di GCC 14.2.0..."

    sudo apt update
    sudo apt install -y wget
    wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz || { echo "❌ Errore nel download di GCC 14.2.0"; exit 1; }
    tar -xzf gcc-14.2.0.tar.gz
    cd gcc-14.2.0 || { echo "❌ Errore nell'estrazione di GCC 14.2.0"; exit 1; }

    sudo apt install -y build-essential libgmp-dev libmpfr-dev libmpc-dev || { echo "❌ Errore nell'installazione delle dipendenze"; exit 1; }

    ./configure --disable-multilib --enable-languages=c,c++ --prefix=/usr/local || { echo "❌ Errore nella configurazione di GCC"; exit 1; }
    make -j$(nproc) || { echo "❌ Errore durante la compilazione di GCC"; exit 1; }
    sudo make install || { echo "❌ Errore nell'installazione di GCC"; exit 1; }

    echo "✅ GCC 14.2.0 installato correttamente."
else
    echo "✅ La versione di GCC ($current_gcc_version) è già la versione richiesta o più recente."
fi
