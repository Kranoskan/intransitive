#include "parameters.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../platform/paths.h"

namespace intransitive {
namespace {

// Elimina espacios y tabulaciones al principio y al final de una cadena.
std::string trim(const std::string& text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

// Interpreta una línea de configuración y detecta errores con su número de línea.
void parse_config_line(
    std::string line,
    std::size_t line_number,
    const std::string& config_path,
    std::map<std::string, std::string>& values
);

// Lee todas las opciones sin aceptar claves duplicadas ni líneas mal formadas.
std::map<std::string, std::string> parse_config(
    const std::string& config_path
) {
    std::unique_ptr<FILE, int (*)(FILE*)> file(
        open_path(config_path, "rb"), &std::fclose
    );
    if (!file) {
        throw std::runtime_error(
            "No se pudo abrir el archivo de parámetros: " + config_path
        );
    }

    std::map<std::string, std::string> values;
    std::string line;
    std::size_t line_number = 0;
    int character = 0;
    while ((character = std::fgetc(file.get())) != EOF) {
        if (character != '\n') {
            line.push_back(static_cast<char>(character));
            continue;
        }

        ++line_number;
        parse_config_line(line, line_number, config_path, values);
        line.clear();
    }
    if (!line.empty()) {
        ++line_number;
        parse_config_line(line, line_number, config_path, values);
    }
    if (std::ferror(file.get())) {
        throw std::runtime_error(
            "Error al leer el archivo de parámetros: " + config_path
        );
    }
    return values;
}

void parse_config_line(
    std::string line,
    std::size_t line_number,
    const std::string& config_path,
    std::map<std::string, std::string>& values
) {
    if (line_number == 1 &&
        line.size() >= 3 &&
        static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }

    line = trim(line.substr(0, line.find('#')));
    if (line.empty()) {
        return;
    }

    const std::size_t separator = line.find(':');
    if (separator == std::string::npos) {
        throw std::runtime_error(
            "Formato inválido en " + config_path +
            ", línea " + std::to_string(line_number) +
            ": se esperaba clave:valor."
        );
    }

    const std::string key = trim(line.substr(0, separator));
    const std::string value = trim(line.substr(separator + 1));
    if (key.empty() || value.empty()) {
        throw std::runtime_error(
            "Parámetro vacío en " + config_path +
            ", línea " + std::to_string(line_number) + "."
        );
    }
    if (!values.emplace(key, value).second) {
        throw std::runtime_error(
            "Parámetro duplicado '" + key + "' en " + config_path +
            ", línea " + std::to_string(line_number) + "."
        );
    }
}

// Devuelve la lista de claves que toda configuración debe proporcionar.
const std::vector<std::string>& required_keys() {
    static const std::vector<std::string> keys = {
        "n", "c", "max_valor", "reinicios", "iteraciones",
        "temperatura_inicial", "enfriamiento", "semilla",
        "caras_distintas", "path_results"
    };
    return keys;
}

// Obtiene una opción obligatoria o informa del archivo que la omite.
const std::string& get_value(
    const std::map<std::string, std::string>& values,
    const std::string& key,
    const std::string& config_path
) {
    const auto found = values.find(key);
    if (found == values.end()) {
        throw std::runtime_error(
            "Falta el parámetro '" + key + "' en " + config_path + "."
        );
    }
    return found->second;
}

// Convierte una opción en entero no negativo y comprueba su rango.
std::size_t parse_size(
    const std::string& value,
    const std::string& key,
    const std::string& config_path
) {
    try {
        if (value.empty() || value.front() == '-') {
            throw std::invalid_argument("negative integer");
        }
        std::size_t parsed_length = 0;
        const unsigned long long parsed = std::stoull(value, &parsed_length);
        if (parsed_length != value.size() ||
            parsed > std::numeric_limits<std::size_t>::max()) {
            throw std::invalid_argument("out of range");
        }
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception&) {
        throw std::runtime_error(
            "El parámetro '" + key +
            "' debe ser un entero no negativo en " + config_path + "."
        );
    }
}

// Convierte una opción en número finito y rechaza texto sobrante.
double parse_double(
    const std::string& value,
    const std::string& key,
    const std::string& config_path
) {
    try {
        std::size_t parsed_length = 0;
        const double parsed = std::stod(value, &parsed_length);
        if (parsed_length != value.size() || !std::isfinite(parsed)) {
            throw std::invalid_argument("invalid number");
        }
        return parsed;
    } catch (const std::exception&) {
        throw std::runtime_error(
            "El parámetro '" + key +
            "' debe ser un número válido en " + config_path + "."
        );
    }
}

}

// Carga las opciones, aplica sus tipos y resuelve la ruta de salida.
Parameters load_parameters(const std::string& config_path) {
    const std::string resolved_config =
        absolute_path(config_path, current_directory());
    const std::map<std::string, std::string> values =
        parse_config(resolved_config);

    for (const auto& entry : values) {
        bool known = false;
        for (const std::string& key : required_keys()) {
            if (entry.first == key) {
                known = true;
                break;
            }
        }
        if (!known) {
            throw std::runtime_error(
                "Parámetro desconocido '" + entry.first + "' en " +
                resolved_config + "."
            );
        }
    }

    Parameters parameters;
    parameters.pair_count =
        parse_size(get_value(values, "n", resolved_config), "n", resolved_config);
    parameters.faces_per_die =
        parse_size(get_value(values, "c", resolved_config), "c", resolved_config);
    parameters.max_face_value = parse_size(
        get_value(values, "max_valor", resolved_config),
        "max_valor",
        resolved_config
    );
    parameters.restarts = parse_size(
        get_value(values, "reinicios", resolved_config),
        "reinicios",
        resolved_config
    );
    parameters.iterations = parse_size(
        get_value(values, "iteraciones", resolved_config),
        "iteraciones",
        resolved_config
    );
    parameters.initial_temperature = parse_double(
        get_value(values, "temperatura_inicial", resolved_config),
        "temperatura_inicial",
        resolved_config
    );
    parameters.cooling_rate = parse_double(
        get_value(values, "enfriamiento", resolved_config),
        "enfriamiento",
        resolved_config
    );

    const std::size_t seed = parse_size(
        get_value(values, "semilla", resolved_config),
        "semilla",
        resolved_config
    );
    if (seed > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error(
            "El parámetro 'semilla' excede el rango permitido en " +
            resolved_config + "."
        );
    }
    parameters.seed = static_cast<std::uint32_t>(seed);

    std::string distinct_faces =
        get_value(values, "caras_distintas", resolved_config);
    std::transform(
        distinct_faces.begin(),
        distinct_faces.end(),
        distinct_faces.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        }
    );
    if (distinct_faces == "true") {
        parameters.distinct_faces = true;
    } else if (distinct_faces == "false") {
        parameters.distinct_faces = false;
    } else {
        throw std::runtime_error(
            "El parámetro 'caras_distintas' debe ser true o false en " +
            resolved_config + "."
        );
    }

    const std::string result_path =
        get_value(values, "path_results", resolved_config);
    parameters.results_path = absolute_path(
        result_path, parent_path(resolved_config)
    );
    return parameters;
}

// Comprueba las restricciones del algoritmo y los límites de los contadores.
void validate_parameters(const Parameters& parameters) {
    if (parameters.pair_count < 3) {
        throw std::invalid_argument("Un ciclo estricto requiere n >= 3.");
    }
    if (parameters.faces_per_die < 1 ||
        parameters.max_face_value < 1) {
        throw std::invalid_argument("c y max_valor deben ser positivos.");
    }
    if (parameters.restarts < 1) {
        throw std::invalid_argument("reinicios debe ser mayor que cero.");
    }
    if (parameters.distinct_faces &&
        parameters.faces_per_die > parameters.max_face_value) {
        throw std::invalid_argument(
            "No caben c caras distintas entre 1 y max_valor."
        );
    }
    if (parameters.max_face_value >
        (std::numeric_limits<std::size_t>::max() - 1) / 2) {
        throw std::invalid_argument(
            "max_valor es demasiado grande para construir los histogramas."
        );
    }
    if (parameters.initial_temperature < 0.0) {
        throw std::invalid_argument(
            "temperatura_inicial no puede ser negativa."
        );
    }
    if (parameters.cooling_rate < 0.0) {
        throw std::invalid_argument("enfriamiento no puede ser negativo.");
    }

    const std::uint64_t max_count =
        std::numeric_limits<std::uint64_t>::max();
    std::uint64_t total = 1;
    for (int i = 0; i < 4; ++i) {
        if (parameters.faces_per_die > max_count / total) {
            throw std::invalid_argument(
                "c es demasiado grande para contar las combinaciones."
            );
        }
        total *= parameters.faces_per_die;
    }
    if (total > max_count / 2) {
        throw std::invalid_argument(
            "c es demasiado grande para representar las probabilidades."
        );
    }
}

}
