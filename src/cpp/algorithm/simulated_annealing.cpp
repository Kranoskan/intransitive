#include "simulated_annealing.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>

#include "../analysis/cycle.h"
#include "../platform/parallel.h"

namespace intransitive {
namespace {

struct RestartResult {
    DicePairs dice;
    double score = -std::numeric_limits<double>::infinity();
};

struct SearchContext {
    const Parameters* parameters;
    std::vector<RestartResult>* results;
    std::vector<std::exception_ptr>* errors;
};

void run_restart(std::size_t restart, void* raw_context) {
    SearchContext& context = *static_cast<SearchContext*>(raw_context);
    const Parameters& parameters = *context.parameters;

    try {
        // Cada reinicio tiene un motor propio; su secuencia no depende del planificador.
        const std::uint32_t restart_seed =
            parameters.seed + static_cast<std::uint32_t>(restart);
        std::mt19937 random_engine(restart_seed);
        EvaluationWorkspace workspace(parameters);
        DicePairs current = create_dice(parameters, random_engine);
        double current_score =
            evaluate_cycle_score(current, parameters, workspace);
        DicePairs best_local = current;
        double best_local_score = current_score;
        DicePairs candidate;
        double temperature = parameters.initial_temperature;

        for (std::size_t step = 0; step < parameters.iterations; ++step) {
            create_neighbor(
                current, candidate, parameters, random_engine
            );
            const double candidate_score =
                evaluate_cycle_score(candidate, parameters, workspace);
            const double delta = candidate_score - current_score;

            bool accept_candidate = delta >= 0.0;
            if (!accept_candidate) {
                std::uniform_real_distribution<double> probability_distribution(
                    0.0, 1.0
                );
                const double probability =
                    probability_distribution(random_engine);
                const double acceptance_probability = std::exp(
                    std::max(-700.0, delta / std::max(temperature, 1e-12))
                );
                accept_candidate = probability < acceptance_probability;
            }

            if (accept_candidate) {
                current.swap(candidate);
                current_score = candidate_score;
            }
            if (current_score > best_local_score) {
                best_local = current;
                best_local_score = current_score;
            }
            temperature *= parameters.cooling_rate;
        }

        (*context.results)[restart].dice = std::move(best_local);
        (*context.results)[restart].score = best_local_score;
    } catch (...) {
        (*context.errors)[restart] = std::current_exception();
    }
}

}

// Explora vecinos y conserva la mejor solución de todos los reinicios.
DicePairs find_best_cycle(
    const Parameters& parameters
) {
    std::vector<RestartResult> results(parameters.restarts);
    std::vector<std::exception_ptr> errors(parameters.restarts);
    SearchContext context = {&parameters, &results, &errors};
    parallel_for(parameters.restarts, run_restart, &context);

    DicePairs best_global;
    double best_global_score = -std::numeric_limits<double>::infinity();
    for (std::size_t restart = 0; restart < parameters.restarts; ++restart) {
        if (errors[restart]) {
            std::rethrow_exception(errors[restart]);
        }
        //std::cout << "Reinicio " << restart + 1 << "/"
        //          << parameters.restarts
        //          << ": mejor puntuacion = "
        //          << std::fixed << std::setprecision(6)
        //         << results[restart].score << "\n";
        if (results[restart].score > best_global_score) {
            best_global = std::move(results[restart].dice);
            best_global_score = results[restart].score;
        }
    }
    return best_global;
}

}
