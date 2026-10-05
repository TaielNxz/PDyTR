#!/usr/bin/env bash
# Dentro de la VM cliente. Uso: bash /vagrant/auto_cl.sh {timing|pingpong} ip [puerto] [ejecuciones] [carpeta]
set -euo pipefail

EXPERIMENTO="${1:?Indique timing o pingpong}"
SERVIDOR="${2:?Indique la IP del servidor}"
PUERTO="${3:-5000}"
EJECUCIONES="${4:-10}"
SALIDA="${5:-/vagrant/experiments/resultados/${EXPERIMENTO}_$(date +%Y%m%d_%H%M%S)_$$}"
case "$EXPERIMENTO" in
    timing) FUENTE=client_timing_chart.c ;;
    pingpong) FUENTE=client_pingpong.c ;;
    *) echo "Experimento invalido: $EXPERIMENTO" >&2; exit 1 ;;
esac
[[ "$PUERTO" =~ ^[0-9]+$ && "$EJECUCIONES" =~ ^[0-9]+$ ]] || exit 1
(( PUERTO >= 1 && PUERTO <= 65535 && EJECUCIONES >= 1 )) || exit 1

BIN="${TMPDIR:-/tmp}/tp2-${UID}"
mkdir -p "$BIN" "$(dirname "$SALIDA")"
# Una carpeta nueva evita sobrescribir resultados anteriores.
mkdir "$SALIDA"
gcc -O2 -Wall -Wextra "/experiments-src/$EXPERIMENTO/$FUENTE" -o "$BIN/client_$EXPERIMENTO"

echo "Esperando al servidor $SERVIDOR:$PUERTO..."
INTENTOS=0
until nc -z -w 1 "$SERVIDOR" "$PUERTO" >/dev/null 2>&1; do
    INTENTOS=$((INTENTOS + 1))
    if (( INTENTOS >= 60 )); then
        echo "El servidor no esta disponible" >&2
        exit 1
    fi
    sleep 1
done

# La espera anterior queda fuera de los tiempos medidos.
# Cada CSV contiene seis tamanos, con diez repeticiones por tamano.
for ((i=1; i<=EJECUCIONES; i++)); do
    printf -v ARCHIVO '%s/experimento_%02d.csv' "$SALIDA" "$i"
    timeout 120s "$BIN/client_$EXPERIMENTO" "$SERVIDOR" "$PUERTO" >"$ARCHIVO.partial"
    [[ "$(wc -l <"$ARCHIVO.partial")" -eq 7 ]] || { echo "CSV incompleto" >&2; exit 1; }
    mv "$ARCHIVO.partial" "$ARCHIVO"
    echo "Experimento $i/$EJECUCIONES guardado en $ARCHIVO"
    if (( i < EJECUCIONES )); then sleep 1; fi
done
