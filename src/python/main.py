"""Punto de entrada de la versión Python."""

import argparse
from pathlib import Path

from intransitive.application import ejecutar


def crear_interfaz() -> argparse.ArgumentParser:
    """Define los argumentos disponibles al ejecutar el programa."""
    parser = argparse.ArgumentParser(
        description="Busca ciclos no transitivos de parejas de dados."
    )
    parser.add_argument(
        "parametros",
        nargs="?",
        type=Path,
        default=Path(__file__).resolve().parents[2]
        / "config"
        / "Parámetros.txt",
        help="Archivo de parámetros (por defecto: config/Parámetros.txt).",
    )
    return parser


def main() -> int:
    """Procesa los argumentos y ejecuta la búsqueda configurada."""
    argumentos = crear_interfaz().parse_args()
    ejecutar(argumentos.parametros)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
