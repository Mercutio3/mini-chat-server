#include "../include/clientList.hpp"
#include "../include/commands.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags;
    cout << "MOCKING SEND with fd " << sockfd << " and message: \"" << string((char *)buf, len)
         << "\"" << endl;
    return len; // Return buffer len for testing purposes
}

int main() {
    cout << "Starting command tests...\n";

    ThreadClientList clientList;
    clientList.addClient(1, "UserA");

    // Test /help
    processHelpCmd(1);

    // Test list command with one client
    processListCmd(1, clientList);

    clientList.addClient(2, "UserB");

    // Test list command with more than one client
    processListCmd(1, clientList);

    // Test valid name command
    processNameCmd(1, clientList, "NewUserA", 64);
    assert(clientList.getUsernameFromFd(1) == "NewUserA");

    // Test invalid name commands
    processNameCmd(1, clientList, "UserB", 64); // Taken
    assert(clientList.getUsernameFromFd(1) == "NewUserA");
    processNameCmd(1, clientList, "", 64); // Invalid
    assert(clientList.getUsernameFromFd(1) == "NewUserA");
    processNameCmd(1, clientList, "New UserA", 64); // Has space
    assert(clientList.getUsernameFromFd(1) == "NewUserA");
    processNameCmd(1, clientList,
                   "An extremely and unreasonably long username that will exceed the server "
                   "default of sixty four character",
                   64);
    assert(clientList.getUsernameFromFd(1) ==
           "NewUserA"); // None of the invalids resulted in a change

    // Test message command
    processMsgCmd(1, clientList, "UserB Private message test!");

    processMsgCmd(1, clientList, "NewUserA Private message test!");

    processMsgCmd(1, clientList, "UserC Private message test!");

    processMsgCmd(1, clientList, "");

    // Cleanup
    clientList.deleteClient(1);
    clientList.deleteClient(2);
    assert(clientList.getAllFds().empty());

    cout << "Testing complete." << endl;
    return 0;
}