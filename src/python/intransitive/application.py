"""Coordinación de la carga de parámetros, búsqueda y generación de resultados."""

import random
import time
from pathlib import Path

from .busqueda import recocido_simulado
from .configuracion import cargar_configuracion
from .informe import formatear_informe, guardar_informe


def ejecutar(ruta_parametros: Path) -> Path:
    """Ejecuta la búsqueda, añade la duración y guarda el informe."""
    inicio = time.perf_counter()
    configuracion = cargar_configuracion(ruta_parametros)
    azar = random.Random(configuracion.semilla)
    solucion = recocido_simulado(configuracion, azar)
    contenido = formatear_informe(solucion, configuracion)
    duracion = time.perf_counter() - inicio
    contenido += f"\n\nTiempo de ejecución: {duracion:.6f} segundos\n"
    guardar_informe(configuracion.path_results, contenido)
    return configuracion.path_results
