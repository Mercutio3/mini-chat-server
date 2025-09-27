/**
 * @file commands.hpp
 * @brief Command processing functions.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "clientList.hpp"
#include <string>

using namespace std;

/**
 * @brief Process the /help command. Send help message to client.
 *
 * Sends a predefined help message with available commands a client.
 *
 * @param clientFd File descriptor of the client requesting help.
 */
void processHelpCmd(int clientFd);

/**
 * @brief Process the /list command. Send list of connected users to client.
 *
 * Retrieves a list of connected usernames from the client list
 * and sends it to the specified client, one username per line.
 *
 * @param clientFd File descriptor of the client requesting the list.
 * @param clientList Reference to ThreadClientList instance managing clients.
 */
void processListCmd(int clientFd, ThreadClientList &clientList);

/**
 * @brief Process the /name command. Change client's username.
 *
 * Calls ThreadClientList::changeUsername to attempt to change.
 *
 * @param clientFd File descriptor of the client requesting the name change.
 * @param clientList Reference to the ThreadClientList instance.
 * @param newName New name for the client.
 * @param maxLength Maximum length for the name.
 */
void processNameCmd(int clientFd, ThreadClientList &clientList, const string &newName,
                    int maxLength);

/**
 * @brief Process the /msg command. Send a private message.
 *
 * Parses target username and message from input string, verifies target
 * exists, and sends the private message if valid.
 *
 * @param clientFd File descriptor of the client sending the message.
 * @param clientList Reference to ThreadClientList instance managing clients.
 * @param msg Full message string containing target and message.
 */
void processMsgCmd(int clientFd, ThreadClientList &clientList, const string &msg);

#endif