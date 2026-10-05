#!/usr/bin/env python3

import argparse
import csv
import math
import sys
from pathlib import Path


COLUMNAS_REQUERIDAS = {
    "bytes",
    "write_avg_ns",
    "read_avg_ns",
}


def leer_resultados(ruta_csv):
    with ruta_csv.open("r", encoding="utf-8", newline="") as archivo:
        lector = csv.DictReader(archivo)
        columnas = set(lector.fieldnames or [])
        faltantes = COLUMNAS_REQUERIDAS - columnas

        if faltantes:
            raise ValueError(
                "Faltan columnas en el CSV: " + ", ".join(sorted(faltantes))
            )

        resultados = []
        for numero_fila, fila in enumerate(lector, start=2):
            try:
                resultados.append(
                    {
                        "bytes": int(fila["bytes"]),
                        "write_us": float(fila["write_avg_ns"]) / 1000,
                        "read_us": float(fila["read_avg_ns"]) / 1000,
                    }
                )
            except (TypeError, ValueError) as error:
                raise ValueError(
                    f"La fila {numero_fila} contiene un valor invalido"
                ) from error

    if len(resultados) < 2:
        raise ValueError("El CSV debe contener al menos dos resultados")
    if any(
        resultado[clave] <= 0
        for resultado in resultados
        for clave in ("bytes", "write_us", "read_us")
    ):
        raise ValueError("Los bytes y los tiempos deben ser mayores que cero")

    return sorted(resultados, key=lambda resultado: resultado["bytes"])


def crear_svg(resultados, clave_tiempo, titulo, color):
    ancho, alto = 1000, 620
    izquierda, derecha, arriba, abajo = 95, 45, 75, 90
    grafico_ancho = ancho - izquierda - derecha
    grafico_alto = alto - arriba - abajo

    x_min = math.log10(resultados[0]["bytes"])
    x_max = math.log10(resultados[-1]["bytes"])
    tiempos = [resultado[clave_tiempo] for resultado in resultados]
    y_min = math.log10(min(tiempos))
    y_max = math.log10(max(tiempos))
    margen_y = max((y_max - y_min) * 0.08, 0.15)
    y_min -= margen_y
    y_max += margen_y

    def posicion_x(bytes_enviados):
        return izquierda + (math.log10(bytes_enviados) - x_min) * grafico_ancho / (x_max - x_min)

    def posicion_y(tiempo_us):
        return arriba + (y_max - math.log10(tiempo_us)) * grafico_alto / (y_max - y_min)

    svg = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{ancho}" height="{alto}" viewBox="0 0 {ancho} {alto}">',
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        '<style>text{font-family:Arial,sans-serif;fill:#202124}.titulo{font-size:24px;font-weight:bold}.eje{font-size:14px}.marca{font-size:12px;fill:#5f6368}.leyenda{font-size:14px;font-weight:bold}</style>',
        f'<text class="titulo" x="{ancho / 2}" y="36" text-anchor="middle">{titulo}</text>',
    ]

    cantidad_lineas_y = 6
    for indice in range(cantidad_lineas_y):
        log_valor = y_min + indice * (y_max - y_min) / (cantidad_lineas_y - 1)
        valor = 10 ** log_valor
        y = posicion_y(valor)
        svg.append(f'<line x1="{izquierda}" y1="{y:.2f}" x2="{ancho - derecha}" y2="{y:.2f}" stroke="#dfe3e8"/>')
        svg.append(f'<text class="marca" x="{izquierda - 12}" y="{y + 4:.2f}" text-anchor="end">{valor:,.1f}</text>')

    for resultado in resultados:
        x = posicion_x(resultado["bytes"])
        svg.append(f'<line x1="{x:.2f}" y1="{arriba}" x2="{x:.2f}" y2="{alto - abajo}" stroke="#eef0f2"/>')
        svg.append(f'<text class="marca" x="{x:.2f}" y="{alto - abajo + 24}" text-anchor="middle">{resultado["bytes"]:,}</text>')

    svg.extend(
        [
            f'<line x1="{izquierda}" y1="{arriba}" x2="{izquierda}" y2="{alto - abajo}" stroke="#202124" stroke-width="2"/>',
            f'<line x1="{izquierda}" y1="{alto - abajo}" x2="{ancho - derecha}" y2="{alto - abajo}" stroke="#202124" stroke-width="2"/>',
            f'<text class="eje" x="{ancho / 2}" y="{alto - 25}" text-anchor="middle">Cantidad de bytes (escala logaritmica)</text>',
            f'<text class="eje" x="24" y="{alto / 2}" text-anchor="middle" transform="rotate(-90 24 {alto / 2})">Tiempo promedio en us (escala logaritmica)</text>',
        ]
    )

    puntos = " ".join(
        f'{posicion_x(resultado["bytes"]):.2f},{posicion_y(resultado[clave_tiempo]):.2f}'
        for resultado in resultados
    )
    svg.append(
        f'<polyline points="{puntos}" fill="none" stroke="{color}" stroke-width="3"/>'
    )
    for resultado in resultados:
        svg.append(
            f'<circle cx="{posicion_x(resultado["bytes"]):.2f}" cy="{posicion_y(resultado[clave_tiempo]):.2f}" r="5" fill="{color}"/>'
        )

    svg.append("</svg>")
    return "\n".join(svg) + "\n"


def main():
    ruta_predeterminada = Path(__file__).with_name("resultados.csv")
    parser = argparse.ArgumentParser(
        description="Genera graficos SVG separados para write() y read()."
    )
    parser.add_argument(
        "csv",
        nargs="?",
        type=Path,
        default=ruta_predeterminada,
        help="archivo CSV (por defecto: resultados.csv junto al script)",
    )
    parser.add_argument(
        "-o",
        "--salida",
        type=Path,
        help="nombre base de salida (por defecto: mismo nombre que el CSV)",
    )
    argumentos = parser.parse_args()
    ruta_base = argumentos.salida or argumentos.csv.with_suffix("")
    ruta_base = ruta_base.with_suffix("")
    ruta_write = ruta_base.with_name(ruta_base.name + "_write.svg")
    ruta_read = ruta_base.with_name(ruta_base.name + "_read.svg")

    try:
        resultados = leer_resultados(argumentos.csv)
        ruta_write.write_text(
            crear_svg(resultados, "write_us", "Tiempo promedio de write()", "#1565c0"),
            encoding="utf-8",
        )
        ruta_read.write_text(
            crear_svg(resultados, "read_us", "Tiempo promedio de read()", "#d84315"),
            encoding="utf-8",
        )
    except (OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(f"Grafico de write() generado: {ruta_write}")
    print(f"Grafico de read() generado:  {ruta_read}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
