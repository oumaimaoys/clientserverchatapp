#include <iostream>
#include "server.h"
#include "message.h"

#include <string>
#include <thread>
#include <functional>
#include <unistd.h>
#include <algorithm>


void handle_client(client_data client, Server& server) {
    int client_socket = client.client_socket;
    std::cout << "accepted client id=" << client.client_id
              << " address=" << client.address << "\n";

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
            auto& clients = server.get_clients_sockets();
            clients.erase(
                std::remove_if(
                    clients.begin(),
                    clients.end(),
                    [client_socket](const client_data& c) {
                        return c.client_socket == client_socket;
                    }
                ),
                clients.end()
            );
            break;
        }

        if (received < 0) {
            perror("recv");
            break;
        }

        std::string raw(buffer, static_cast<std::size_t>(received));
        Message message = Message::parse_message(raw);
        int dest = message.get_receiver();

        std::cout << "Server received from id=" << client.client_id
                  << " to id=" << dest << ": " << message.get_content() << '\n';

        if (!server.send_to(dest, message.serialize())) {
            std::cout << "unknown receiver id=" << dest << '\n';
        }
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
        client_data cli = server.accept_client();
        if (cli.client_socket >= 0){
            std::thread client_thread(handle_client, cli, std::ref(server));
            client_thread.detach();
        }
        else{
            flag = false;
        }
    }

   server.close_socket();


    return 0;
}
