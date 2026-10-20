"""Carga, conversión y validación de los parámetros de ejecución."""

import math
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Configuracion:
    """Parámetros de la búsqueda y ubicación del informe de resultados."""

    parejas: int
    caras: int
    max_valor: int
    reinicios: int
    iteraciones: int
    temperatura_inicial: float
    enfriamiento: float
    semilla: int
    caras_distintas: bool
    path_results: Path


def leer_opciones(ruta: Path) -> dict[str, str]:
    """Lee líneas clave:valor y rechaza entradas vacías o duplicadas."""
    opciones: dict[str, str] = {}
    with ruta.open(encoding="utf-8-sig") as archivo:
        for numero_linea, linea in enumerate(archivo, start=1):
            linea = linea.split("#", 1)[0].strip()
            if not linea:
                continue
            if ":" not in linea:
                raise ValueError(
                    f"Formato inválido en {ruta}, línea {numero_linea}: "
                    "se esperaba clave:valor."
                )

            clave, valor = (parte.strip() for parte in linea.split(":", 1))
            if not clave or not valor:
                raise ValueError(
                    f"Parámetro vacío en {ruta}, línea {numero_linea}."
                )
            if clave in opciones:
                raise ValueError(
                    f"Parámetro duplicado '{clave}' en {ruta}, "
                    f"línea {numero_linea}."
                )
            opciones[clave] = valor
    return opciones


def convertir_opciones(opciones: dict[str, str], ruta: Path) -> Configuracion:
    """Convierte las opciones de texto a sus tipos y valida los nombres."""
    tipos_numericos: dict[str, type[int] | type[float]] = {
        "n": int,
        "c": int,
        "max_valor": int,
        "reinicios": int,
        "iteraciones": int,
        "temperatura_inicial": float,
        "enfriamiento": float,
        "semilla": int,
    }
    requeridos = {*tipos_numericos, "caras_distintas", "path_results"}
    faltantes = requeridos - opciones.keys()
    desconocidos = opciones.keys() - requeridos

    if faltantes:
        raise ValueError(
            f"Faltan parámetros en {ruta}: {', '.join(sorted(faltantes))}."
        )
    if desconocidos:
        raise ValueError(
            f"Parámetros desconocidos en {ruta}: "
            f"{', '.join(sorted(desconocidos))}."
        )

    convertidos: dict[str, int | float] = {}
    for clave, tipo in tipos_numericos.items():
        try:
            convertidos[clave] = tipo(opciones[clave])
        except ValueError as error:
            raise ValueError(
                f"El parámetro '{clave}' debe ser de tipo "
                f"{tipo.__name__} en {ruta}."
            ) from error

    valor_booleano = opciones["caras_distintas"].lower()
    if valor_booleano not in ("true", "false"):
        raise ValueError(
            f"El parámetro 'caras_distintas' debe ser true o false en {ruta}."
        )

    ruta_resultados = Path(opciones["path_results"])
    if not ruta_resultados.is_absolute():
        ruta_resultados = ruta.parent / ruta_resultados

    return Configuracion(
        parejas=int(convertidos["n"]),
        caras=int(convertidos["c"]),
        max_valor=int(convertidos["max_valor"]),
        reinicios=int(convertidos["reinicios"]),
        iteraciones=int(convertidos["iteraciones"]),
        temperatura_inicial=float(convertidos["temperatura_inicial"]),
        enfriamiento=float(convertidos["enfriamiento"]),
        semilla=int(convertidos["semilla"]),
        caras_distintas=valor_booleano == "true",
        path_results=ruta_resultados.resolve(),
    )


def validar_configuracion(configuracion: Configuracion) -> None:
    """Comprueba que los parámetros cumplen las restricciones del algoritmo."""
    if configuracion.parejas < 3:
        raise ValueError("Un ciclo estricto requiere n >= 3.")
    if configuracion.caras < 1 or configuracion.max_valor < 1:
        raise ValueError("c y max_valor deben ser positivos.")
    if (
        configuracion.caras_distintas
        and configuracion.caras > configuracion.max_valor
    ):
        raise ValueError(
            "No caben c caras distintas entre 1 y max_valor."
        )
    if configuracion.reinicios < 1:
        raise ValueError("reinicios debe ser al menos 1.")
    if configuracion.iteraciones < 0:
        raise ValueError("iteraciones no puede ser negativo.")
    if (
        not math.isfinite(configuracion.temperatura_inicial)
        or configuracion.temperatura_inicial <= 0
    ):
        raise ValueError(
            "temperatura_inicial debe ser positiva y finita."
        )
    if (
        not math.isfinite(configuracion.enfriamiento)
        or configuracion.enfriamiento < 0
    ):
        raise ValueError("enfriamiento debe ser finito y no negativo.")


def cargar_configuracion(ruta: Path) -> Configuracion:
    """Carga el archivo de parámetros y devuelve una configuración validada."""
    ruta_resuelta = ruta.resolve()
    configuracion = convertir_opciones(leer_opciones(ruta_resuelta), ruta_resuelta)
    validar_configuracion(configuracion)
    return configuracion
