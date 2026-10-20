#include "application.h"

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "algorithm/simulated_annealing.h"
#include "config/parameters.h"
#include "model/dice.h"
#include "output/report.h"
#include "platform/paths.h"

namespace intransitive {

int run_application(int argc, char* argv[]) {
    // Mide desde la carga de argumentos hasta el cálculo del informe final.
    const std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    try {
        if (argc > 2) {
            throw std::invalid_argument(
                "Uso: intransitive [ruta_a_Parámetros.txt]"
            );
        }

        const std::string config_path = argc == 2
            ? argument_to_utf8(argv[1])
            : executable_directory(argv[0]) + "/config/Parámetros.txt";

        const Parameters parameters = load_parameters(config_path);
        validate_parameters(parameters);

        const DicePairs solution = find_best_cycle(parameters);
        std::string report = format_report(solution, parameters);
        const double elapsed_seconds =
            std::chrono::duration_cast<std::chrono::duration<double> >(
                std::chrono::steady_clock::now() - start
            ).count();
        std::ostringstream elapsed_report;
        elapsed_report << "\nTiempo de ejecución: "
                       << std::fixed << std::setprecision(6)
                       << elapsed_seconds << " segundos\n";
        report += elapsed_report.str();

        //std::cout << "Parámetros cargados desde: " << config_path << "\n";
        //std::cout << report;
        write_report(parameters.results_path, report);
        //std::cout << "Resultados guardados en: "
        //          << parameters.results_path << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}

}
