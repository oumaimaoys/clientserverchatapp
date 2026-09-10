#include <iostream>
#include "client.h"
#include "message.h"
#include <cstring>
#include <atomic>
#include <thread>
#include <limits>
#include <ctime>
#include <sys/socket.h>
#include <mutex>


std::atomic<bool> running{true};
std::atomic<bool> chat_running{true};
std::mutex cout_mutex;

// helpers
bool read_integer(int& input){
    
    std::cin >> input;
    if(std::cin.fail()){
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;

}

void me_prompt() {
    std::lock_guard<std::mutex> lock(cout_mutex);

    std::cout << "Me: " << std::flush;
}

void menu_input(int msg, int client_socket, int sender_id, std::string text){
    if (msg == 1) {
        Message m(std::time(nullptr), MessageType::LIST_USERS, sender_id, sender_id);
        ssize_t sent = send(
            client_socket,
            m.serialize().c_str(),
            m.serialize().size(),
            0
        );
        char buffer[1024];
        ssize_t received = recv(
            client_socket,
            buffer,
            sizeof(buffer),
            0
        );
        if (received == 0) {
            std::cout << "Server disconnected\n";
        }
        Message message = Message::parse_message(std::string(buffer, static_cast<std::size_t>(received)));
        std::cout << message.get_content() + "\n";
        
    }
    if (msg == 2) {
        Message m(std::time(nullptr), MessageType::CHANGE_USERNAME, sender_id, sender_id, text);
        ssize_t sent = send(
            client_socket,
            m.serialize().c_str(),
            m.serialize().size(),
            0
        );
        char buffer[1024];
        ssize_t received = recv(
            client_socket,
            buffer,
            sizeof(buffer),
            0
        );
        if (received == 0) {
            std::cout << "Server disconnected\n";
        }
        Message message = Message::parse_message(std::string(buffer, static_cast<std::size_t>(received)));
        std::cout << message.get_content() + "\n";
    }
    if (msg == 4) {
        shutdown(client_socket, SHUT_RDWR);
    }
}

void handle_input(int client_socket, int sender_id, int receiver){
    while(chat_running){
        
        std::string msg;
        if (!std::getline(std::cin, msg)) {
            chat_running = false;
            shutdown(client_socket, SHUT_RDWR);
            break;
        }

        if (msg == "/quit") {
            chat_running = false;
            break;
        }
        if (msg.empty()) {
            continue;
        }
        std::string wire_msg = msg;
        Message message(std::time(nullptr), MessageType::CHAT, sender_id, receiver, wire_msg);
        ssize_t sent = send(
            client_socket,
            message.serialize().c_str(),
            message.serialize().size(),
            0
        );

        if (sent < 0) {
            perror("send");
            chat_running = false;
            break;
        }
    }
}

void handle_incoming(int client_socket){
    while(chat_running){
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
            chat_running = false;
            break;
        }

        if (received < 0) {
            if (running) {
                perror("recv");
            }
            chat_running = false;
            break;
        }
        std::lock_guard<std::mutex> lock(cout_mutex);
        Message message = Message::parse_message(std::string(buffer, static_cast<std::size_t>(received)));
        std::cout << "User #" << message.get_sender() << ": " + message.get_content() + "\n";
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

    if (!client.receive_assigned_id()) {
        std::cout << "failed to receive client id\n";
        return 1;
    }

    int client_socket = client.get_socket();
    std::cout << "assigned client id=" << client.get_id() << "\n";

    bool menu_flag{true};

    while (menu_flag){
        // menu
        std::cout << "+---------- COMMANDS MENU ---------+\n";
        std::cout << "| 1. list users.                   |\n";
        std::cout << "| 2. set or change username.       |\n";
        std::cout << "| 3. chat with a user.             |\n";
        std::cout << "| 4. disconnect from server.       |\n";
        std::cout << "+----------------------------------+\n";

        std::cout << "\n";
        std::cout << "Enter command:";
    
        int command;
        if (!read_integer(command) || command < 1 || command > 4) {
            std::cout << "invalid command number\n";
        }
        else if (command == 3) {
            menu_flag = false;
        }
        else if (command == 4){
            menu_input(command, client_socket, client.get_id(), "");
            menu_flag = false;
            return 1;
        }
        else if (command == 2){
            std::cout << "enter a username\n";
            std::string username;
            std::cin >> username;
            menu_input(command, client_socket, client.get_id(), username);
            if(!client.update_username(username)){
                std::cout << "error: failed to update username\n";
            }
        }
        else{
            menu_input(command, client_socket, client.get_id(), "");
        }

    }



    int receiver;
    std::cout << "enter receiver id: ";
    if (!read_integer(receiver)) {
        std::cout << "invalid receiver id\n";
        return 1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::thread input_thread(handle_input, client_socket, client.get_id(), receiver);
    std::thread incoming_thread(handle_incoming, client_socket);

    input_thread.join();
    running = false;
    shutdown(client_socket, SHUT_RDWR);
    incoming_thread.join();

    std::cout << "connection closed\n";
    client.close_connection();


    return 0;
}
