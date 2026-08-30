#include "client.h"
#include <sys/socket.h>
#include <unistd.h>

Client::Client(){
}


bool Client::create_socket(){
    this->c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( c_socket < 0 ){
        return false;
    }
    return true;
}

bool Client::connect_to_server(std::string server_ip, int port){
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET; // sets family of ip to ipv4
    server_address.sin_port = htons(5000); // sets port to 5000 and turn port number to network byte order
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr); // turn ip address to binary form

    int connection = connect(c_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if ( connection < 0 ){
        return false;
    }
    return true;
}

void Client::close_connection(){
    close(c_socket);
}

int Client::get_socket(){ 
    return c_socket; 
}

