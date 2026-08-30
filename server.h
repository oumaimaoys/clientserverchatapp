#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

class Server {
private:
    sockaddr_in socket_address{};
    int l_socket; // listening socket
    std::vector<int> connected_clients;

public:
    Server();

    bool create_socket();
    bool bind_address();
    bool start_listening();
    int accept_client();
    void close_socket();
    sockaddr_in get_socket_address();
    std::vector<int> get_clients_sockets() const;
    int get_server_socket();


};


#endif