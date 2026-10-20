"""Histogramas, comparaciones exhaustivas y puntuación del ciclo."""

from dataclasses import dataclass
from fractions import Fraction

from .configuracion import Configuracion
from .dados import TipoDados, TipoPareja


@dataclass(frozen=True)
class Comparacion:
    """Conteos exhaustivos y probabilidad efectiva de una comparación."""

    victorias: int
    empates: int
    derrotas: int
    probabilidad: Fraction


@dataclass(frozen=True)
class Evaluacion:
    """Puntuación calculada y comparaciones que forman el ciclo."""

    puntuacion: float
    resultados: list[Comparacion]


def calcular_histograma(
    pareja: TipoPareja,
    configuracion: Configuracion,
) -> list[int]:
    """Cuenta las frecuencias de suma entre todas las caras de una pareja."""
    dado1, dado2 = pareja
    histograma = [0] * (2 * configuracion.max_valor + 1)

    for valor1 in dado1:
        for valor2 in dado2:
            histograma[valor1 + valor2] += 1

    return histograma


def comparar_histogramas(
    primero: list[int],
    segundo: list[int],
    total_combinaciones: int,
) -> Comparacion:
    """Calcula victorias, empates, derrotas y probabilidad exacta."""
    victorias = 0
    empates = 0
    acumulado_segundo = 0

    for suma in range(2, len(primero)):
        frecuencia_primera = primero[suma]
        frecuencia_segunda = segundo[suma]
        victorias += frecuencia_primera * acumulado_segundo
        empates += frecuencia_primera * frecuencia_segunda
        acumulado_segundo += frecuencia_segunda

    derrotas = total_combinaciones - victorias - empates
    probabilidad = Fraction(
        2 * victorias + empates,
        2 * total_combinaciones,
    )
    return Comparacion(victorias, empates, derrotas, probabilidad)


def evaluar_ciclo(
    dados: TipoDados,
    configuracion: Configuracion,
) -> Evaluacion:
    """Evalúa las comparaciones consecutivas y puntúa el ciclo completo."""
    total_combinaciones = configuracion.caras**4
    histogramas = [
        calcular_histograma(pareja, configuracion)
        for pareja in dados
    ]
    resultados = [
        comparar_histogramas(
            histogramas[indice],
            histogramas[(indice + 1) % configuracion.parejas],
            total_combinaciones,
        )
        for indice in range(configuracion.parejas)
    ]

    ventajas = [
        float(resultado.probabilidad) - 0.5
        for resultado in resultados
    ]
    puntuacion = min(ventajas) + 0.05 * sum(ventajas) / configuracion.parejas
    return Evaluacion(puntuacion, resultados)
