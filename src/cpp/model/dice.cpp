#include "dice.h"

#include <algorithm>
#include <unordered_set>

namespace intransitive {
namespace {

// Genera un dado aleatorio con valores repetidos o distintos según la configuración.
Die create_die(
    const Parameters& parameters,
    std::mt19937& random_engine
) {
    Die die;
    die.reserve(parameters.faces_per_die);

    if (parameters.distinct_faces) {
        std::unordered_set<std::size_t> selected;
        std::uniform_int_distribution<std::size_t> candidate_distribution;
        const std::size_t first_candidate =
            parameters.max_face_value - parameters.faces_per_die;
        for (std::size_t candidate_range = first_candidate;
             candidate_range < parameters.max_face_value;
             ++candidate_range) {
            candidate_distribution =
                std::uniform_int_distribution<std::size_t>(
                    0, candidate_range
                );
            const std::size_t candidate =
                candidate_distribution(random_engine);
            if (!selected.insert(candidate).second) {
                selected.insert(candidate_range);
            }
        }
        for (const std::size_t value : selected) {
            die.push_back(value + 1);
        }
        std::shuffle(die.begin(), die.end(), random_engine);
    } else {
        std::uniform_int_distribution<std::size_t> value_distribution(
            1, parameters.max_face_value
        );
        for (std::size_t face = 0;
             face < parameters.faces_per_die;
             ++face) {
            die.push_back(value_distribution(random_engine));
        }
    }
    return die;
}

}

// Construye las parejas iniciales usadas al comenzar cada reinicio.
DicePairs create_dice(
    const Parameters& parameters,
    std::mt19937& random_engine
) {
    DicePairs dice;
    dice.reserve(parameters.pair_count);
    for (std::size_t index = 0; index < parameters.pair_count; ++index) {
        dice.push_back({
            create_die(parameters, random_engine),
            create_die(parameters, random_engine)
        });
    }
    return dice;
}

// Cuenta en un histograma las sumas de todas las combinaciones de dos caras.
SumHistogram create_sum_histogram(
    const DicePair& pair,
    const Parameters& parameters
) {
    SumHistogram histogram(2 * parameters.max_face_value + 1, 0);
    for (const std::size_t first_value : pair.first) {
        for (const std::size_t second_value : pair.second) {
            ++histogram[first_value + second_value];
        }
    }
    return histogram;
}

// Cambia una sola cara para proponer una solución vecina al recocido.
void create_neighbor(
    const DicePairs& current,
    DicePairs& neighbor,
    const Parameters& parameters,
    std::mt19937& random_engine
) {
    neighbor = current;
    std::uniform_int_distribution<std::size_t> pair_distribution(
        0, parameters.pair_count - 1
    );
    std::uniform_int_distribution<std::size_t> die_distribution(0, 1);
    std::uniform_int_distribution<std::size_t> face_distribution(
        0, parameters.faces_per_die - 1
    );
    std::uniform_int_distribution<std::size_t> value_distribution(
        1, parameters.max_face_value
    );

    for (int attempt = 0; attempt < 100; ++attempt) {
        const std::size_t pair_index = pair_distribution(random_engine);
        const std::size_t die_index = die_distribution(random_engine);
        const std::size_t face_index = face_distribution(random_engine);
        const std::size_t new_value = value_distribution(random_engine);
        Die& die = die_index == 0
            ? neighbor[pair_index].first
            : neighbor[pair_index].second;

        if (parameters.distinct_faces &&
            std::find(die.begin(), die.end(), new_value) != die.end() &&
            die[face_index] != new_value) {
            continue;
        }
        die[face_index] = new_value;
        return;
    }
}

}
