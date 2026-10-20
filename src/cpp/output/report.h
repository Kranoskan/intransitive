#ifndef INTRANSITIVE_OUTPUT_REPORT_H
#define INTRANSITIVE_OUTPUT_REPORT_H

#include <string>

#include "../config/parameters.h"
#include "../model/dice.h"

namespace intransitive {

// Formatea los dados, las comparaciones exhaustivas y la validez del ciclo.
std::string format_report(
    const DicePairs& dice,
    const Parameters& parameters
);

// Crea la carpeta de destino cuando haga falta y escribe el informe completo.
void write_report(
    const std::string& output_path,
    const std::string& report
);

}

#endif
