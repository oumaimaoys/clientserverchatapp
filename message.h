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
    Message(time_t timmestamp, MessageType type, int sender, int receiver, std::string content);

    std::string get_content();
    int get_receiver() const;
    std::string serialize();
    static Message parse_message(const std::string& msg);


};

#endif