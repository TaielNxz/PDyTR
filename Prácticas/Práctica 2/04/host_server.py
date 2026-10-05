#!/usr/bin/env python3
# En Windows: python .\host_server.py {timing|pingpong} --host 192.168.56.1 --port 5000
# Usar la IP del adaptador host-only de VirtualBox y permitir TCP 5000 en el firewall.
# El cliente VM usa 192.168.56.21. El servidor se detiene con Ctrl+C.

import argparse
import socket
import struct
import sys


TAMANOS_PERMITIDOS = {10, 100, 1000, 10000, 100000, 1000000}


def recibir_todo(conexion, cantidad, permitir_vacio=False):
    datos = bytearray()
    while len(datos) < cantidad:
        bloque = conexion.recv(cantidad - len(datos))
        if not bloque:
            if permitir_vacio and not datos:
                return None
            raise ConnectionError("la conexión se cerró antes de tiempo")
        datos.extend(bloque)
    return bytes(datos)


def atender_cliente(conexion, experimento):
    # nc -z comprueba el puerto y cierra la conexion sin enviar datos.
    cabecera = recibir_todo(conexion, 4, permitir_vacio=True)
    if cabecera is None:
        return
    cantidad = struct.unpack("!I", cabecera)[0]
    if cantidad not in TAMANOS_PERMITIDOS:
        raise ValueError(f"cantidad de bytes no permitida: {cantidad}")

    datos = recibir_todo(conexion, cantidad)
    esperado = bytes(indice % 256 for indice in range(cantidad))
    if datos != esperado:
        raise ValueError("los datos recibidos no son correctos")
    if experimento == "timing":
        conexion.sendall(struct.pack("!I", cantidad))
    else:
        conexion.sendall(datos)


def main():
    parser = argparse.ArgumentParser(
        description="Servidor del host para el escenario b."
    )
    parser.add_argument("experimento", choices=("timing", "pingpong"))
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=5000)
    argumentos = parser.parse_args()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as servidor:
        servidor.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        servidor.bind((argumentos.host, argumentos.port))
        servidor.listen(5)
        print(
            f"Servidor del host escuchando en {argumentos.host}:{argumentos.port}",
            flush=True,
        )

        while True:
            conexion, direccion = servidor.accept()
            with conexion:
                try:
                    atender_cliente(conexion, argumentos.experimento)
                except (ConnectionError, OSError, ValueError) as error:
                    print(f"Cliente {direccion}: {error}", file=sys.stderr, flush=True)


if __name__ == "__main__":
    main()
