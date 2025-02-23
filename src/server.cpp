#include <iostream>
#include <string>
#include <cstdlib>  // For std::stoi
#include "pub_sub_server.hpp"


bool isValidPort(const std::string& portStr) {
    if (portStr.empty() || portStr.find_first_not_of("0123456789") != std::string::npos) {
        return false;  // Must be numeric
    }

    int port = std::stoi(portStr);
    return port > 0 && port <= 65535;  // Valid port range
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    std::string portStr = argv[1];

    if (!isValidPort(portStr)) {
        std::cerr << "Error: Invalid port number. Must be between 1 and 65535.\n";
        return 1;
    }

    int port = std::stoi(portStr);
    PubSubServer server(port);

    return 0;
}