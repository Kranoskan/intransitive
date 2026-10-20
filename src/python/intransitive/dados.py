"""Creación de dados, parejas y mutaciones para la búsqueda."""

import random

from .configuracion import Configuracion

TipoDado = list[int]
TipoPareja = list[TipoDado]
TipoDados = list[TipoPareja]


def crear_dado(configuracion: Configuracion, azar: random.Random) -> TipoDado:
    """Genera un dado aleatorio conforme a la regla de caras configurada."""
    if configuracion.caras_distintas:
        return azar.sample(
            range(1, configuracion.max_valor + 1),
            configuracion.caras,
        )
    return [
        azar.randint(1, configuracion.max_valor)
        for _ in range(configuracion.caras)
    ]


def crear_dados(configuracion: Configuracion, azar: random.Random) -> TipoDados:
    """Genera todas las parejas iniciales de dados."""
    return [
        [crear_dado(configuracion, azar), crear_dado(configuracion, azar)]
        for _ in range(configuracion.parejas)
    ]


def crear_vecino(
    dados: TipoDados,
    configuracion: Configuracion,
    azar: random.Random,
) -> TipoDados:
    """Copia los dados y muta una cara sin romper las restricciones."""
    vecino = [[dado.copy() for dado in pareja] for pareja in dados]

    for _ in range(100):
        indice_pareja = azar.randrange(configuracion.parejas)
        indice_dado = azar.randrange(2)
        indice_cara = azar.randrange(configuracion.caras)
        valor = azar.randint(1, configuracion.max_valor)
        dado = vecino[indice_pareja][indice_dado]

        if (
            configuracion.caras_distintas
            and valor in dado
            and valor != dado[indice_cara]
        ):
            continue

        dado[indice_cara] = valor
        return vecino

    return vecino
