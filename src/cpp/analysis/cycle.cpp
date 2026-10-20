#include "cycle.h"

#include <algorithm>
#include <stdexcept>

namespace intransitive {
namespace {

// Calcula el máximo común divisor para reducir la fracción de probabilidad.
std::uint64_t greatest_common_divisor(
    std::uint64_t first,
    std::uint64_t second
) {
    while (second != 0) {
        const std::uint64_t remainder = first % second;
        first = second;
        second = remainder;
    }
    return first;
}

}

EvaluationWorkspace::EvaluationWorkspace(const Parameters& parameters) {
    histograms.reserve(parameters.pair_count);
    const std::size_t histogram_size =
        2 * parameters.max_face_value + 1;
    for (std::size_t index = 0; index < parameters.pair_count; ++index) {
        histograms.push_back(SumHistogram(histogram_size, 0));
    }
}

// Convierte la fracción exacta almacenada a un valor útil para puntuar.
double Comparison::probability() const {
    return static_cast<double>(probability_numerator) /
           static_cast<double>(probability_denominator);
}

// Cuenta los resultados de comparar todas las sumas de ambas parejas.
Comparison compare_histograms(
    const SumHistogram& first,
    const SumHistogram& second,
    const Parameters& parameters
) {
    Comparison comparison;
    std::uint64_t cumulative_second = 0;

    for (std::size_t sum = 0; sum < first.size(); ++sum) {
        comparison.wins += first[sum] * cumulative_second;
        comparison.ties += first[sum] * second[sum];
        cumulative_second += second[sum];
    }

    std::uint64_t total = 1;
    for (int i = 0; i < 4; ++i) {
        total *= parameters.faces_per_die;
    }
    comparison.losses = total - comparison.wins - comparison.ties;
    comparison.probability_numerator =
        2 * comparison.wins + comparison.ties;
    comparison.probability_denominator = 2 * total;

    const std::uint64_t divisor = greatest_common_divisor(
        comparison.probability_numerator,
        comparison.probability_denominator
    );
    comparison.probability_numerator /= divisor;
    comparison.probability_denominator /= divisor;
    return comparison;
}

// Compara cada pareja con la siguiente y calcula la puntuación del ciclo.
Evaluation evaluate_cycle(
    const DicePairs& dice,
    const Parameters& parameters
) {
    EvaluationWorkspace workspace(parameters);
    if (dice.size() != parameters.pair_count) {
        throw std::invalid_argument(
            "La cantidad de parejas no coincide con el parámetro n."
        );
    }

    for (std::size_t index = 0; index < dice.size(); ++index) {
        SumHistogram& histogram = workspace.histograms[index];
        std::fill(histogram.begin(), histogram.end(), 0);
        for (const std::size_t first_value : dice[index].first) {
            for (const std::size_t second_value : dice[index].second) {
                ++histogram[first_value + second_value];
            }
        }
    }

    Evaluation evaluation;
    evaluation.comparisons.reserve(parameters.pair_count);
    double minimum_advantage = 1.0;
    double total_advantage = 0.0;
    for (std::size_t index = 0; index < parameters.pair_count; ++index) {
        const std::size_t next = (index + 1) % parameters.pair_count;
        Comparison comparison = compare_histograms(
            workspace.histograms[index],
            workspace.histograms[next],
            parameters
        );
        const double advantage = comparison.probability() - 0.5;
        minimum_advantage = std::min(minimum_advantage, advantage);
        total_advantage += advantage;
        evaluation.comparisons.push_back(comparison);
    }

    evaluation.score =
        minimum_advantage +
        0.05 * total_advantage / parameters.pair_count;
    return evaluation;
}

double evaluate_cycle_score(
    const DicePairs& dice,
    const Parameters& parameters,
    EvaluationWorkspace& workspace
) {
    if (dice.size() != parameters.pair_count) {
        throw std::invalid_argument(
            "La cantidad de parejas no coincide con el parámetro n."
        );
    }
    if (workspace.histograms.size() != parameters.pair_count) {
        throw std::invalid_argument(
            "El espacio de evaluación no coincide con el parámetro n."
        );
    }

    for (std::size_t index = 0; index < dice.size(); ++index) {
        SumHistogram& histogram = workspace.histograms[index];
        std::fill(histogram.begin(), histogram.end(), 0);
        for (const std::size_t first_value : dice[index].first) {
            for (const std::size_t second_value : dice[index].second) {
                ++histogram[first_value + second_value];
            }
        }
    }

    double minimum_advantage = 1.0;
    double total_advantage = 0.0;
    std::uint64_t total_outcomes = 1;
    for (int i = 0; i < 4; ++i) {
        total_outcomes *= parameters.faces_per_die;
    }
    const double probability_denominator =
        2.0 * static_cast<double>(total_outcomes);

    for (std::size_t index = 0; index < parameters.pair_count; ++index) {
        const std::size_t next = (index + 1) % parameters.pair_count;
        const SumHistogram& first = workspace.histograms[index];
        const SumHistogram& second = workspace.histograms[next];
        std::uint64_t wins = 0;
        std::uint64_t ties = 0;
        std::uint64_t cumulative_second = 0;
        for (std::size_t sum = 0; sum < first.size(); ++sum) {
            wins += first[sum] * cumulative_second;
            ties += first[sum] * second[sum];
            cumulative_second += second[sum];
        }

        const double advantage =
            static_cast<double>(2 * wins + ties) /
                probability_denominator -
            0.5;
        minimum_advantage = std::min(minimum_advantage, advantage);
        total_advantage += advantage;
    }

    return minimum_advantage +
           0.05 * total_advantage / parameters.pair_count;
}

}
