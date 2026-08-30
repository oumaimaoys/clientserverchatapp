#include <iostream>
#include "server.h"
#include <string>
#include <thread>
#include <unistd.h>


void handle_client(int client_socket) {
    std::cout << "accepted client\n";

    while (true) {
        char buffer[1024];

        ssize_t received = recv(
            client_socket,
            buffer,
            sizeof(buffer),
            0
        );

        if (received == 0) {
            std::cout << "client disconnected\n";
            break;
        }

        if (received < 0) {
            perror("recv");
            break;
        }

        std::string reply = "Server received: " +
                            std::string(buffer, received) +
                            "\n";


        if (send(client_socket, reply.c_str(), reply.size(), 0) < 0) {
            perror("send");
            break;
        }
        std::cout << "Server received: ";
        std::cout.write(buffer, received);
        std::cout << '\n';
    }

    close(client_socket);
    std::cout << "client handler stopped\n";
}

int main(){
    Server server;
    if (server.create_socket()){
        std::cout << "Server Socket created successfully\n";
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
    std::cout << "awaiting client...\n";

    
    bool flag = true;
    while(flag){
        int cli = server.accept_client();
        if (cli >= 0){
            server.get_clients_sockets().push_back(cli);
            std::thread client_thread(handle_client, cli );
            client_thread.detach();
        }
        else{
            flag = false;
        }
    }

   server.close_socket();


    return 0;
}
