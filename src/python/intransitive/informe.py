"""Presentación y escritura del informe de resultados."""

from pathlib import Path

from .analisis import Evaluacion, evaluar_ciclo
from .configuracion import Configuracion
from .dados import TipoDados


def formatear_informe(
    dados: TipoDados,
    configuracion: Configuracion,
    evaluacion: Evaluacion | None = None,
) -> str:
    """Formatea los dados y los resultados de sus comparaciones."""
    if evaluacion is None:
        evaluacion = evaluar_ciclo(dados, configuracion)

    lineas = ["=== DADOS ENCONTRADOS ===", ""]
    for indice, pareja in enumerate(dados, start=1):
        lineas.extend([
            f"Pareja {indice}:",
            f"  Dado 1: {pareja[0]}",
            f"  Dado 2: {pareja[1]}",
            "",
        ])

    lineas.append("=== COMPROBACION EXHAUSTIVA ===")
    ciclo_valido = True
    total_esperado = configuracion.caras**4

    for indice, resultado in enumerate(evaluacion.resultados):
        siguiente = (indice + 1) % configuracion.parejas
        total = resultado.victorias + resultado.empates + resultado.derrotas
        if total != total_esperado:
            raise AssertionError(
                "El total de combinaciones no coincide con c ** 4."
            )

        lineas.extend([
            "",
            f"Pareja {indice + 1} contra pareja {siguiente + 1}:",
            f"  Victorias primera: {resultado.victorias}",
            f"  Empates:           {resultado.empates}",
            f"  Victorias segunda: {resultado.derrotas}",
            f"  Total:             {total}",
            f"  Probabilidad exacta: {resultado.probabilidad}",
            f"  Porcentaje:         "
            f"{100 * float(resultado.probabilidad):.6f}%",
        ])
        if resultado.probabilidad <= 0.5:
            ciclo_valido = False

    lineas.extend([
        "",
        f"Ciclo no transitivo: {ciclo_valido}",
        f"Puntuacion: {evaluacion.puntuacion}",
    ])
    if not ciclo_valido:
        lineas.append(
            "No se encontró un ciclo válido en esta búsqueda. "
            "Prueba más reinicios o iteraciones."
        )

    return "\n".join(lineas)


def guardar_informe(ruta: Path, contenido: str) -> None:
    """Crea la carpeta de destino si hace falta y guarda el informe UTF-8."""
    ruta.parent.mkdir(parents=True, exist_ok=True)
    ruta.write_text(contenido, encoding="utf-8")
