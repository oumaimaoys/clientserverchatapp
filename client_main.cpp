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
#include <sys/select.h>


std::atomic<bool> running{true};
std::atomic<bool> chat_running{true};
std::atomic<bool> menu_flag{true};

std::mutex cout_mutex;

// helpers
void show_menu(){
    // menu
    std::cout << "+---------- COMMANDS MENU ---------+\n";
    std::cout << "| 1. list users.                   |\n";
    std::cout << "| 2. set or change username.       |\n";
    std::cout << "| 3. chat with a user.             |\n";
    std::cout << "| 4. disconnect from server.       |\n";
    std::cout << "+----------------------------------+\n";

    std::cout << "\n";
    std::cout << "Enter command:";
}

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
        running = false;
    }
}

void handle_input(int client_socket, int sender_id, int receiver){
    while(chat_running){
        std::string msg;
        if (!std::getline(std::cin, msg)) {
            chat_running = false;
            running = false;              // EOF really does mean "leave"
            shutdown(client_socket, SHUT_RDWR);
            break;
        }

        if (msg == "/quit") {
            chat_running = false;         // just leave chat, keep connection
            break;
        }
        if (msg.empty()) {
            continue;
        }
        Message message(std::time(nullptr), MessageType::CHAT, sender_id, receiver, msg);
        ssize_t sent = send(client_socket, message.serialize().c_str(), message.serialize().size(), 0);
        if (sent < 0) {
            perror("send");
            chat_running = false;
            running = false;
            break;
        }
    }
}


void handle_incoming(int client_socket){
    while(chat_running){
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);

        timeval tv{0, 200000}; // 200ms poll, so we can re-check chat_running
        int ready = select(client_socket + 1, &readfds, nullptr, nullptr, &tv);

        if (ready < 0) {
            if (running) perror("select");
            chat_running = false;
            break;
        }
        if (ready == 0) {
            continue; // timed out, loop and re-check chat_running
        }

        char buffer[1024];
        ssize_t received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            std::cout << "Server disconnected\n";
            running = false;
            chat_running = false;
            break;
        }
        if (received < 0) {
            if (running) perror("recv");
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

    std::string server_ip;
    std::cout << "Enter server Ip";
    std::cin >> server_ip;

    bool connection = client.connect_to_server(server_ip, 5000);

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

    
    while (running) {
        menu_flag = true;
        while (menu_flag){
            show_menu();

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

        if (!running) break; // disconnected (command 4 or server dropped)

        int receiver;
        std::cout << "enter receiver id: ";
        if (!read_integer(receiver)) {
            std::cout << "invalid receiver id\n";
            continue; // back to menu
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        chat_running = true;
        std::thread input_thread(handle_input, client_socket, client.get_id(), receiver);
        std::thread incoming_thread(handle_incoming, client_socket);

        input_thread.join();
        chat_running = false;   // make sure incoming thread's loop condition also flips
        incoming_thread.join();

        // loop back to menu unless we've fully disconnected
    }

    shutdown(client_socket, SHUT_RDWR);
    std::cout << "connection closed\n";
    client.close_connection();

    return 0;
}
