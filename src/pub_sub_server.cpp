#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include "pub_sub_server.hpp"


PubSubServer::PubSubServer(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server listening on port " << port << std::endl;
    accept_clients();
}

PubSubServer::~PubSubServer() {
    close(server_fd);
}



void PubSubServer::accept_clients() {
    while (true) {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "Client connected." << std::endl;
        std::thread(&PubSubServer::handle_client, this, client_socket).detach();
    }
}

void PubSubServer::handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            std::cout << "Client disconnected." << std::endl;
            client_names.erase(client_socket);
            close(client_socket);
            return;
        }
        
        std::string message(buffer);
        handle_message(client_socket, message);
    }
    
}

void PubSubServer::handle_message(int client_socket, const std::string& message) {
    if (message.rfind(SUBSCRIBE_COMMAND, 0) == 0) {
        if(message.rfind("/", sizeof(SUBSCRIBE_COMMAND))!= std::string::npos) {
            std::string topic = message.substr(message.rfind("/", sizeof(SUBSCRIBE_COMMAND)));
            // Properly remove newline characters (\n, \r,) when used from therminal
            topic.erase(std::remove(topic.begin(), topic.end(), '\n'), topic.end());
            topic.erase(std::remove(topic.begin(), topic.end(), '\r'), topic.end());
            std::lock_guard<std::mutex> lock(mutex_);
            topic_subscribers[topic].insert(client_socket);
            std::cout << "Client " << client_names[client_socket] << " subscribed to: '" << topic <<"'" <<std::endl;
        }
    } else if (message.rfind(PUBLISH_COMMAND, 0) == 0) {
        auto pos = message.find(' ', sizeof(PUBLISH_COMMAND));
        std::string topic = message.substr(sizeof(PUBLISH_COMMAND), pos - sizeof(PUBLISH_COMMAND));
        std::string content = message.substr(pos + 1);
        std::lock_guard<std::mutex> lock(mutex_);
        if (topic_subscribers.count(topic)) {
            for (int subscriber : topic_subscribers[topic]) {
                send_message(subscriber, "Topic: " + topic + " Data: " + content + "\n");
            }
        }
    } else if (message.rfind(UNSUBSCRIBE_COMMAND, 0) == 0) {
        if(message.rfind("/", sizeof(UNSUBSCRIBE_COMMAND))!= std::string::npos) {
            std::string topic = message.substr(message.rfind("/", sizeof(UNSUBSCRIBE_COMMAND)));
            // Properly remove newline characters (\n, \r,) when used from therminal
            topic.erase(std::remove(topic.begin(), topic.end(), '\n'), topic.end());
            topic.erase(std::remove(topic.begin(), topic.end(), '\r'), topic.end());
            std::lock_guard<std::mutex> lock(mutex_);

            topic_subscribers[topic].erase(client_socket);
            std::cout << "Client " << client_names[client_socket] << " unsubscribed from: '" << topic <<"'" <<std::endl;
        }
    } else if (message.rfind(NAME_COMMAND, 0) == 0) {
        std::string client_name = message.substr(sizeof(NAME_COMMAND));
        // Properly remove newline characters (\n, \r,) when used from therminal
        client_name.erase(std::remove(client_name.begin(), client_name.end(), '\n'), client_name.end());
        client_name.erase(std::remove(client_name.begin(), client_name.end(), '\r'), client_name.end());

        client_names[client_socket] = client_name;
        std::cout << "With name: " << client_name <<std::endl;
    }
}

void PubSubServer::send_message(int client_socket, const std::string& message) {
    send(client_socket, message.c_str(), message.length(), 0);
}