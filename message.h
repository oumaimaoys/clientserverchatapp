#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

enum class MessageType{
    LOGIN,
    CHAT,
    LIST_USERS,
    CHANGE_USERNAME,
    LOGOUT,
    UNKNOWN

};

class Message{
private:
    time_t timestamp;
    MessageType type;
    int sender;
    int reciever;
    std::string content;

public:
    Message(time_t timestamp, MessageType type, int sender,  int receiver = -1, std::string content = "");

    std::string get_content();
    bool set_content(std::string text);
    int get_receiver() const;
    int get_sender() const;
    MessageType get_type() const;
    std::string serialize();
    static Message parse_message(const std::string& msg);


};

#endif