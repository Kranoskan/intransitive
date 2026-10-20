#ifndef INTRANSITIVE_MODEL_DICE_H
#define INTRANSITIVE_MODEL_DICE_H

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "../config/parameters.h"

namespace intransitive {

using Die = std::vector<std::size_t>;

struct DicePair {
    Die first;
    Die second;
};

using DicePairs = std::vector<DicePair>;
using SumHistogram = std::vector<std::uint64_t>;

// Crea todas las parejas iniciales conforme al rango y las restricciones configuradas.
DicePairs create_dice(
    const Parameters& parameters,
    std::mt19937& random_engine
);

// Cuenta cuántas combinaciones de caras producen cada suma para una pareja.
SumHistogram create_sum_histogram(
    const DicePair& pair,
    const Parameters& parameters
);

// Copia la solución en neighbor y modifica una cara válida, reutilizando capacidad.
void create_neighbor(
    const DicePairs& current,
    DicePairs& neighbor,
    const Parameters& parameters,
    std::mt19937& random_engine
);

}

#endif
