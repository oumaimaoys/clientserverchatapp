#include <iostream>
#include "client.h"
#include "server.h"
#include <cstring>

int main(){
    Server server;
    Client client;
    if (server.create_socket()){
        std::cout << "Server Socket created successfully\n";
    }
    else{
        return 1;
    }
    if (client.create_socket()){
        std::cout << "Client Socket created successfully\n";
    }
    else{
        return 1;
    }
    if (server.bind_address()) {
        std::cout << "address binded successfuly\n";
    }
    else{
        return 1;
    }

    if (server.start_listening()) {
        std::cout << "started listening\n";
    }
    else{
        return 1;
    }

    if(client.connect_to_server("127.0.0.1", 5000)){
        std::cout << "client connect to server\n";
    }
    else{
        return 1;
    }
    if(server.accept_client()){
        std::cout << "server accept client\n";
    }
    else{
        return 1;
    }

    const char* message = "Hello from client!";
    send(client.get_socket(), message, std::strlen(message), 0);

    char buffer[1024]{};
    ssize_t received = recv(
        server.get_client_socket(),
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received > 0) {
        std::cout << "\nServer received: " << buffer << '\n';
    }

    client.close_connection();
    server.close_socket();
    return 0;
}