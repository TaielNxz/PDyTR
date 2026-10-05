#!/usr/bin/env bash
# Procedimiento de comprobacion del ejercicio 5, sin cambiar los programas medidos.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")" && pwd)"
ROL="${1:?Indique server o client}"
CONDICION="${2:?Indique sin_retraso o con_retraso}"

if [[ "$ROL" == server ]]; then
    if [[ "$CONDICION" == con_retraso ]]; then
        # Esperar a que el cliente haya llegado al bucle de disponibilidad.
        for ((i=0; i<600; i++)); do
            if grep -q 'Esperando al servidor' "$RAIZ/${CONDICION}_cliente.log" 2>/dev/null; then break; fi
            sleep 0.1
        done
        grep -q 'Esperando al servidor' "$RAIZ/${CONDICION}_cliente.log"
        date +%s.%N > "$RAIZ/inicio_sleep.txt"

        sleep 10 # <<< ============== RECONTRA IMPORTANTE

        date +%s.%N > "$RAIZ/fin_sleep.txt"
    fi
    date +%s.%N > "$RAIZ/${CONDICION}_inicio_servidor.txt"
    exec bash /vagrant/script_servidor.sh pingpong 5000
elif [[ "$ROL" == client ]]; then
    date +%s.%N > "$RAIZ/${CONDICION}_inicio_cliente.txt"
    /usr/bin/time -f 'real_s=%e\nuser_s=%U\nsys_s=%S' \
        -o "$RAIZ/${CONDICION}_tiempo_total.txt" \
        bash /vagrant/script_cliente.sh pingpong 192.168.56.20 5000 10 "$RAIZ/$CONDICION" \
        > "$RAIZ/${CONDICION}_cliente.log" 2>&1
    date +%s.%N > "$RAIZ/${CONDICION}_fin_cliente.txt"
else
    echo 'Rol invalido' >&2
    exit 1
fi
