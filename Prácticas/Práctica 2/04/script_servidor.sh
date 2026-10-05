#!/usr/bin/env bash
# Uso dentro de la VM: bash /vagrant/auto_sv.sh {timing|pingpong} [puerto]
set -euo pipefail

EXPERIMENTO="${1:?Indique timing o pingpong}"
PUERTO="${2:-5000}"
case "$EXPERIMENTO" in
    timing) FUENTE=server_timing.c ;;
    pingpong) FUENTE=server_pingpong.c ;;
    *) echo "Experimento invalido: $EXPERIMENTO" >&2; exit 1 ;;
esac
[[ "$PUERTO" =~ ^[0-9]+$ ]] && (( PUERTO >= 1 && PUERTO <= 65535 )) || exit 1

# Compilar en /tmp evita ejecutar binarios desde la carpeta compartida.
BIN="${TMPDIR:-/tmp}/tp2-${UID}"
mkdir -p "$BIN"
gcc -O2 -Wall -Wextra "/experiments-src/$EXPERIMENTO/$FUENTE" -o "$BIN/server_$EXPERIMENTO"

# El servidor ya atiende varias conexiones; se detiene con Ctrl+C.
exec "$BIN/server_$EXPERIMENTO" "$PUERTO"
