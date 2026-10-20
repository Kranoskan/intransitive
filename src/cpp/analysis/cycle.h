#ifndef INTRANSITIVE_ANALYSIS_CYCLE_H
#define INTRANSITIVE_ANALYSIS_CYCLE_H

#include <cstdint>
#include <vector>

#include "../config/parameters.h"
#include "../model/dice.h"

namespace intransitive {

struct Comparison {
    std::uint64_t wins = 0;
    std::uint64_t ties = 0;
    std::uint64_t losses = 0;
    std::uint64_t probability_numerator = 0;
    std::uint64_t probability_denominator = 1;

    // Devuelve la probabilidad con empates contados como media victoria.
    double probability() const;
};

struct Evaluation {
    double score = 0.0;
    std::vector<Comparison> comparisons;
};

struct EvaluationWorkspace {
    // Reserva una sola vez los histogramas que se reutilizan durante la búsqueda.
    explicit EvaluationWorkspace(const Parameters& parameters);

    std::vector<SumHistogram> histograms;
};

// Compara dos histogramas y cuenta victorias, empates y derrotas.
Comparison compare_histograms(
    const SumHistogram& first,
    const SumHistogram& second,
    const Parameters& parameters
);

// Analiza cada enfrentamiento y devuelve los conteos completos y la puntuación.
Evaluation evaluate_cycle(
    const DicePairs& dice,
    const Parameters& parameters
);

// Reutiliza histogramas reservados y calcula la puntuación sin conteos detallados.
double evaluate_cycle_score(
    const DicePairs& dice,
    const Parameters& parameters,
    EvaluationWorkspace& workspace
);

}

#endif
