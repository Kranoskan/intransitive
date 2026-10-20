"""Búsqueda de ciclos mediante recocido simulado."""

import math
import random

from .analisis import evaluar_ciclo
from .configuracion import Configuracion
from .dados import TipoDados, crear_dados, crear_vecino


def recocido_simulado(
    configuracion: Configuracion,
    azar: random.Random,
) -> TipoDados:
    """Devuelve la mejor solución encontrada en todos los reinicios."""
    mejor_global: TipoDados | None = None
    mejor_puntuacion = -float("inf")

    for _ in range(configuracion.reinicios):
        actual = crear_dados(configuracion, azar)
        evaluacion = evaluar_ciclo(actual, configuracion)
        mejor_local = actual
        mejor_puntuacion_local = evaluacion.puntuacion
        temperatura = configuracion.temperatura_inicial

        for _ in range(configuracion.iteraciones):
            candidato = crear_vecino(actual, configuracion, azar)
            nueva_evaluacion = evaluar_ciclo(candidato, configuracion)
            delta = nueva_evaluacion.puntuacion - evaluacion.puntuacion

            aceptar = delta >= 0 or azar.random() < math.exp(
                max(-700, delta / max(temperatura, 1e-12))
            )
            if aceptar:
                actual = candidato
                evaluacion = nueva_evaluacion

            if evaluacion.puntuacion > mejor_puntuacion_local:
                mejor_local = actual
                mejor_puntuacion_local = evaluacion.puntuacion

            temperatura *= configuracion.enfriamiento

        if mejor_puntuacion_local > mejor_puntuacion:
            mejor_global = mejor_local
            mejor_puntuacion = mejor_puntuacion_local

    if mejor_global is None:
        raise RuntimeError("La búsqueda no produjo ninguna solución.")
    return mejor_global
