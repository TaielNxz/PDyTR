#!/usr/bin/env python3

import argparse
import csv
import sys
from pathlib import Path


COLUMNAS_REQUERIDAS = (
    "bytes",
    "repeticiones",
    "write_avg_ns",
    "read_avg_ns",
)


def leer_resultados(ruta_csv):
    with ruta_csv.open("r", encoding="utf-8", newline="") as archivo:
        lector = csv.DictReader(archivo)
        columnas = set(lector.fieldnames or [])
        faltantes = set(COLUMNAS_REQUERIDAS) - columnas

        if faltantes:
            raise ValueError(
                "Faltan columnas en el CSV: " + ", ".join(sorted(faltantes))
            )

        resultados = []
        for numero_fila, fila in enumerate(lector, start=2):
            try:
                resultados.append(
                    (
                        int(fila["bytes"]),
                        int(fila["repeticiones"]),
                        float(fila["write_avg_ns"]) / 1000,
                        float(fila["read_avg_ns"]) / 1000,
                    )
                )
            except (TypeError, ValueError) as error:
                raise ValueError(
                    f"La fila {numero_fila} contiene un valor invalido"
                ) from error

    if not resultados:
        raise ValueError("El CSV no contiene resultados")

    return sorted(resultados)


def imprimir_tabla(resultados):
    encabezados = ("Bytes", "Rep.", "Write (us)", "Read (us)")
    filas = [
        (f"{bytes_enviados:,}", str(repeticiones), f"{write_us:,.3f}", f"{read_us:,.3f}")
        for bytes_enviados, repeticiones, write_us, read_us in resultados
    ]
    anchos = [
        max(len(encabezados[columna]), *(len(fila[columna]) for fila in filas))
        for columna in range(len(encabezados))
    ]

    borde = "+-" + "-+-".join("-" * ancho for ancho in anchos) + "-+"

    def linea(valores):
        return "| " + " | ".join(
            valor.rjust(anchos[indice])
            for indice, valor in enumerate(valores)
        ) + " |"

    print(borde)
    print(linea(encabezados))
    print(borde)
    for fila in filas:
        print(linea(fila))
    print(borde)


def main():
    ruta_predeterminada = Path(__file__).with_name("resultados.csv")
    parser = argparse.ArgumentParser(
        description="Muestra los resultados de tiempos en una tabla compacta."
    )
    parser.add_argument(
        "csv",
        nargs="?",
        type=Path,
        default=ruta_predeterminada,
        help="archivo CSV (por defecto: resultados.csv junto al script)",
    )
    argumentos = parser.parse_args()

    try:
        imprimir_tabla(leer_resultados(argumentos.csv))
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
