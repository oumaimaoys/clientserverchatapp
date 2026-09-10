#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

struct client_data
{
    int client_socket;
    int client_id;
    std::string username;
    std::string address;
};


class Server {
private:
    sockaddr_in socket_address{};
    int l_socket; // listening socket
    int next_client_id;
    std::vector<client_data> connected_clients;

public:
    Server();

    bool create_socket();
    bool bind_address();
    bool start_listening();
    client_data accept_client();
    void close_socket();
    sockaddr_in get_socket_address();
    std::vector<client_data>& get_clients_sockets();
    
    int get_server_socket();
    bool send_all(std::vector<client_data> clients_list, std::string msg);
    bool send_to(int client_id, const std::string& msg);


};


#endif
