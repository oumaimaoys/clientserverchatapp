#include "server.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>

Server::Server(){
    next_client_id = 1;
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

client_data Server::accept_client(){
    client_data data{};
    data.client_socket = -1;
    data.client_id = 0;

    sockaddr_in peer{};
    socklen_t peer_len = sizeof(peer);
    int fd = accept(l_socket, reinterpret_cast<sockaddr*>(&peer), &peer_len);
    if (fd < 0) {
        return data;
    }

    char ip[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));

    data.client_socket = fd;
    data.client_id = next_client_id++;
    data.address = std::string(ip) + ":" + std::to_string(ntohs(peer.sin_port));

    connected_clients.push_back(data);

    uint32_t net_id = htonl(static_cast<uint32_t>(data.client_id));
    send(fd, &net_id, sizeof(net_id), 0);

    return data;
}

void Server::close_socket(){
    close(l_socket);
}

std::vector<client_data>& Server::get_clients_sockets() {
    return connected_clients; 
}

int Server::get_server_socket(){
    return l_socket;
}

bool Server::send_all(std::vector<client_data> clients_list, std::string msg){
    for (client_data client : clients_list){
        if (send(client.client_socket, msg.c_str(), msg.size(), 0) < 0 ){
            return false;
        }
    }
    return true;
}

bool Server::send_to(int client_id, const std::string& msg){
    for (const client_data& client : connected_clients){
        if (client.client_id == client_id){
            return send(client.client_socket, msg.c_str(), msg.size(), 0) >= 0;
        }
    }
    return false;
}
