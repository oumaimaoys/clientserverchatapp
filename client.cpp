#include "client.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>

Client::Client(){
    c_socket = -1;
    client_id = 0;
}


bool Client::create_socket(){
    this->c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( c_socket < 0 ){
        return false;
    }
    return true;
}

bool Client::connect_to_server(const std::string& server_ip, int port){
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET; // sets family of ip to ipv4
    server_address.sin_port = htons(port); // sets port to 5000 and turn port number to network byte order
    inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr); // turn ip address to binary form

    int connection = connect(c_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if ( connection < 0 ){
        return false;
    }
    return true;
}

bool Client::receive_assigned_id(){
    uint32_t net_id = 0;
    ssize_t n = recv(c_socket, &net_id, sizeof(net_id), MSG_WAITALL);
    if (n != static_cast<ssize_t>(sizeof(net_id))) {
        return false;
    }
    client_id = static_cast<int>(ntohl(net_id));
    return true;
}

void Client::close_connection(){
    close(c_socket);
}

int Client::get_socket(){ 
    return c_socket; 
}

int Client::get_id() const{
    return client_id;
}

bool Client::update_username(std::string username){
    this->username = username;
    return true;
}