#!/bin/bash
# Progetto IIW - Script di avvio sequenziale per installazione e demo
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

clear
echo "🛠️  Benvenuto nell'installer del progetto IIW!"
echo

# Chiede se l'utente vuole disinstallare prima di continuare
read -p "Vuoi eseguire una disinstallazione del programma? [s/n]: " scelta_disinstallazione

if [[ "$scelta_disinstallazione" =~ ^[sS]$ ]]; then
    echo "🚮 Avvio disinstallazione..."

    if [ -f "./uninstall.sh" ]; then
        bash ./uninstall.sh || { echo "❌ Errore durante la disinstallazione."; exit 1; }
        exit 0
    else
        echo "❌ uninstall.sh non trovato. Impossibile eseguire la disinstallazione."
        exit 1
    fi
else
    echo "❎ Non verrà eseguita alcuna disinstallazione. Continuo con l'installazione."
fi

# Chiede all'utente il tipo di installazione
read -p "Vuoi eseguire un'installazione completa (include strumenti e potrebbe necessitare di permessi admin)? [s/n]: " scelta_completa

if [[ "$scelta_completa" =~ ^[sS]$ ]]; then
    echo "🚀 Avvio installazione COMPLETA (tools + programma)..."
    
    if [ -f "./tools_install.sh" ]; then
        bash ./tools_install.sh || { echo "❌ Errore in tools_install.sh"; exit 1; }
    else
        echo "❌ tools_install.sh non trovato."
        exit 1
    fi
else
    echo "⚙️  Avvio installazione STANDARD (solo programma)..."
fi

# Esegui sempre program_install.sh
if [ -f "./program_install.sh" ]; then
    bash ./program_install.sh || { echo "❌ Errore in program_install.sh"; exit 1; }
else
    echo "❌ program_install.sh non trovato."
    exit 1
fi

# Chiede se avviare la demo
echo
read -p "Vuoi avviare la demo del programma ora? [s/n]: " scelta_demo

if [[ "$scelta_demo" =~ ^[sS]$ ]]; then
    DEMO_PATH="$HOME/local_bin/Progetto_IIW/run_demo.sh"
    if [ -f "$DEMO_PATH" ]; then
        echo "▶️  Avvio della demo da $DEMO_PATH..."
        bash "$DEMO_PATH"
    else
        echo "❌ run_demo.sh non trovato in $DEMO_PATH."
        exit 1
    fi
else
    echo "✅ Installazione completata. Puoi eseguire la demo in seguito con:"
    echo "   bash ~/local_bin/Progetto_IIW/run_demo.sh"
fi
