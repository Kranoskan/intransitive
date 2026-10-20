import math
import random
import sys
import time
from fractions import Fraction
from pathlib import Path


# ============================================================
# CONFIGURACION
# ============================================================

def cargar_parametros(ruta):
    """Lee, valida y resuelve las rutas de configuración."""
    ruta = Path(ruta).resolve()
    parametros = {}

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

            clave, valor = map(str.strip, linea.split(":", 1))

            if not clave or not valor:
                raise ValueError(
                    f"Parámetro vacío en {ruta}, línea {numero_linea}."
                )

            if clave in parametros:
                raise ValueError(
                    f"Parámetro duplicado '{clave}' en {ruta}, "
                    f"línea {numero_linea}."
                )

            parametros[clave] = valor

    tipos = {
        "n": int,
        "c": int,
        "max_valor": int,
        "reinicios": int,
        "iteraciones": int,
        "temperatura_inicial": float,
        "enfriamiento": float,
        "semilla": int,
    }

    requeridos = {*tipos, "caras_distintas", "path_results"}
    faltantes = requeridos - parametros.keys()
    desconocidos = parametros.keys() - requeridos

    if faltantes:
        raise ValueError(
            f"Faltan parámetros en {ruta}: "
            f"{', '.join(sorted(faltantes))}."
        )

    if desconocidos:
        raise ValueError(
            f"Parámetros desconocidos en {ruta}: "
            f"{', '.join(sorted(desconocidos))}."
        )

    for clave, tipo in tipos.items():
        try:
            parametros[clave] = tipo(parametros[clave])
        except ValueError as error:
            raise ValueError(
                f"El parámetro '{clave}' debe ser de tipo "
                f"{tipo.__name__} en {ruta}."
            ) from error

    valor_booleano = parametros["caras_distintas"].lower()

    if valor_booleano not in ("true", "false"):
        raise ValueError(
            f"El parámetro 'caras_distintas' debe ser true o false "
            f"en {ruta}."
        )

    parametros["caras_distintas"] = valor_booleano == "true"

    ruta_resultados = Path(parametros["path_results"])

    if not ruta_resultados.is_absolute():
        ruta_resultados = ruta.parent / ruta_resultados

    parametros["path_results"] = ruta_resultados.resolve()

    return parametros


# ============================================================
# INICIALIZACION
# ============================================================

inicio_ejecucion = time.perf_counter()

ruta_parametros = (
    Path(sys.argv[1])
    if len(sys.argv) > 1
    else Path(__file__).resolve().parents[2]
    / "config"
    / "Parámetros.txt"
)

if len(sys.argv) > 2:
    raise ValueError("Uso: python main.py [ruta_a_Parámetros.txt]")

parametros = cargar_parametros(ruta_parametros)

n = parametros["n"]
c = parametros["c"]
max_valor = parametros["max_valor"]
reinicios = parametros["reinicios"]
iteraciones = parametros["iteraciones"]
temperatura_inicial = parametros["temperatura_inicial"]
enfriamiento = parametros["enfriamiento"]
semilla = parametros["semilla"]
caras_distintas = parametros["caras_distintas"]
path_results = parametros["path_results"]

if n < 3:
    raise ValueError("Un ciclo estricto requiere n >= 3.")

if c < 1 or max_valor < 1:
    raise ValueError("c y max_valor deben ser positivos.")

if caras_distintas and c > max_valor:
    raise ValueError(
        "No caben c caras distintas entre 1 y max_valor."
    )

if reinicios < 1:
    raise ValueError("reinicios debe ser al menos 1.")

if iteraciones < 0:
    raise ValueError("iteraciones no puede ser negativo.")

if not math.isfinite(temperatura_inicial) or temperatura_inicial <= 0:
    raise ValueError("temperatura_inicial debe ser positiva y finita.")

if not math.isfinite(enfriamiento) or enfriamiento < 0:
    raise ValueError("enfriamiento debe ser finito y no negativo.")

random.seed(semilla)

# Constantes reutilizadas durante las evaluaciones.
RANGO_CARAS = range(1, max_valor + 1)
LONGITUD_HISTOGRAMA = 2 * max_valor + 1
TOTAL_COMBINACIONES = c ** 4
MEDIO = Fraction(1, 2)


# ============================================================
# REPRESENTACION
# Cada pareja contiene dos dados, cada uno con c caras.
# dados[i] = (dado_1, dado_2)
# ============================================================

def crear_dados():
    """Genera las parejas iniciales respetando las restricciones."""
    if caras_distintas:
        generar_dado = lambda: random.sample(RANGO_CARAS, c)
    else:
        generar_dado = lambda: [
            random.randint(1, max_valor) for _ in range(c)
        ]

    return [
        [generar_dado(), generar_dado()]
        for _ in range(n)
    ]


def sumas_pareja(pareja):
    """Calcula el histograma de sumas de una pareja de dados."""
    dado1, dado2 = pareja
    hist = [0] * LONGITUD_HISTOGRAMA

    for x in dado1:
        for y in dado2:
            hist[x + y] += 1

    return hist


def comparar_histogramas(h1, h2):
    """Calcula victorias, empates, derrotas y probabilidad exacta."""
    victorias = 0
    empates = 0
    acumulado_segunda = 0

    # Las sumas posibles comienzan en 2, pues las caras son positivas.
    for s in range(2, LONGITUD_HISTOGRAMA):
        frecuencia_primera = h1[s]
        frecuencia_segunda = h2[s]

        # La primera pareja obtiene una suma mayor.
        victorias += frecuencia_primera * acumulado_segunda

        # Ambas parejas obtienen la misma suma.
        empates += frecuencia_primera * frecuencia_segunda

        acumulado_segunda += frecuencia_segunda

    derrotas = TOTAL_COMBINACIONES - victorias - empates

    # Los empates cuentan como media victoria.
    p = Fraction(
        2 * victorias + empates,
        2 * TOTAL_COMBINACIONES
    )

    return victorias, empates, derrotas, p


def evaluar(dados):
    """Evalúa el ciclo y calcula su puntuación."""
    # Cada histograma se calcula una sola vez por evaluación.
    histogramas = [sumas_pareja(pareja) for pareja in dados]

    resultados = [
        comparar_histogramas(
            histogramas[i],
            histogramas[(i + 1) % n],
        )
        for i in range(n)
    ]

    probabilidades = [float(resultado[3]) for resultado in resultados]
    ventajas = [p - 0.5 for p in probabilidades]

    # Prioriza la peor ventaja del ciclo y después la media.
    puntuacion = min(ventajas) + 0.05 * sum(ventajas) / n

    return puntuacion, resultados


# ============================================================
# MUTACION
# ============================================================

def vecino(dados):
    """Crea una copia vecina cambiando una cara."""
    nuevo = [
        [dado.copy() for dado in pareja]
        for pareja in dados
    ]

    for _ in range(100):
        i = random.randrange(n)
        d = random.randrange(2)
        cara = random.randrange(c)
        valor = random.randint(1, max_valor)

        if caras_distintas:
            if valor in nuevo[i][d] and valor != nuevo[i][d][cara]:
                continue

        nuevo[i][d][cara] = valor
        return nuevo

    return nuevo


# ============================================================
# RECOCIDO SIMULADO
# ============================================================

def recocido():
    """Busca el mejor ciclo mediante reinicios y recocido simulado."""
    mejor_global = None
    mejor_puntuacion = -float("inf")

    for reinicio in range(reinicios):
        actual = crear_dados()
        puntuacion, resultados = evaluar(actual)

        mejor_local = actual
        puntuacion_local = puntuacion
        temperatura = temperatura_inicial

        for _ in range(iteraciones):
            candidato = vecino(actual)
            nueva_puntuacion, nuevos_resultados = evaluar(candidato)

            delta = nueva_puntuacion - puntuacion

            # Acepta mejoras y, ocasionalmente, empeoramientos.
            if delta >= 0 or random.random() < math.exp(
                max(-700, delta / max(temperatura, 1e-12))
            ):
                actual = candidato
                puntuacion = nueva_puntuacion
                resultados = nuevos_resultados

            if puntuacion > puntuacion_local:
                mejor_local = actual
                puntuacion_local = puntuacion

            temperatura *= enfriamiento

        if puntuacion_local > mejor_puntuacion:
            mejor_global = mejor_local
            mejor_puntuacion = puntuacion_local

        # Impresión por pantalla desactivada.
        # print(
        #     f"Reinicio {reinicio + 1}/{reinicios}: "
        #     f"mejor puntuacion = {puntuacion_local:.6f}"
        # )

    return mejor_global


# ============================================================
# VERIFICACION Y RESULTADOS
# ============================================================

def mostrar_resultados(dados):
    """Genera el informe completo de resultados."""
    puntuacion, resultados = evaluar(dados)
    lineas = []

    lineas.append("=== DADOS ENCONTRADOS ===")
    lineas.append("")

    for i, pareja in enumerate(dados, start=1):
        lineas.append(f"Pareja {i}:")
        lineas.append(f"  Dado 1: {pareja[0]}")
        lineas.append(f"  Dado 2: {pareja[1]}")
        lineas.append("")

    lineas.append("=== COMPROBACION EXHAUSTIVA ===")

    ciclo_valido = True

    for i, (victorias, empates, derrotas, p) in enumerate(resultados):
        j = (i + 1) % n
        total = victorias + empates + derrotas

        assert total == TOTAL_COMBINACIONES

        lineas.extend([
            "",
            f"Pareja {i + 1} contra pareja {j + 1}:",
            f"  Victorias primera: {victorias}",
            f"  Empates:           {empates}",
            f"  Victorias segunda: {derrotas}",
            f"  Total:             {total}",
            f"  Probabilidad exacta: {p}",
            f"  Porcentaje:         {100 * float(p):.6f}%"
        ])

        if p <= MEDIO:
            ciclo_valido = False

    lineas.extend([
        "",
        f"Ciclo no transitivo: {ciclo_valido}",
        f"Puntuacion: {puntuacion}"
    ])

    if not ciclo_valido:
        lineas.append(
            "No se encontró un ciclo válido en esta búsqueda. "
            "Prueba más reinicios o iteraciones."
        )

    # Impresión del informe por pantalla desactivada.
    # print("\n".join(lineas))

    return "\n".join(lineas)


# ============================================================
# EJECUCION Y GUARDADO
# ============================================================

solucion = recocido()

# Impresión por pantalla desactivada.
# print(f"Parámetros cargados desde: {ruta_parametros.resolve()}")

texto_resultados = mostrar_resultados(solucion)

duracion_ejecucion = time.perf_counter() - inicio_ejecucion

texto_resultados += (
    f"\n\nTiempo de ejecución: {duracion_ejecucion:.6f} segundos\n"
)

# Impresión del informe por pantalla desactivada.
# print(texto_resultados, end="")

# Guardar los resultados en el fichero configurado.
path_results.parent.mkdir(parents=True, exist_ok=True)
path_results.write_text(texto_resultados, encoding="utf-8")

# Confirmación por pantalla desactivada.
# print(f"Resultados guardados en: {path_results}")
