#include "report.h"

#include <cstdio>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../analysis/cycle.h"
#include "../platform/paths.h"

namespace intransitive {
namespace {

// Escribe las caras de un dado con el formato de lista del informe.
void write_die(std::ostream& output, const Die& die) {
    output << "[";
    for (std::size_t index = 0; index < die.size(); ++index) {
        if (index > 0) {
            output << ", ";
        }
        output << die[index];
    }
    output << "]";
}

}

// Genera el informe legible con dados y verificación exhaustiva del ciclo.
std::string format_report(
    const DicePairs& dice,
    const Parameters& parameters
) {
    const Evaluation evaluation = evaluate_cycle(dice, parameters);
    std::ostringstream output;
    output << "\n=== DADOS ENCONTRADOS ===\n";

    for (std::size_t index = 0; index < dice.size(); ++index) {
        output << "\nPareja " << index + 1 << ":\n";
        output << "  Dado 1: ";
        write_die(output, dice[index].first);
        output << "\n  Dado 2: ";
        write_die(output, dice[index].second);
        output << "\n";
    }

    output << "\n=== COMPROBACION EXHAUSTIVA ===\n";
    bool valid_cycle = true;
    std::uint64_t total = 1;
    for (int i = 0; i < 4; ++i) {
        total *= parameters.faces_per_die;
    }

    for (std::size_t index = 0;
         index < evaluation.comparisons.size();
         ++index) {
        const Comparison& comparison = evaluation.comparisons[index];
        const std::size_t next = (index + 1) % parameters.pair_count;
        if (comparison.wins + comparison.ties + comparison.losses != total) {
            throw std::logic_error(
                "El total de combinaciones de la comparación no coincide."
            );
        }

        output << "\nPareja " << index + 1 << " contra pareja "
               << next + 1 << ":\n";
        output << "  Victorias primera: " << comparison.wins << "\n";
        output << "  Empates:           " << comparison.ties << "\n";
        output << "  Victorias segunda: " << comparison.losses << "\n";
        output << "  Total:             " << total << "\n";
        output << "  Probabilidad exacta: "
               << comparison.probability_numerator << "/"
               << comparison.probability_denominator << "\n";
        output << "  Porcentaje: " << std::fixed << std::setprecision(6)
               << 100.0 * comparison.probability() << "%\n";

        if (comparison.probability_numerator <=
            comparison.probability_denominator / 2) {
            valid_cycle = false;
        }
    }

    output << "\nCiclo no transitivo: "
           << (valid_cycle ? "True" : "False") << "\n";
    output << "Puntuacion: " << std::fixed << std::setprecision(6)
           << evaluation.score << "\n";

    if (!valid_cycle) {
        output << "No se encontró un ciclo válido en esta búsqueda. "
                  "Prueba más reinicios o iteraciones.\n";
    }
    return output.str();
}

// Crea las carpetas necesarias y escribe todos los bytes del informe.
void write_report(
    const std::string& output_path,
    const std::string& report
) {
    const std::string parent = parent_path(output_path);
    if (!parent.empty()) {
        std::string error_message;
        if (!create_directories(parent, error_message)) {
            throw std::runtime_error(
                "No se pudo crear la carpeta de resultados " + parent +
                ": " + error_message
            );
        }
    }

    FILE* output = open_path(output_path, "wb");
    if (output == NULL) {
        throw std::runtime_error(
            "No se pudo abrir el archivo de resultados: " + output_path
        );
    }
    const std::size_t written =
        std::fwrite(report.data(), 1, report.size(), output);
    const int close_result = std::fclose(output);
    if (written != report.size() || close_result != 0) {
        throw std::runtime_error(
            "Error al escribir el archivo de resultados: " + output_path
        );
    }
}

}
