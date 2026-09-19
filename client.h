#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client {
private:
    int c_socket;
    int client_id;
    std::string username;

public:
    Client();

    bool create_socket();
    bool connect_to_server(const std::string& server_ip, int port);
    bool receive_assigned_id();
    void close_connection();
    int get_socket();
    int get_id() const;
    bool update_username(std::string username);



};

#endif
