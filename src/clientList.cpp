/*
clientList.cpp - Client vector management functions
*/

#include "../include/clientList.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;

void ThreadClientList::addClient(int fd, const string &username) {
    lock_guard<mutex> lock(mtx);
    clients.push_back({fd, username});
    cout << "Added client with fd " << fd << " and username " << username << " to linked list."
         << endl;
}

void ThreadClientList::deleteClient(int fd) {
    lock_guard<mutex> lock(mtx);
    clients.erase(remove_if(clients.begin(), clients.end(),
                            [fd](const ClientInfo &client) { return client.fd == fd; }),
                  clients.end());
    cout << "Deleted client with fd " << fd << " from linked list." << endl;
}

void ThreadClientList::printList() {
    lock_guard<mutex> lock(mtx);
    cout << "Printing list of clients..." << endl;
    for (const auto &client : clients) {
        cout << "Client FD: " << client.fd << ", Username: " << client.username;
    }
    cout << endl;
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
        cerr << "Username must be between 1 and " << maxLength << " characters." << endl;
        send(fd, "Username must be between 1 and 64 characters.\n", 65, 0);
        return;
    }

    // Check if user isn't already taken
    for (const auto &client : clients) {
        if (client.username == newName) {
            cerr << "Username '" << newName << "' is already taken." << endl;
            send(fd, "Username is already taken.\n", 28, 0);
            return;
        }
    }

    // Check if user has space (it can't)
    if (newName.find(' ') != string::npos) {
        cerr << "Username can't contain spaces." << endl;
        send(fd, "Username can't contain spaces.\n", 32, 0);
        return;
    }

    // Change username
    for (auto &client : clients) {
        if (client.fd == fd) {
            client.username = newName;
            cout << "Changed username for client " << fd << " to " << newName << endl;
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
