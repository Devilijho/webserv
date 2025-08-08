/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pde-vara <pde-vara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 12:57:19 by pde-vara          #+#    #+#             */
/*   Updated: 2025/08/08 12:57:27 by pde-vara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <sstream>
#include <iostream>

Request::Request(const std::string& raw_request) {
	parse(raw_request);
}

void Request::parse(const std::string& raw_request) {
	std::istringstream stream(raw_request);
	std::string line;

	// Parse request line: METHOD PATH HTTP_VERSION
	if (std::getline(stream, line)) {
		std::istringstream line_stream(line);
		line_stream >> method >> path >> http_version;
	}

    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);

            // Trim whitespace
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) value.erase(0, 1);
            while (!value.empty() && (value.back() == '\r' || value.back() == '\n')) value.pop_back();

            headers[key] = value;
        }
    }

    // Read body if Content-Length header is present
    auto it = headers.find("Content-Length");
    if (it != headers.end()) {
        int length = std::stoi(it->second);
        body.resize(length);
        stream.read(&body[0], length);
    }
}

std::string Request::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}
