/*
kserver.c - Simple echoing server using kqueue.
*/

#include "../include/clientList.h"
#include "../include/commands.h"
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
#include <time.h>
#include <stdbool.h>

#define BACKLOG 5
#define MAXDATASIZE 200
#define USERNAME_MAX_LENGTH 64

// #define RECEIVE_OWN_MESSAGE //Server sends message back to sender as well

volatile sig_atomic_t run = 1;
int sock_fd = -1;

// Close server socket gracefully
void shutdownServer(struct clientNode *current) {
    // Check if all nodes were freed, for robustness
    while (current != NULL) {
        struct clientNode *next = current->next;
        free(current);
        current = next;
    }

    // Close server socket
    if (sock_fd != -1) {
        if (close(sock_fd) == -1) {
            perror("Error closing.");
        } else {
            LOG_DEBUG("Server socket closed successfully.");
        }
        sock_fd = -1;
    }
}

// Loops through buffer ensuring legal UTF-8
bool isUTF8(const char *s, size_t len){
    size_t i = 0;
    while (i < len) {
        unsigned char c = (unsigned char)s[i];
        if(c <= 0x7F) {
            // 1-byte character (ASCII)
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character; next byte must start with 10
            if(i + 1 >= len || (s[i + 1] & 0xC0) != 0x80 || c < 0xC2){
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0){
            // 3-byte UTF-8 character; next bytes must start with 10
            if (i + 2 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80){
                return false;
            }

            // Check for overlong encoding
            if (c == 0xE0 && (unsigned char)s[i + 1] < 0xA0) {
                return false;
            }
            if (c == 0xED && (unsigned char)s[i + 1] >= 0xA0) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0){
            // 4-byte UTF-8 character; next bytes must start with 10
            if (i + 3 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80 || (s[i + 3] & 0xC0) != 0x80){
                return false;
            }

            // Check for overlong encoding
            if (c == 0xF0 && (unsigned char)s[i + 1] < 0x90) {
                return false;
            }
            if (c == 0xF4 && (unsigned char)s[i + 1] > 0x8F) {
                return false;
            }
            if (c > 0xF4) {
                return false;
            }
            i += 4;
        } else {
            return false; // Invalid byte
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *servInfo, *p;
    socklen_t sin_size;
    int yes = 1;
    // char s[INET6_ADDRSTRLEN]; TBD
    int rv;

    char message[MAXDATASIZE];
    int numBytes;
    const char shutdownMsg[] = "SERVER_SHUTDOWN";
    char port[6] = "5223"; // Default port

    // Print usage if no port is provided
    if (argc > 2) {
        fprintf(stderr, "Usage: ./kserver [port]\n");
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        strncpy(port, argv[1], sizeof(port) - 1);
        port[sizeof(port) - 1] = '\0';
    }

    int intPort = atoi(port);
    if(intPort < 1024 || intPort > 65535){
        LOG_ERROR("Invalid port. Port must be between 1024 and 65535.");
        return EXIT_FAILURE;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use local IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servInfo)) != 0) {
        LOG_ERROR("Getaddrinfo: %s", gai_strerror(rv));
        return EXIT_FAILURE;
    }

    // Loop thorugh results and bind to first available address
    for (p = servInfo; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Server socket error");
            continue;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("Setsockopt error");
            return EXIT_FAILURE;
        }

        if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_fd);
            perror("Server bind error");
            continue;
        }

        break;
    }
    freeaddrinfo(servInfo); // Not used after here

    if (p == NULL) {
        LOG_ERROR("Failed to bind.");
        return EXIT_FAILURE;
    }

    if (listen(sock_fd, BACKLOG) == -1) {
        perror("Server listening error");
    }

    LOG_DEBUG("Server socket creation and listening successful.");

    // Set up kqueue instance
    int kq = kqueue();
    if (kq == -1) {
        perror("kqueue creation error");
        return EXIT_FAILURE;
    }

    // Register listening socket, retrying if call fails with EINTR
    struct kevent change;
    int ret;
    EV_SET(&change, sock_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    do {
        ret = kevent(kq, &change, 1, NULL, 0, NULL);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1) {
        perror("kevent listening socket error");
        return EXIT_FAILURE;
    }

    // Register SIGINT for kqueue monitoring, retrying if call fails with EINTR
    EV_SET(&change, SIGINT, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
    do {
        ret = kevent(kq, &change, 1, NULL, 0, NULL);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1) {
        perror("error registering SIGINT");
        return EXIT_FAILURE;
    }

    // Do the same for SIGTERM
    EV_SET(&change, SIGTERM, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
    do {
        ret = kevent(kq, &change, 1, NULL, 0, NULL);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1) {
        perror("error registering SIGTERM");
        return EXIT_FAILURE;
    }

    // Ignore default signal handling so we use our own
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    // Inialize linked list of clients
    struct clientNode *connectedClients = NULL;

    LOG_INFO("Server started and listening on port %s.", port);
    openChatLog("chatLog.log");
    logChatMessage("Server started and listening on port %s.", port);
    LOG_INFO("Press Ctrl+C to shut down server.");

    // Main kevent loop
    while (run) {
        struct kevent events[64];

        // Call kevent to get array of ready events
        int newEvent = kevent(kq, NULL, 0, events, 64, NULL);
        if (newEvent == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("Kevent wait error");
            break;
        }

        // Process each event
        for (int i = 0; i < newEvent; i++) {
            int event_fd = (int)events[i].ident;

            // Handle signals
            if (events[i].filter == EVFILT_SIGNAL) {
                LOG_DEBUG("Signal received!");

                if (events[i].ident == SIGINT || events[i].ident == SIGTERM) {
                    LOG_DEBUG("Termination signal received. Shutting down...");
                    // Traverse client socket linked list, closing all sockets
                    struct clientNode *current = connectedClients;
                    while (current != NULL) {
                        int fd = current->fd;
                        struct clientNode *next = current->next;

                        LOG_DEBUG("Closing client with fd %d.", fd);
                        // Send a shutdown message to client
                        if (send(fd, shutdownMsg, strlen(shutdownMsg), 0) == -1) {
                            perror("send error");
                        }

                        // Delete from kqueue and linked list, then close the socket
                        EV_SET(&change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                        if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
                            if (errno == EINTR) {
                                continue;
                            }
                            perror("kevent delete error");
                        }

                        connectedClients = deleteClient(connectedClients, fd);
#ifdef DEBUG
                        printList(connectedClients);
#endif
                        close(fd);
                        current = next;
                    }
                    run = 0;
                    break;
                }
            }

            if (event_fd == sock_fd) {
                // Event is listening socket; accept new connection
                LOG_DEBUG("Accepting connection.");
                struct sockaddr_storage client_addr;

                sin_size = sizeof client_addr;
                int client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);

                // accept() error handling
                if (client_fd == -1) {
                    if (errno == EMFILE || errno == ENFILE) {
                        // Accept failed due to file descriptor exhaustion
                        LOG_ERROR("Max file descriptors reached. Server will continue but "
                                  "won't accept new connections.");
                    } else {
                        // Other error
                        perror("Error accepting");
                    }
                    continue;
                }

                // Add client to kqueue and linked list
                EV_SET(&change, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
                    if (errno == EINTR) {
                        continue;
                    }
                    perror("kevent add error");
                }

                connectedClients = addClient(connectedClients, client_fd);
#ifdef DEBUG
                printList(connectedClients);
#endif
                LOG_INFO("Client %d connected.", client_fd);
                logChatMessage("Client %d connected.", client_fd);
            } else {
                // Event is client socket; receive data
                if ((numBytes = recv(event_fd, message, MAXDATASIZE - 1, 0)) == -1) {
                    perror("recv error.");
                    return EXIT_FAILURE;
                }

                // Verify if message is valid UTF-8
                if (!isUTF8(message, (size_t)numBytes)) {
                    LOG_ERROR("Invalid UTF-8 message from client %d.", event_fd);
                    continue;
                }

                // Client either disconneted or error occurred
                if (numBytes <= 0) {
                    if (numBytes == 0) {
                        // Disconnected
                        LOG_DEBUG("Client %d disconnecting...", event_fd);
                    } else {
                        // Error
                        perror("recv error");
                    }

                    // In either case, remove socket from kqueue and linked list, close it
                    EV_SET(&change, event_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
                        if (errno == EINTR) {
                            continue;
                        }
                        perror("kevent delete error");
                    }
                    connectedClients = deleteClient(connectedClients, event_fd);
#ifdef DEBUG
                    printList(connectedClients);
#endif
                    LOG_INFO("Client %d disconnected.", event_fd);
                    logChatMessage("Client %d disconnected.", event_fd);
                    close(event_fd);
                } else {
                    // Print message received
                    message[numBytes] = '\0';
                    char *senderName = getUserNameFromFD(connectedClients, event_fd);
                    printf("%s: %s\n", senderName, message);
                    // Write message on log
                    logChatMessage("%s: %s", senderName, message);

                    // Check if user entered '/' for commands
                    if (message[0] == '/') {
                        if (strcmp(message, "/exit") == 0) {
// Client wants to disconnect
#ifdef DEBUG
                            printf("Client %d sent /exit command. Disconnecting...\n", event_fd);
#endif

                            // Remove socket from kqueue and linked list, close it
                            EV_SET(&change, event_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                            if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
                                if (errno == EINTR) {
                                    continue;
                                }
                                perror("kevent delete error");
                            }
                            connectedClients = deleteClient(connectedClients, event_fd);
#ifdef DEBUG
                            printList(connectedClients);
#endif
                            LOG_INFO("Client %d disconnected.", event_fd);
                            close(event_fd);
                        } else if (strcmp(message, "/help") == 0) {
                            processHelpCmd(event_fd);
                            LOG_INFO("Sent list of commands to Client %d.", event_fd);
                        } else if (strcmp(message, "/list") == 0) {
                            processListCmd(event_fd, connectedClients, USERNAME_MAX_LENGTH);
                            LOG_INFO("Sent list of connected users to Client %d.", event_fd);
                        } else if (strncmp(message, "/name ", 6) == 0) {
                            processNameCmd(event_fd, connectedClients, message + 6,
                                           USERNAME_MAX_LENGTH);
                        } else if (strncmp(message, "/msg", 4) == 0) {
                            // Private message
                            processMsgCmd(event_fd, connectedClients, message, USERNAME_MAX_LENGTH);
                        } else {
                            // Unknown command
                            const char unknownCmdMsg[] = "Unknown command.";
                            if (send(event_fd, unknownCmdMsg, strlen(unknownCmdMsg), 0) == -1) {
                                perror("send error");
                            }
                        }
                    } else {
// Regular message; echo to all clients (and optionally to sender)
#ifdef RECEIVE_OWN_MESSAGE
                        if (send(event_fd, message, numBytes, 0) == -1) {
                            perror("send error");
                        }
#endif

                        struct clientNode *current = connectedClients;
                        LOG_INFO("Broadcasting message from %s to other clients...", senderName);
                        char fullMessage[MAXDATASIZE + USERNAME_MAX_LENGTH + 4]; // Extra
                        snprintf(fullMessage, sizeof(fullMessage), "%s: %s", senderName, message);

                        while (current != NULL) {
                            if (current->fd != event_fd) {
                                if (send(current->fd, fullMessage, strlen(fullMessage), 0) == -1) {
                                    perror("send error");
                                    if (errno == EPIPE || errno == ECONNRESET) {
                                        // Client disconnected, remove from list
                                        LOG_DEBUG(
                                            "Client %d returned EPIPE or ECONNRESET. Removing...",
                                            current->fd);
                                        EV_SET(&change, current->fd, EVFILT_READ, EV_DELETE, 0, 0,
                                               NULL);
                                        if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
                                            if (errno == EINTR) {
                                                continue;
                                            }
                                            perror("kevent delete error");
                                        }

                                        connectedClients =
                                            deleteClient(connectedClients, current->fd);
#ifdef DEBUG
                                        printList(connectedClients);
#endif
                                        close(current->fd);
                                    }
                                }
                                LOG_DEBUG("Sent message '%s' to Client %d.", message, current->fd);
                            }
                            current = current->next;
                        }
                    }
                }
            }
        }
    }
    shutdownServer(connectedClients);
    logChatMessage("Server shut down.");
    closeChatLog();
    LOG_INFO("Server shut down.");
    return EXIT_SUCCESS;
}