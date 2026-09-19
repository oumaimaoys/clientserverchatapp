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

        // handle list users command
        std::string message_to_send="+--Connected clients:--+\n";
        if (message.get_type() == MessageType::LIST_USERS){
            for( client_data cli: server.get_clients_sockets()){
                message_to_send += "| - User #" + std::to_string(cli.client_id) + "\n";
            }
            message_to_send += "-------------------\n";
            message.set_content(message_to_send);
        }

        // handle update username command

        if (message.get_type() == MessageType::CHANGE_USERNAME){
            std::vector<client_data> clients_list = server.get_clients_sockets();
            for (client_data& cli : clients_list){
                if (cli.client_id == message.get_sender()){
                    cli.username = message.get_content();
                    break;
                }
            }
            message_to_send = "Success updating username!";
            message.set_content(message_to_send);
        }

        
        if (!server.send_to(dest, message.serialize())) {
            std::cout << "unknown receiver id=" << dest << '\n';
        }
    }

    close(client_socket);
    std::cout << "client handler stopped\n";
}

int main(){
    Server server(5000);
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
