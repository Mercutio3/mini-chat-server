/*
commands.h - Command processing declarations
*/

#ifndef COMMANDS_H
#define COMMANDS_H

#include "clientList.hpp"
#include <string>

using namespace std;

void processHelpCmd(int clientFd);
void processListCmd(int clientFd, ThreadClientList &clientList);
void processNameCmd(int clientFd, ThreadClientList &clientList, const string &newName,
                    int maxLength);
void processMsgCmd(int clientFd, ThreadClientList &clientList, const string &msg);

#endif