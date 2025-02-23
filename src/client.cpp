#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <condition_variable>
#include <arpa/inet.h>
#include <unistd.h>

bool running = true;
int sockfd = -1;

void receive_messages() {
    char buffer[1024];
    while (running) {
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "[Message] "<<  buffer << std::endl;
        } else if (bytes_received == 0) {
            std::cout << "Server disconnected." << std::endl;
            running = false;
        } else {
            perror("recv");
        }
    }
}

int main() {
    std::string input;
    std::thread receiver_thread;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.empty()) continue;
        
        if (tokens[0] == "CONNECT" && tokens.size() == 3) {
            int port = std::stoi(tokens[1]);
            std::string client_name = tokens[2];

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                perror("socket");
                continue;
            }

            sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = INADDR_ANY;

            if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
                perror("connect");
                close(sockfd);
                sockfd = -1;
                continue;
            }
            std::string Name_command = "NAME " + client_name;
            send(sockfd, Name_command.c_str(), Name_command.length(), 0);
            std::cout << "Connected as " << client_name << std::endl;
            
            running = true;
            receiver_thread = std::thread(receive_messages);

        } else if (tokens[0] == "DISCONNECT") {
            if (sockfd != -1) {
                close(sockfd);
                sockfd = -1;
                running = false;
                std::cout << "Disconnected from server." << std::endl;
            }
        } else if ((tokens[0] == "PUBLISH" || tokens[0] == "SUBSCRIBE" || tokens[0] == "UNSUBSCRIBE") && tokens.size() >= 2) {
            if (sockfd == -1) {
                std::cout << "Not connected to server." << std::endl;
                continue;
            }
            std::string message = input + "\n";
            send(sockfd, message.c_str(), message.length(), 0);
        } else {
            std::cout << "Invalid command." << std::endl;
        }
    }
    return 0;
}
