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

bool Message::set_content(std::string chat){
    this->content = chat;
    return true;
}

int Message::get_sender() const{
    return sender;
}

int Message::get_receiver() const{
    return reciever;
}

MessageType Message::get_type() const {
    return type;
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
    return t + "|" + std::to_string(sender) + "|" + std::to_string(reciever) + "|" + time_string + "|" + content + "\n";
}

Message Message::parse_message(const std::string& msg){
    std::string rest = msg;
    while (!rest.empty() && (rest.back() == '\n' || rest.back() == '\r')) {
        rest.pop_back();
    }

    auto take_field = [&rest](std::string& field) {
        std::size_t separator = rest.find("|");
        if (separator == std::string::npos) {
            return false;
        }
        field = rest.substr(0, separator);
        rest = rest.substr(separator + 1);
        return true;
    };

    std::string type;
    std::string sender;
    std::string reciever;
    std::string timestamp;
    if (!take_field(type) || !take_field(sender) || !take_field(reciever) || !take_field(timestamp)) {
        return Message(0, MessageType::UNKNOWN, 0, 0, msg);
    }
    std::string content = rest;

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

    int s = 0;
    int r = 0;
    try {
        s = std::stoi(sender);
        r = std::stoi(reciever);
    } catch (...) {
        return Message(0, MessageType::UNKNOWN, 0, 0, msg);
    }

    return Message(tmp, t, s, r, content);
}