#ifndef INTRANSITIVE_CONFIG_PARAMETERS_H
#define INTRANSITIVE_CONFIG_PARAMETERS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace intransitive {

struct Parameters {
    std::size_t pair_count = 0;
    std::size_t faces_per_die = 0;
    std::size_t max_face_value = 0;
    std::size_t restarts = 0;
    std::size_t iterations = 0;
    double initial_temperature = 0.0;
    double cooling_rate = 0.0;
    std::uint32_t seed = 0;
    bool distinct_faces = false;
    std::string results_path;
};

// Lee y convierte las opciones del archivo; las rutas relativas parten de su carpeta.
Parameters load_parameters(const std::string& config_path);

// Comprueba que los valores permiten ejecutar el análisis sin desbordamientos.
void validate_parameters(const Parameters& parameters);

}

#endif
