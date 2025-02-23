#include <mutex>
#include <unordered_set>
#include <unordered_map>

#define SUBSCRIBE_COMMAND       "SUBSCRIBE"
#define UNSUBSCRIBE_COMMAND     "UNSUBSCRIBE"
#define PUBLISH_COMMAND         "PUBLISH"
#define NAME_COMMAND            "NAME"

class PubSubServer {
public:
    PubSubServer(int port);
    ~PubSubServer();
private:
    int server_fd;
    std::unordered_map<std::string, std::unordered_set<int>> topic_subscribers;
    std::unordered_map<int, std::string> client_names;
    std::mutex mutex_;

    void accept_clients();
    void handle_client(int client_socket);
    void handle_message(int client_socket, const std::string& message);
    void send_message(int client_socket, const std::string& message);
};