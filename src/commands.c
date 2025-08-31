#include "../include/commands.h"
#include "../include/clientList.h"
#include "../include/log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef MOCK_SEND
ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags);
#define send mock_send
#endif

#define MAXDATASIZE 200

// #define DEBUG

void processHelpCmd(int client_fd) {
    // Send list of commands to client
    const char helpMsg[] =
        "Available commands:\n/help - Show available commands\n/exit - "
        "Disconnect from server\n/list - Show connected users\n/name <new_name> "
        "- Change username\n/msg <username> - Send a private message to someone.";
    if (send(client_fd, helpMsg, strlen(helpMsg), 0) == -1) {
        perror("send error");
    }
}

void processListCmd(int client_fd, struct clientNode *head, int maxLength) {
    // Send list of connected users to client
    struct clientNode *current = head;
    const char connectedUsersMsg[] = "Connected users:\n";
    if (send(client_fd, connectedUsersMsg, strlen(connectedUsersMsg), 0) == -1) {
        perror("send error");
    }
    while (current != NULL) {
        char userEntry[maxLength + 2];
        snprintf(userEntry, sizeof(userEntry), "%s\n", current->username);
        if (send(client_fd, userEntry, strlen(userEntry), 0) == -1) {
            perror("send error");
        }
        current = current->next;
    }
}

void processNameCmd(int client_fd, struct clientNode *head, const char *newName, int maxLength) {
    // Change username
    // Username can't be empty or longer than max length
    if (strlen(newName) == 0 || (size_t)strlen(newName) >= (size_t)maxLength - 1 ||
        strchr(newName, ' ') != NULL) {
        const char invalidNameMsg[] = "Invalid username. Please choose a different one.";
        if (send(client_fd, invalidNameMsg, strlen(invalidNameMsg), 0) == -1) {
            perror("send error");
        }
    } else {
        struct clientNode *current = head;
        // Check if name is taken
        int nameTaken = 0;
        while (current != NULL) {
            if (strcmp(current->username, newName) == 0) {
                nameTaken = 1;
                break;
            }
            current = current->next;
        }
        if (nameTaken) {
            const char nameTakenMsg[] = "Username already taken. Please choose a different one.";
            if (send(client_fd, nameTakenMsg, strlen(nameTakenMsg), 0) == -1) {
                perror("send error");
            }
        } else {
            // Change the name
            current = head;
            while (current != NULL) {
                if (current->fd == client_fd) {
                    strncpy(current->username, newName, sizeof(current->username) - 1);
                    current->username[sizeof(current->username) - 1] = '\0';
                    break;
                }
                current = current->next;
            }
            const char nameChangedMsg[] = "Username changed successfully.";
            LOG_INFO("Client with fd %d changed their name to %s.", client_fd, newName);
            logChatMessage("Client with fd %d changed their name to %s.", client_fd, newName);
            if (send(client_fd, nameChangedMsg, strlen(nameChangedMsg), 0) == -1) {
                perror("send error");
            }
        }
    }
}

void processMsgCmd(int client_fd, struct clientNode *head, char *message, int maxLength) {
    // Separate string, check if formatted properly
    strtok(message, " \n");
    char *target = strtok(NULL, " \n");
    char *privMsg = strtok(NULL, "\n");
    LOG_DEBUG("Target: %s", target);
    LOG_DEBUG("Message: %s", privMsg);
    if (target == NULL || privMsg == NULL) {
        const char invalidFormat[] = "Invalid format. Use /msg <username> <message>";
        if (send(client_fd, invalidFormat, strlen(invalidFormat), 0) == -1) {
            perror("send error");
        }
    } else {
        LOG_DEBUG("Private message formatted well. Proceeding with search.");
        struct clientNode *current = head;

        // Look for specified username
        int targetFound = 0;
        int target_fd;
        while (current != NULL) {
            if (strcmp(current->username, target) == 0) {
                targetFound = 1;
                target_fd = current->fd;
                break;
            }
            current = current->next;
        }
        char *senderName = getUserNameFromFD(head, client_fd);
        if (targetFound == 0) {
            LOG_INFO("%s tried to send a private message to non-existent user %s.", senderName,
                     target);
            // If not found
            const char userNotFound[] = "Username not found.";
            if (send(client_fd, userNotFound, strlen(userNotFound), 0) == -1) {
                perror("send error");
            }
        } else {
            // If found, send message;
            char fullMsg[MAXDATASIZE + maxLength + 12]; // Extra space for formatting
            snprintf(fullMsg, sizeof(fullMsg), "Private message from %s: %s", senderName, privMsg);
            if (send(target_fd, fullMsg, strlen(fullMsg), 0) == -1) {
                perror("send error");
            }
            LOG_INFO("%s sent a private message to %s.", senderName, target);
            logChatMessage("%s -> %s: %s", senderName, target, privMsg);
        }
    }
}