#include "message.h"

Message::Message(time_t timestamp, MessageType type, int sender, int receiver, std::string content){
    this->timestamp = timestamp;
    this->type = type;
    this->sender = sender;
    this->reciever = receiver;
    this->content = content;
}

std::string Message::get_content(){
    return content;
}


std::string Message::serialize(){
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&timestamp));

    std::string time_string(buffer);

    std::string t{};
    switch (this->type)
    {
    case MessageType::LOGIN:
        t="login";
        break;
    case MessageType::CHAT:
        t = "chat";
        break;
    case MessageType::LOGOUT:
        t = "logout";
        break;
    case MessageType::LIST_USERS:
        t = "list users";
        break;
    case MessageType::CHANGE_USERNAME:
        t = "change username";
        break;
    default:
        t = "unknown";
        break;
    }
    return t + "|" + std::to_string(sender) + "|" + std::to_string(reciever) + "|" + time_string + "|" + content;
}

Message Message::parse_message(const std::string& msg){
    std::size_t separator = msg.find("|");
    std::string type = msg.substr(0, separator);

    std::string mesg = msg.substr(separator+1);
    separator = mesg.find("|");
    std::string sender = mesg.substr(0, separator);

    mesg = msg.substr(separator+1);
    separator = mesg.find("|");
    std::string reciever = mesg.substr(0, separator);

    mesg = mesg.substr(separator+1);
    separator = mesg.find("|");
    std::string timestamp = mesg.substr(0, separator);

    std::string content = mesg.substr(separator+1);

    MessageType t;
    if( type == "login"){
        t = MessageType::LOGIN;
    }
    else if( type == "chat"){
        t = MessageType::CHAT;
    }
    else if( type == "change username"){
        t = MessageType::CHANGE_USERNAME;
    }
    else if( type == "list users"){
        t = MessageType::LIST_USERS;
    }
    else if( type == "logout"){
        t = MessageType::LOGOUT;
    }
    else{
        t = MessageType::UNKNOWN;
    }

    
    std::tm tm = {};
    std::istringstream ss(timestamp);

    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    time_t tmp = std::mktime(&tm);
    
    int s = std::stoi(sender);
    int r = std::stoi(reciever);

    return Message(tmp, t, s, r, content);
}