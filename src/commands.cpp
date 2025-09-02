/*
commands.cpp - Command processing functions
*/

#include "../include/commands.hpp"
#include "../include/clientList.hpp"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#ifdef MOCK_SEND
ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags);
#define send mock_send
#endif

#define MAX_DATA_SIZE 200

// Send list of commands to client
void processHelpCmd(int clientFd) {
    const char *helpMessage = "Available commands:\n"
                              "/help - Show this help message\n"
                              "/list - List all connected clients\n"
                              "/name <new_name> - Change your name\n"
                              "/msg <message> - Send a message to all clients\n";
    send(clientFd, helpMessage, strlen(helpMessage), 0);
}

// Send list of connected users to client
void processListCmd(int clientFd, ThreadClientList &clientList) {
    vector<string> clientNames = clientList.getUsernames();
    for (const auto &name : clientNames) {
        string nameWithNewline = name + "\n";
        send(clientFd, nameWithNewline.c_str(), nameWithNewline.length(), 0);
    }
}

// Change username; username can't be empty or longer than max length
void processNameCmd(int clientFd, ThreadClientList &clientList, const string &newName,
                    int maxLength) {
    clientList.changeUsername(clientFd, newName, maxLength);
}

// Send a private message
void processMsgCmd(int clientFd, ThreadClientList &clientList, const string &msg) {
    // Parse and verify formatting
    size_t firstSpace = msg.find(' ');
    if (firstSpace == string::npos) {
        string error = "Usage: /msg <target username> <message>";
        send(clientFd, error.c_str(), error.length(), 0);
        return;
    }
    string target = msg.substr(0, firstSpace);
    string privMsg = msg.substr(firstSpace + 1);

    int targetFd = clientList.getFdFromUsername(target);
    string senderName = clientList.getUsernameFromFd(clientFd);

    // Make sure target exists
    if (targetFd == -1) {
        string error = "Username not found.";
        send(clientFd, error.c_str(), error.length(), 0);
        return;
    }

    // If found, send message;
    string fullMsg = "Private message from " + senderName + ": " + privMsg;
    send(targetFd, fullMsg.c_str(), fullMsg.length(), 0);
}
