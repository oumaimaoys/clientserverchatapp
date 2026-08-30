#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

class Server {
private:
    sockaddr_in socket_address{};
    int l_socket;
    int client_socket; // make list later

public:
    Server();

    bool create_socket();
    bool bind_address();
    bool start_listening();
    bool accept_client();
    void close_socket();
    sockaddr_in get_socket_address();
    int get_client_socket() const;
    int get_server_socket();


};


#endif