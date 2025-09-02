#include "../include/clientList.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
    cout << "Starting clientList tests...\n";

    ThreadClientList clientList;

    // Test adding clients, checking list integrity after each addition
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    clientList.addClient(3, "User3");
    vector<int> fds = clientList.getAllFds();
    assert(fds.size() == 3 && fds[0] == 1 && fds[1] == 2 && fds[2] == 3);

    // Get usernames
    assert(clientList.getUsernameFromFd(1) == "User1");
    assert(clientList.getUsernameFromFd(2) == "User2");
    assert(clientList.getUsernameFromFd(3) == "User3");
    assert(clientList.getUsernameFromFd(10) == "");

    // Test deleting clients, checking list integrity
    clientList.deleteClient(2);
    fds = clientList.getAllFds();
    assert(fds.size() == 2 && fds[0] == 1 && fds[1] == 3);
    assert(clientList.getUsernameFromFd(2).empty());

    // Test changing username
    clientList.changeUsername(3, "NewUser3", 64);
    assert(clientList.getUsernameFromFd(3) == "NewUser3");

    // Adding and deleting a large number of clients
    for (int i = 4; i <= 100; i++) {
        clientList.addClient(i, "User" + to_string(i));
    }
    fds = clientList.getAllFds();
    assert(fds.size() == 99);

    // Delete all clients
    for (int fd : fds) {
        clientList.deleteClient(fd);
    }
    fds = clientList.getAllFds();
    assert(fds.empty());

    cout << "Testing complete." << endl;
    return 0;
}