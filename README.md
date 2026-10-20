# Dados no transitivos | Intransitive Dice

## Español

### Piedra, papel o tijera, con una, dos o tres vueltas de tuerca

Solemos pensar que, si A vence a B y B vence a C, entonces A también debería vencer a C. Pero no siempre es así: piedra, papel o tijera es un ejemplo cotidiano de una relación no transitiva.

Los dados ofrecen otro ejemplo interesante. Supongamos un juego en el que al lanzar dos dados, gana el que obtiene la suma más alta. Sin embargo, tener una media mayor no implica necesariamente tener más probabilidades de ganar. Por eso, las probabilidades de victoria entre varias opciones pueden formar un ciclo, en lugar de un orden universal.

Por ejemplo, estos dados de seis caras:

```text
A: {2, 2, 4, 4, 10, 10}
B: {0, 0, 6, 6, 8, 8}
C: {3, 3, 5, 5, 7, 7}
```

tienen medias `A = 16/3`, `C = 15/3` y `B = 14/3`, de modo que el orden por media es `A > C > B`. No obstante, al considerar todas las tiradas posibles, aparece el ciclo:

```text
A vence a B: 5/9 ≈ 55,56 %
B vence a C: 5/9 ≈ 55,56 %
C vence a A: 5/9 ≈ 55,56 %
```

Este ejemplo muestra que una sola medida, como la media, no siempre describe cómo se comportan las opciones cuando se enfrentan. La teoría y los datos son valiosos; contrastarlos con experimentos también ayuda a desarrollar criterio.

### El experimento: ciclos de parejas de dados

Este proyecto implementa en **C++ y Python** un algoritmo de **recocido simulado** para buscar ciclos de elementos no transitivos. Se le ha decidido dar una vuelta de tuerca y cada elemento no es un dado, cada elemento es una **pareja de dados**, no un dado individual. Por tanto, el resultado no es un conjunto de dados individuales no transitivos: es una secuencia cíclica de parejas de dados en la que cada pareja tiene ventaja sobre la siguiente.

Si llamamos a las parejas `P₁, P₂, …, Pₙ`, el algoritmo busca un ciclo en el que:

```text
P₁ vence a P₂
P₂ vence a P₃
...
Pₙ vence a P₁
```

Cada pareja se compara con la siguiente mediante las sumas que se obtienen al enfrentar sus dados. Se enumeran exhaustivamente todas las combinaciones de caras; una suma mayor cuenta como victoria y los empates cuentan como media victoria. El ciclo solo se considera **válido** si cada pareja vence a la siguiente con una probabilidad estrictamente superior a `1/2`.

La búsqueda favorece los ciclos cuya comparación menos favorable tenga una ventaja grande y utiliza la ventaja media como criterio secundario. El programa guarda en el informe las parejas encontradas, los conteos exhaustivos, las probabilidades exactas, la puntuación y el tiempo de ejecución.

El algoritmo es heurístico: **no garantiza encontrar un ciclo válido para cualquier configuración**. El resultado depende del espacio de búsqueda, la semilla aleatoria y los parámetros del recocido simulado.

### Comparar C++ y Python

En las pruebas de desarrollo de este proyecto se observó que:

- La implementación Python ocupa aproximadamente una cuarta parte de las líneas de la versión C++.
- La versión C++ se ejecutó entre **100 y 250 veces más rápido** en las pruebas realizadas.

Son resultados observados, no una garantía para todos los equipos o configuraciones. El tiempo depende del hardware, del compilador y sus opciones, de la versión de Python, del tamaño de la búsqueda y de la carga del sistema.

Para comparar ambas versiones, ejecútalas con el mismo contenido de `config/Parámetros.txt` y en condiciones similares. Después, compara el campo `Tiempo de ejecución` que cada implementación añade a su archivo de resultados. La implementación C++ ejecuta los reinicios en paralelo en Windows; Python los ejecuta secuencialmente. Por tanto, las cifras comparan las implementaciones completas, no únicamente los lenguajes.

Antes de comparar, configura una ruta absoluta distinta en `path_results` para cada ejecución o guarda una copia del informe tras cada una, para evitar que la segunda ejecución sobrescriba la primera. La semilla permite repetir la búsqueda dentro de cada implementación, pero C++ y Python no tienen por qué producir las mismas parejas: utilizan generadores aleatorios distintos.

### Estructura del proyecto

```text
config/
  Parámetros.txt
src/
  cpp/
    main.cpp
    application.cpp
    algorithm/       # recocido simulado
    analysis/        # histogramas, probabilidades y puntuación
    config/          # lectura y validación de parámetros
    model/           # dados, parejas y mutaciones
    output/          # generación y escritura del informe
    platform/        # concurrencia y operaciones de rutas/archivos
  python/
    main.py          # punto de entrada
    intransitive/
      configuracion.py
      dados.py
      analisis.py
      busqueda.py
      informe.py
      application.py
```

### Ejecutar Python

Requiere Python 3.10 o posterior. Desde la raíz del proyecto:

```powershell
py src/python/main.py
```

También se puede indicar otro archivo de parámetros:

```powershell
py src/python/main.py "C:\ruta\Parámetros.txt"
```

### Compilar y ejecutar C++

Requiere MinGW con soporte para C++11. Desde la raíz del proyecto:

```powershell
g++ -std=c++11 -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8 `
  src/cpp/main.cpp src/cpp/application.cpp `
  src/cpp/algorithm/simulated_annealing.cpp `
  src/cpp/analysis/cycle.cpp src/cpp/config/parameters.cpp `
  src/cpp/model/dice.cpp src/cpp/platform/parallel.cpp `
  src/cpp/platform/paths.cpp src/cpp/output/report.cpp `
  -o intransitive.exe
.\intransitive.exe config\Parámetros.txt
```

En VS Code también están disponibles la tarea **C/C++: build intransitive**, la tarea **C/C++: run intransitive** y la configuración de depuración **Debug intransitive (C++)**.

### Configuración y resultados

Los parámetros se escriben como `clave:valor` en `config/Parámetros.txt`. El parámetro `path_results` define el archivo donde se guarda el informe; las rutas relativas se resuelven respecto al archivo de configuración. Usa `reinicios` e `iteraciones` para ajustar cuánto explora el recocido simulado, y `semilla` para repetir una búsqueda.

Por defecto, Python busca `config/Parámetros.txt` desde la raíz del proyecto. C++ busca esa configuración junto al ejecutable. Ambos programas aceptan una ruta explícita como argumento.

El informe incluye las parejas de dados encontradas, los resultados de las comparaciones entre parejas consecutivas, las probabilidades exactas, la puntuación y el tiempo de ejecución. Comprueba que el ciclo sea válido verificando que **todas** las probabilidades de victoria sean mayores que `1/2`; una puntuación alta, por sí sola, no sustituye esa comprobación.

---

## English

### Rock, paper, scissors—with one, two, or three extra twists

We tend to assume that if A beats B and B beats C, then A should beat C as well. But that is not always true: rock, paper, scissors is a familiar example of a non-transitive relationship.

Dice provide another interesting example. Let's imagine a game in which two dice are rolled, and the player with the highest total wins. However, having a higher average does not necessarily mean having a better chance of winning. Therefore, the probabilities of winning among various options can form a cycle, rather than a universal order.

For example, consider these six-sided dice:

```text
A: {2, 2, 4, 4, 10, 10}
B: {0, 0, 6, 6, 8, 8}
C: {3, 3, 5, 5, 7, 7}
```

Their means are `A = 16/3`, `C = 15/3`, and `B = 14/3`, giving the average ranking `A > C > B`. However, considering every possible roll produces this cycle:

```text
A beats B: 5/9 ≈ 55.56%
B beats C: 5/9 ≈ 55.56%
C beats A: 5/9 ≈ 55.56%
```

This example shows why a single measure, such as the mean, cannot always describe how options perform against one another. Theory and data are valuable; testing them through experiments also helps build sound judgment.

### The experiment: cycles of dice pairs

This project implements **simulated annealing** in **C++ and Python** to search for cycles of non-transitive elements. I've decided to take it a step further: each element isn't a die; each element is a **pair of dice**, not a single die. Therefore, the result isn't a set of individual, non-transitive dice: it's a cyclic sequence of pairs of dice in which each pair has an advantage over the next one.

Let the pairs be `P₁, P₂, …, Pₙ`. The algorithm searches for a cycle satisfying:

```text
P₁ beats P₂
P₂ beats P₃
...
Pₙ beats P₁
```

Each pair is compared with the next by considering the totals obtained when their dice are rolled. Every possible face combination is enumerated exhaustively; a higher total counts as a win, and ties count as half a win. A cycle is considered **valid** only if every pair beats the next with a probability strictly greater than `1/2`.

The search favors cycles whose weakest comparison has a large advantage and uses the average advantage as a secondary criterion. The program saves the resulting pairs, exhaustive counts, exact probabilities, score, and execution time in a report.

The algorithm is heuristic: **it does not guarantee that a valid cycle will be found for every configuration**. The result depends on the search space, random seed, and simulated-annealing parameters.

### Comparing C++ and Python

Development tests for this project observed that:

- The Python implementation uses approximately one quarter as many lines of code as the C++ version.
- The C++ version ran **100 to 250 times faster** in the tests performed.

These are observed results, not a guarantee for every machine or configuration. Runtime depends on hardware, compiler and compiler options, Python version, search size, and system load.

To compare the implementations, run both with the same `config/Parámetros.txt` contents under similar conditions. Then compare the `Tiempo de ejecución` field written to each results file. The C++ implementation runs restarts concurrently on Windows, while Python runs them sequentially. These figures therefore compare the complete implementations, not just the languages.

Before benchmarking, use a different absolute path in `path_results` for each run or save a copy of each report so the second run does not overwrite the first. The seed makes a search repeatable within each implementation, but C++ and Python need not produce identical pairs because they use different random-number generators.

### Project structure

```text
config/
  Parámetros.txt
src/
  cpp/
    main.cpp
    application.cpp
    algorithm/       # simulated annealing
    analysis/        # histograms, probabilities, and scoring
    config/          # parameter loading and validation
    model/           # dice, pairs, and mutations
    output/          # report generation and writing
    platform/        # concurrency and path/file operations
  python/
    main.py          # entry point
    intransitive/
      configuracion.py
      dados.py
      analisis.py
      busqueda.py
      informe.py
      application.py
```

### Run Python

Requires Python 3.10 or newer. From the project root:

```powershell
py src/python/main.py
```

You can also provide a different parameter file:

```powershell
py src/python/main.py "C:\path\Parámetros.txt"
```

### Build and run C++

Requires MinGW with C++11 support. From the project root:

```powershell
g++ -std=c++11 -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8 `
  src/cpp/main.cpp src/cpp/application.cpp `
  src/cpp/algorithm/simulated_annealing.cpp `
  src/cpp/analysis/cycle.cpp src/cpp/config/parameters.cpp `
  src/cpp/model/dice.cpp src/cpp/platform/parallel.cpp `
  src/cpp/platform/paths.cpp src/cpp/output/report.cpp `
  -o intransitive.exe
.\intransitive.exe config\Parámetros.txt
```

VS Code also provides the **C/C++: build intransitive** and **C/C++: run intransitive** tasks, plus the **Debug intransitive (C++)** launch configuration.

### Configuration and results

Parameters use the `key:value` format in `config/Parámetros.txt`. The `path_results` parameter specifies the report file; relative paths are resolved from the parameter file's directory. Adjust `reinicios` and `iteraciones` to control how much the simulated-annealing search explores, and use `semilla` to repeat a search.

By default, Python looks for `config/Parámetros.txt` from the project root. C++ looks for that configuration next to its executable. Both programs accept an explicit configuration path as an argument.

The report includes the resulting dice pairs, comparisons between consecutive pairs, exact probabilities, score, and execution time. To verify that a cycle is valid, check that **every** win probability is greater than `1/2`; a high score alone does not replace this check.
'''

out = Path("/mnt/data/README_mejorado.md")
pypandoc.convert_text(content, 'md', format='md', outputfile=str(out), extra_args=['--standalone'])
print(f"Creado: {out} ({out.stat().st_size} bytes)")
