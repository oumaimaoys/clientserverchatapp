#include <iostream>
#include "server.h"

#include <string>
#include <thread>
#include <functional>
#include <unistd.h>


void handle_client(int client_socket, Server& server) {
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
            std::vector<int> clients = server.get_clients_sockets();
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            break;
        }

        if (received < 0) {
            perror("recv");
            break;
        }

        std::string reply =  std::string(buffer, received) +
                            "\n";

        server.send_all(server.get_clients_sockets(), reply);
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
            std::thread client_thread(handle_client, cli,  std::ref(server));
            client_thread.detach();
        }
        else{
            flag = false;
        }
    }

   server.close_socket();


    return 0;
}
