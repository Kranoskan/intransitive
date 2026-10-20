# intransitive

Busca ciclos no transitivos de parejas de dados mediante recocido simulado.
La versión C++ está separada por responsabilidades:

```text
src/
  cpp/
    main.cpp
    application.cpp
    algorithm/       # búsqueda por recocido simulado
    analysis/        # comparación de histogramas y puntuaciones
    config/          # lectura y validación de parámetros
    model/           # dados, parejas e histogramas
    output/          # generación y escritura del informe
    platform/        # concurrencia, rutas y acceso a archivos
  python/
    main.py            # punto de entrada
    intransitive/
      configuracion.py  # lectura y validación de parámetros
      dados.py          # generación y mutación de dados
      analisis.py       # histogramas y comparaciones
      busqueda.py       # recocido simulado
      informe.py        # formato y escritura de resultados
      application.py    # coordinación del flujo de ejecución
config/
  Parámetros.txt
```

## Compilar y ejecutar C++

Requiere MinGW con soporte para C++11 y GDB para depurar en VS Code.
Desde la carpeta del proyecto, ejecuta:

```powershell
g++ -std=c++11 -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8 `
  src/cpp/main.cpp src/cpp/application.cpp `
  src/cpp/algorithm/simulated_annealing.cpp `
  src/cpp/analysis/cycle.cpp src/cpp/config/parameters.cpp `
  src/cpp/model/dice.cpp src/cpp/platform/parallel.cpp `
  src/cpp/platform/paths.cpp `
  src/cpp/output/report.cpp `
  -o intransitive.exe
.\intransitive.exe config\Parámetros.txt
```

También puedes usar la tarea **C/C++: build intransitive** con
`Ctrl+Shift+B`, ejecutar **C/C++: run intransitive**, o iniciar la
configuración **Debug intransitive (C++)** con `F5`.

La búsqueda ejecuta los reinicios en paralelo en Windows. Cada reinicio usa
una semilla derivada del valor configurado y su índice, por lo que el resultado
es reproducible y no depende del orden de ejecución. La evaluación reutiliza
los histogramas y el almacenamiento del candidato entre iteraciones para
reducir asignaciones. En otras plataformas la búsqueda se ejecuta en serie.

Si no se proporciona una ruta de configuración, el programa busca
`config/Parámetros.txt` junto al ejecutable. Se puede pasar otra ruta como
primer argumento. Los parámetros usan el formato `clave:valor`; `path_results`
indica dónde guardar el informe. Las rutas de resultados relativas se resuelven
respecto al archivo de parámetros. En la configuración incluida, el informe
queda en `config/resultados.txt`. El informe incluye el tiempo total desde la
carga de parámetros hasta el análisis final, en segundos con seis decimales.

## Configuración

El archivo `config/Parámetros.txt` incluye los parámetros del recocido simulado,
la cantidad de caras y el rango de valores, además de si las caras de cada dado
deben ser distintas. Reduce `reinicios` y `iteraciones` para hacer ejecuciones
de prueba más cortas.

La versión Python sigue disponible en `src/python/main.py`, usa el mismo
formato de parámetros e incluye el tiempo de ejecución en su informe:

```powershell
py src/python/main.py
py src/python/main.py config\Parámetros.txt
```

El código Python está separado en `src/python/intransitive/`: configuración,
modelo de dados, análisis, búsqueda e informe tienen módulos independientes.
