/*
clientList.cpp - Client vector management functions
*/

#include "../include/clientList.hpp"
#include "../include/log.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;

#ifdef MOCK_SEND
ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags);
#define send mock_send
#endif

void ThreadClientList::addClient(int fd, const string &username) {
    lock_guard<mutex> lock(mtx);

    if (username.empty()) {
        LOG_ERROR("Attempted to add client with empty username.", logger);
        send(fd, "Username cannot be empty.\n", 26, 0);
        return;
    }

    for (const auto &client : clients) {
        if (client.username == username) {
            LOG_ERROR("Username: " + username + " is already taken.", logger);
            send(fd, "Username is already taken.\n", 28, 0);
            return;
        }
    }

    clients.push_back({fd, username});
    LOG_INFO("Added client with fd " + to_string(fd) + " and username " + username +
                 " to linked list.",
             logger);
}

void ThreadClientList::deleteClient(int fd) {
    lock_guard<mutex> lock(mtx);
    clients.erase(remove_if(clients.begin(), clients.end(),
                            [fd](const ClientInfo &client) { return client.fd == fd; }),
                  clients.end());
    LOG_INFO("Deleted client with fd " + to_string(fd) + " from linked list.", logger);
}

void ThreadClientList::printList() {
    lock_guard<mutex> lock(mtx);
    LOG_INFO("Printing list of clients...", logger);
    for (const auto &client : clients) {
        LOG_INFO("Client FD: " + to_string(client.fd) + ", Username: " + client.username, logger);
    }
    LOG_INFO("", logger);
}

vector<int> ThreadClientList::getAllFds() {
    lock_guard<mutex> lock(mtx);
    vector<int> fds;
    for (const auto &client : clients) {
        fds.push_back(client.fd);
    }
    return fds;
}

vector<string> ThreadClientList::getUsernames() {
    lock_guard<mutex> lock(mtx);
    vector<string> usernames;
    for (const auto &client : clients) {
        usernames.push_back(client.username);
    }
    return usernames;
}

string ThreadClientList::getUsernameFromFd(int fd) {
    lock_guard<mutex> lock(mtx);
    for (const auto &client : clients) {
        if (client.fd == fd) {
            return client.username;
        }
    }
    return {};
}

int ThreadClientList::getFdFromUsername(const string &username) {
    lock_guard<mutex> lock(mtx);
    for (const auto &client : clients) {
        if (client.username == username) {
            return client.fd;
        }
    }
    return -1;
}

bool ThreadClientList::isUserTaken(const string &username) {
    lock_guard<mutex> lock(mtx);
    for (const auto &client : clients) {
        if (client.username == username) {
            return true;
        }
    }
    return false;
}

void ThreadClientList::changeUsername(int fd, const string &newName, int maxLength) {
    lock_guard<mutex> lock(mtx);
    // Username can't be empty or longer than max length
    if (newName.empty() || newName.length() > static_cast<string::size_type>(maxLength)) {
        string errorMessage =
            "Username must be between 1 and " + to_string(maxLength) + " characters.";
        LOG_ERROR(errorMessage, logger);
        send(fd, errorMessage.c_str(), errorMessage.length(), 0);
        return;
    }

    // Check for control characters
    if (any_of(newName.begin(), newName.end(), [](char c) { return !isprint(c); })) {
        LOG_ERROR("Username contains invalid characters.", logger);
        send(fd, "Username contains invalid characters.\n", 38, 0);
        return;
    }

    // Check if user isn't already taken
    for (const auto &client : clients) {
        if (client.username == newName && client.fd != fd) {
            LOG_ERROR("Username '" + newName + "' is already taken.", logger);
            send(fd, "Username is already taken.\n", 28, 0);
            return;
        }
    }

    // Check if user has space (it can't)
    if (newName.find(' ') != string::npos) {
        LOG_ERROR("Username can't contain spaces.", logger);
        send(fd, "Username can't contain spaces.\n", 32, 0);
        return;
    }

    // Change username
    for (auto &client : clients) {
        if (client.fd == fd) {
            client.username = newName;
            LOG_INFO("Changed username for client " + to_string(fd) + " to " + newName, logger);
            string confirmationMessage = "Your name has been changed to " + newName + "\n";
            send(fd, confirmationMessage.c_str(), confirmationMessage.length(), 0);
            return;
        }
    }
}

void ThreadClientList::broadcastMessage(const string &message, int excludeFd) {
    lock_guard<mutex> lock(mtx);
    for (const auto &client : clients) {
        if (client.fd != excludeFd) {
            send(client.fd, message.c_str(), message.size(), 0);
        }
    }
}
