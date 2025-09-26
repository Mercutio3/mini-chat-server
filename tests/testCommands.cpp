#include "../include/clientList.hpp"
#include "../include/commands.hpp"
#include "../include/log.hpp"
#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

ChatLogger logger;

ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags;
    cout << "MOCKING SEND with fd " << sockfd << " and message: \"" << string((char *)buf, len)
         << "\"" << endl;
    return len; // Return buffer len for testing purposes
}

class CommandsTest : public ::testing::Test {
  protected:
    ThreadClientList clientList;
};

TEST_F(CommandsTest, HelpCommand) {
    clientList.addClient(1, "User1");
    processHelpCmd(1);
}

TEST_F(CommandsTest, ListCommand) {
    clientList.addClient(1, "User1");
    processListCmd(1, clientList);
    clientList.addClient(2, "User2");
    processListCmd(1, clientList);
    clientList.deleteClient(2);
    processListCmd(1, clientList);
}

TEST_F(CommandsTest, ListCommandEmptyClientlist) { processListCmd(1, clientList); }

TEST_F(CommandsTest, NameCommandValidChange) {
    clientList.addClient(1, "User1");
    processNameCmd(1, clientList, "NewUser1", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "NewUser1");
}

TEST_F(CommandsTest, NameCommandTakenName) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    processNameCmd(1, clientList, "User2", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

TEST_F(CommandsTest, NameCommandEmptyName) {
    clientList.addClient(1, "User1");
    processNameCmd(1, clientList, "", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

TEST_F(CommandsTest, NameCommandHasSpace) {
    clientList.addClient(1, "User1");
    processNameCmd(1, clientList, "New User1", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

TEST_F(CommandsTest, NameCommandTooLong) {
    clientList.addClient(1, "User1");
    processNameCmd(1, clientList,
                   "An extremely and unreasonably long username that will exceed the server "
                   "default of sixty four character",
                   64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

TEST_F(CommandsTest, NameCommandControlChars) {
    clientList.addClient(1, "User1");
    processNameCmd(1, clientList, "New\x01User1", 64);
    EXPECT_EQ(clientList.getUsernameFromFd(1), "User1");
}

TEST_F(CommandsTest, MsgCommandValid) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    processMsgCmd(1, clientList, "User2 Hello User2!");
}

TEST_F(CommandsTest, MsgCommandSelfMessage) {
    clientList.addClient(1, "User1");
    processMsgCmd(1, clientList, "User1 Hello Myself!");
}

TEST_F(CommandsTest, MsgCommandNonExistentUser) {
    clientList.addClient(1, "User1");
    processMsgCmd(1, clientList, "User2 Hello User2!");
}

TEST_F(CommandsTest, MsgCommandEmptyMessage) {
    clientList.addClient(1, "User1");
    clientList.addClient(2, "User2");
    processMsgCmd(1, clientList, "User2 ");
}

TEST_F(CommandsTest, MsgCommandEmptyCommand) {
    clientList.addClient(1, "User1");
    processMsgCmd(1, clientList, "");
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}