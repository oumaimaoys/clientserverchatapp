#include <iostream>
#include "client.h"
#include <cstring>
#include <atomic>
#include <thread>
#include <sys/socket.h>

std::atomic<bool> running{true};

void handle_input(int client_socket){
    while(running){

        std::cout << "Enter a message:\n";

        std::string msg;
        if (!std::getline(std::cin, msg)) {
            running = false;
            shutdown(client_socket, SHUT_RDWR);
            break;
        }

        if (msg == "/quit") {
            running = false;
            shutdown(client_socket, SHUT_RDWR);
            break;
        }
        std::string wire_msg = msg + "\n";        
        ssize_t sent = send(
            client_socket,
            wire_msg.c_str(),
            wire_msg.size(),
            0
        );

        if (sent < 0) {
            perror("send");
            running = false;
            break;
        }
    }
}

void handle_incoming(int client_socket){
    while(running){
        char buffer[1024];

        ssize_t received = recv(
            client_socket,
            buffer,
            sizeof(buffer),
            0
        );
        if (received == 0) {
            std::cout << "Server disconnected\n";
            running = false;
            break;
        }

        if (received < 0) {
            if (running) {
                perror("recv");
            }
            running = false;
            break;
        }

        std::cout.write(buffer, received);
        std::cout << '\n';
    }
}

int main(){
    // create client and establish connection
    Client client;
    if(client.create_socket()){
        std::cout << "Client socket created succesfully!\n";
    }
    else{
        return 1;
    }

    bool connection = client.connect_to_server("127.0.0.1", 5000);

    

    if(connection){
        std::cout << "Established connection!!\n";
    }
    else{
        return 1;
    }

    int client_socket = client.get_socket();
    std::thread input_thread(handle_input, client_socket);
    std::thread incoming_thread(handle_incoming, client_socket);

    input_thread.join();
    running = false;
    shutdown(client_socket, SHUT_RDWR);
    incoming_thread.join();

    std::cout << "connection closed\n";
    client.close_connection();


    return 0;
}
