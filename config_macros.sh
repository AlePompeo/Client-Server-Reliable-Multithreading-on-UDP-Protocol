#!/bin/bash
# Progetto IIW - Script per modificare i parametri nei file macros.h del progetto IIW
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

PARAMS=("N" "P" "T" "FIXEDTIMEOUT" "MAX_THREADS" "PERFORMANCE" "MAXIMUM_ATTEMPT")
FILES=("server/include/macros.h" "Client/include/macros.h")

echo "⚙️  Modifica dei parametri in macros.h"
echo

# Funzione per aggiornare i valori
update_param() {
    local param="$1"
    local new_value="$2"

    for file in "${FILES[@]}"; do
        if [[ -f "$file" ]]; then
            echo "🔧 Aggiornamento $param in $file → $new_value"
            sed -i "s/^#define[[:space:]]\+$param[[:space:]]\+.*/#define $param $new_value/" "$file"
        else
            echo "❌ File non trovato: $file"
        fi
    done
}

# Ciclo di input per ciascun parametro
for param in "${PARAMS[@]}"; do
    read -p "Inserisci nuovo valore per $param (invio per lasciare invariato): " val
    if [[ -n "$val" ]]; then
        update_param "$param" "$val"
    else
        echo "⏭️  $param lasciato invariato."
    fi
    echo
done

echo "✅ Modifica completata."