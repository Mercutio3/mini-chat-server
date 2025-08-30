#ifndef COMMANDS_H
#define COMMANDS_H

#include "clientList.h"

void processHelpCmd(int client_fd);
void processListCmd(int client_fd, clientNode *head, int maxLength);
void processNameCmd(int client_fd, clientNode *head, const char *newName,
                    int maxLength);
void processMsgCmd(int client_fd, clientNode *head, char *message,
                   int maxLength);

#endif