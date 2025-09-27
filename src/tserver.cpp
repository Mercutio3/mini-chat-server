/**
 * @file tserver.cpp
 * @brief Server program and its server-side operations.
 */

#include "../include/clientList.hpp"
#include "../include/commands.hpp"
#include "../include/log.hpp"
#include "../include/socketRAII.hpp"
#include "../include/utils.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

/// Maximum number of pending connections in queue
constexpr int BACKLOG = 5;

/// Maximum number of bytes for a single message
constexpr int MAXDATASIZE = 100;

/// Maximum length for a username
constexpr int USERNAME_MAX_LENGTH = 64;

/// Indicates if the server should keep running.
atomic<bool> run(true);

/// Client list instance
ThreadClientList clientList;

/// Logger instance
ChatLogger logger;

/// Vector to hold client threads
vector<thread> clientThreads;

/**
 * @brief Signal handler to gracefully shut down the server.
 *
 * Sets `run` atomic boolean to false when SIGINT or SIGTERM is received;
 * server exits main loop and cleans up resources before shutting down.
 *
 * @param signum The signal number received.
 */
void signalHandler(int signum) {
    LOG_DEBUG("Signal " + to_string(signum) + " received!", logger);
    run = false;
}

/**
 * @brief Handles communication with a connected client.
 *
 * Manages interaction with a connected client, including receiving messages,
 * processing commands, and broadcasting messages. Also handles client
 * disconnection with proper cleanup.
 *
 * @param clientFd A SocketRAII object representing the client's socket file descriptor.
 */
void handleClient(SocketRAII clientFd) {
    struct timeval tv;
    tv.tv_sec = 1; // 1 second timeout
    tv.tv_usec = 0;
    if (setsockopt(clientFd.get(), SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1) {
        LOG_ERROR("Error setting socket receive timeout: " + string(strerror(errno)), logger);
        close(clientFd.get());
        clientList.deleteClient(clientFd.get());
        return;
    }
    while (run) {
        vector<char> buffer(MAXDATASIZE);

        ssize_t bytesReceived = recv(clientFd, static_cast<char *>(buffer.data()), MAXDATASIZE, 0);
        if (bytesReceived > 0) {
            string msg(buffer.data(), bytesReceived);
            if (msg.length() > MAXDATASIZE) {
                LOG_ERROR("Message too long from " + clientList.getUsernameFromFd(clientFd),
                          logger);
                continue;
            }
            if (!isUTF8(msg)) {
                LOG_ERROR("Received invalid UTF-8 data from " +
                              clientList.getUsernameFromFd(clientFd) + ". Disconnecting client.",
                          logger);
                close(clientFd.get());
                clientList.deleteClient(clientFd.get());
                return;
            }
            if (!msg.empty() && msg[0] == '/') {
                if (msg == "/exit") {
                    send(clientFd, "SERVER_SHUTDOWN", 16, 0);
                    break;
                } else if (msg == "/help") {
                    processHelpCmd(clientFd);
                    LOG_INFO("Sent list of commands to " +
                                 clientList.getUsernameFromFd(clientFd.get()),
                             logger);
                } else if (msg == "/list") {
                    processListCmd(clientFd, clientList);
                    LOG_INFO("Sent list of connected users to " +
                                 clientList.getUsernameFromFd(clientFd.get()),
                             logger);
                } else if (msg.substr(0, 6) == "/name ") {
                    processNameCmd(clientFd, clientList, msg.substr(6), USERNAME_MAX_LENGTH);
                } else if (msg.substr(0, 5) == "/msg ") {
                    processMsgCmd(clientFd, clientList, msg.substr(5));
                }
            } else {
                string fullMsg = clientList.getUsernameFromFd(clientFd.get()) + ": " + msg;
                LOG_INFO("Broadcasting message from " +
                             clientList.getUsernameFromFd(clientFd.get()) +
                             " to other clients: " + msg,
                         logger);
#ifdef RECEIVE_OWN_MESSAGE
                clientList.broadcastMessage(fullMsg, -1);
#else

                clientList.broadcastMessage(fullMsg, clientFd);
#endif
            }
        } else if (bytesReceived == 0) {
            LOG_DEBUG("Client " + clientList.getUsernameFromFd(clientFd.get()) + " disconnected.",
                      logger);
            break; // Client disconnected
        } else if (bytesReceived == -1) {
            if (!run && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break; // Server is shutting down, exit thread
            }
            // Otherwise, keep looping (timeout)
        }
    }
    LOG_INFO("Client " + clientList.getUsernameFromFd(clientFd.get()) + " has disconnected.",
             logger);
    close(clientFd.get());
    clientList.deleteClient(clientFd.get());
}

/**
 * @brief Main server startup function.
 *
 * Initializes server, sets up listening socket, and enters main accept loop
 * to handle incoming client connections. Also manages graceful shutdown on
 * receiving termination signals.
 *
 * @param argc Argument count.
 * @param argv Argument vector. Optionally takes a port number as the first argument.
 * @return EXIT_SUCCESS on successful execution, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[]) {
    string port = "5223"; // Default port
    if (!logger.open("server.log")) {
        cerr << "Failed to open log file. Exiting." << endl;
        return EXIT_FAILURE;
    }

    // Print usage if no port is provided
    if (argc > 2) {
        LOG_ERROR("Usage: ./server [port]", logger);
        return EXIT_FAILURE;
    } else if (argc == 2) {
        port = argv[1];
    }
    int intPort = stoi(port);
    if (intPort < 1024 || intPort > 65535) {
        LOG_ERROR("Invalid port. Port must be between 1024 and 65535.", logger);
        return EXIT_FAILURE;
    }

    struct addrinfo hints, *servInfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use local IP

    if ((rv = getaddrinfo(nullptr, port.c_str(), &hints, &servInfo)) != 0) {
        LOG_ERROR("Getaddrinfo: " + string(gai_strerror(rv)), logger);
        return EXIT_FAILURE;
    }

    SocketRAII serverFd;
    int yes = 1;
    for (p = servInfo; p != nullptr; p = p->ai_next) {
        serverFd = SocketRAII(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (serverFd.get() == -1) {
            LOG_ERROR("Error creating socket: " + string(strerror(errno)), logger);
            continue;
        }
        if (setsockopt(serverFd.get(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            LOG_ERROR("Error setting SO_REUSEADDR: " + string(strerror(errno)), logger);
            close(serverFd.get());
            continue;
        }

        if (::bind(serverFd.get(), p->ai_addr, p->ai_addrlen) == -1) {
            LOG_ERROR("Error binding socket: " + string(strerror(errno)), logger);
            close(serverFd.get());
            continue;
        }
        break;
    }
    freeaddrinfo(servInfo);

    if (p == nullptr) {
        LOG_ERROR("Failed to bind.", logger);
        return EXIT_FAILURE;
    }

    if (listen(serverFd.get(), BACKLOG) == -1) {
        LOG_ERROR("Error listening on socket", logger);
        close(serverFd.get());
        return EXIT_FAILURE;
    }

    int flags = fcntl(serverFd.get(), F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("Error getting socket flags: " + string(strerror(errno)), logger);
        close(serverFd.get());
        return EXIT_FAILURE;
    }
    if (fcntl(serverFd.get(), F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_ERROR("Error setting socket to non-blocking: " + string(strerror(errno)), logger);
        close(serverFd.get());
        return EXIT_FAILURE;
    }

    LOG_INFO("Server started and listening on port " + port, logger);
    cout << "Press Ctrl + C to stop the server" << endl;

    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Main accept loop
    vector<thread> client_threads;
    while (run) {
        SocketRAII clientFd = SocketRAII(accept(serverFd.get(), nullptr, nullptr));
        if (!run) {
            break;
        }
        if (clientFd.get() == -1) {
            if (errno == EINTR) {
                if (!run) {
                    break;
                }
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No pending connections, sleep briefly to avoid busy loop
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }
            LOG_ERROR("Error accepting client connection: " + string(strerror(errno)), logger);
            continue;
        }
        clientList.addClient(clientFd.get(), "Client" + to_string(clientFd.get()));
        LOG_INFO("New client connected with fd " + to_string(clientFd.get()), logger);
        clientThreads.emplace_back(
            [clientFd = std::move(clientFd)]() mutable { handleClient(std::move(clientFd)); });
    }
    // Join client threads before shutting down
    for (auto &t : clientThreads) {
        if (t.joinable())
            t.join();
    }
    const string shutdownMsg = "SERVER_SHUTDOWN";
    LOG_DEBUG("Termination signal received. Shutting down...", logger);
    vector<int> fds = clientList.getAllFds();
    for (int fd : fds) {
        send(fd, shutdownMsg.c_str(), shutdownMsg.size(), 0);
        LOG_DEBUG("Closing client with fd " + to_string(fd), logger);
        close(fd);
    }
    close(serverFd.get());
    LOG_INFO("Server shut down.", logger);
    return EXIT_SUCCESS;
}