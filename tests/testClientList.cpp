#include "../include/clientList.hpp"
#include "../include/log.hpp"
#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

ChatLogger logger;

ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags;
    cout << "MOCKING SEND with fd " << sockfd << " and message: \"" << string((char *)buf, len)
         << "\"" << endl;
    return len; // Return buffer len for testing purposes
}

class ClientListTest : public ::testing::Test {
  protected:
    ThreadClientList clientList;
};

TEST_F(ClientListTest, AddClientsAndCheckListIntegrity) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    clientList.addClient(3, "User3");
    vector<int> fds = clientList.getAllFds();
    EXPECT_EQ(fds.size(), static_cast<size_t>(3));
    EXPECT_EQ(fds[0], 1);
    EXPECT_EQ(fds[1], 2);
    EXPECT_EQ(fds[2], 3);
}

TEST_F(ClientListTest, GetUsernames) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    clientList.addClient(3, "User3");
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
    EXPECT_EQ(clientList.getUsernameFromFd(2), "User2");
    EXPECT_EQ(clientList.getUsernameFromFd(3), "User3");
    EXPECT_EQ(clientList.getUsernameFromFd(10), "");
}

TEST_F(ClientListTest, DeleteClients) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    clientList.addClient(3, "User3");
    clientList.deleteClient(2);
    vector<int> fds = clientList.getAllFds();
    EXPECT_EQ(fds.size(), static_cast<size_t>(2));
    EXPECT_EQ(fds[0], 1);
    EXPECT_EQ(fds[1], 3);
    clientList.deleteClient(1);
    fds = clientList.getAllFds();
    EXPECT_EQ(fds.size(), static_cast<size_t>(1));
    EXPECT_EQ(fds[0], 3);
    clientList.deleteClient(3);
    fds = clientList.getAllFds();
    EXPECT_TRUE(fds.empty());
}

TEST_F(ClientListTest, ChangeUsername) {
    clientList.addClient(1, "User1");
    clientList.changeUsername(1, "NewUser1", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "NewUser1");
}

TEST_F(ClientListTest, ClientListTest_AddAndDeleteManyClients) {
    for (int i = 4; i <= 100; i++) {
        clientList.addClient(i, "User" + to_string(i));
    }
    vector<int> fds = clientList.getAllFds();
    EXPECT_EQ(fds.size(), static_cast<size_t>(97)); // 3 clients already added in previous tests

    // Delete all clients
    for (int fd : fds) {
        clientList.deleteClient(fd);
    }
    fds = clientList.getAllFds();
    EXPECT_TRUE(fds.empty());
}

TEST_F(ClientListTest, DuplicateUsername) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User1");
    EXPECT_EQ(clientList.getFdFromUsername("User1"), 1);
    EXPECT_NE(clientList.getUsernameFromFd(2), "User1");
}

TEST_F(ClientListTest, EmptyUsername) {
    clientList.addClient(1, "");
    EXPECT_EQ(clientList.getUsernameFromFd(1), "");
}

TEST_F(ClientListTest, ChangeUsernameToDuplicate) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    clientList.changeUsername(2, "User1", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(2), "User2");
}

TEST_F(ClientListTest, ChangeUsernameToEmpty) {
    clientList.addClient(1, "User1");
    clientList.changeUsername(1, "", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}