#include "ConfigParser.hpp"

bool ConfigParser::containsOnlyValidChars(const std::string& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        // Solo letras, números, guiones y puntos
        if (!std::isalnum(c) && c != '-' && c != '.') {
            return false;
        }
    }
    return true;
}

bool ConfigParser::isValidHostnameLabel(const std::string& label) {
    // Label no puede estar vacío
    if (label.empty()) {
        return false;
    }

    // Label no puede ser más largo que 63 caracteres (RFC)
    if (label.length() > 63) {
        return false;
    }

    // No puede empezar o terminar con guión
    if (label[0] == '-' || label[label.length() - 1] == '-') {
        return false;
    }

    // Solo caracteres alfanuméricos y guiones
    for (size_t i = 0; i < label.length(); ++i) {
        char c = label[i];
        if (!std::isalnum(c) && c != '-') {
            return false;
        }
    }

    return true;
}

bool ConfigParser::isValidHostname(const std::string& hostname) {
    // Hostname vacío es inválido
    if (hostname.empty()) {
        std::cerr << "Error: Empty hostname" << std::endl;
        return false;
    }

    // Casos especiales válidos
    if (hostname == "localhost") {
        return true;
    }

    // Verificar si es una IP (validación básica)
    bool is_ip = true;
    size_t dot_count = 0;
    for (size_t i = 0; i < hostname.length(); ++i) {
        char c = hostname[i];
        if (c == '.') {
            dot_count++;
        } else if (!std::isdigit(c)) {
            is_ip = false;
            break;
        }
    }

    if (is_ip && dot_count == 3) {
        return true; // Asumimos IP válida (validación básica)
    }

    // Longitud total máxima (RFC 1123)
    if (hostname.length() > 253) {
        std::cerr << "Error: Hostname too long: " << hostname.length()
                  << " characters (max: 253)" << std::endl;
        return false;
    }

    // No puede empezar o terminar con punto
    if (hostname[0] == '.' || hostname[hostname.length() - 1] == '.') {
        std::cerr << "Error: Hostname cannot start or end with dot: "
                  << hostname << std::endl;
        return false;
    }

    // Verificar caracteres válidos (sin wildcards)
    if (!containsOnlyValidChars(hostname)) {
        std::cerr << "Error: Invalid characters in hostname: "
                  << hostname << std::endl;
        return false;
    }

    // No permitir wildcards en webserv
    if (hostname.find('*') != std::string::npos) {
        std::cerr << "Error: Wildcards not supported in hostname: "
                  << hostname << std::endl;
        return false;
    }

    // Dividir en labels y validar cada uno
    std::vector<std::string> labels;
    std::string current_label;

    for (size_t i = 0; i < hostname.length(); ++i) {
        if (hostname[i] == '.') {
            if (!current_label.empty()) {
                labels.push_back(current_label);
                current_label.clear();
            }
        } else {
            current_label += hostname[i];
        }
    }

    // Añadir último label si existe
    if (!current_label.empty()) {
        labels.push_back(current_label);
    }

    // Validar cada label
    for (size_t i = 0; i < labels.size(); ++i) {
        if (!isValidHostnameLabel(labels[i])) {
            std::cerr << "Error: Invalid hostname label: '"
                      << labels[i] << "' in " << hostname << std::endl;
            return false;
        }
    }

    return true;
}
