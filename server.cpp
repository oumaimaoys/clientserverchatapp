#include "server.h"
#include <sys/socket.h>
#include <unistd.h>

Server::Server(){
    socket_address.sin_family = AF_INET; // sets family of ip to ipv4
    socket_address.sin_port = htons(5000); // sets port to 5000 and turn port number to network byte order
    inet_pton(AF_INET, "127.0.0.1", &socket_address.sin_addr); // turn ip address to binary form
}

sockaddr_in Server::get_socket_address(){
    return this->socket_address;
}

bool Server::create_socket(){
    this->l_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (l_socket < 0) {
        return false;
    }
    return true;
}

bool Server::bind_address(){
    int b = bind(l_socket, reinterpret_cast<sockaddr*>(&socket_address), sizeof(socket_address));
    if (b < 0){
        return false;
    }
    return true;
}

bool Server::start_listening(){
    int l = listen(l_socket, 5); // 5 is the max len of the queue of pending connections, if a conection request arrrive after 5 the client will recieve error
    if (l < 0) {
        return false;
    }
    return true;
}

int Server::accept_client(){
    return accept(l_socket, nullptr, nullptr);
    
}

void Server::close_socket(){
    close(l_socket);
}

std::vector<int> Server::get_clients_sockets() const {
    return connected_clients; 
}

int Server::get_server_socket(){
    return l_socket;
}