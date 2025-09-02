#include "../include/clientList.hpp"
#include "../include/commands.hpp"
#include "../include/log.hpp"
#include "../include/utils.hpp"
#include <arpa/inet.h>
#include <atomic>
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

#define BACKLOG 5
#define MAXDATASIZE 100
#define USERNAME_MAX_LENGTH 64

atomic<bool> run(true);
ThreadClientList clientList;
ChatLogger logger;

void signalHandler(int signum) {
    LOG_DEBUG("Signal " + to_string(signum) + " received!", logger);
    run = false;
}

void handleClient(int clientFd) {
    struct timeval tv;
    tv.tv_sec = 1; // 1 second timeout
    tv.tv_usec = 0;
    setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    while (run) {
        vector<char> buffer(MAXDATASIZE);

        if (!isUTF8(buffer.data())) {
            LOG_ERROR("Received invalid UTF-8 data from " + clientList.getUsernameFromFd(clientFd) +
                          ". Disconnecting client.",
                      logger);
            close(clientFd);
            clientList.deleteClient(clientFd);
            return;
        }

        ssize_t bytesReceived = recv(clientFd, static_cast<char *>(buffer.data()), MAXDATASIZE, 0);
        if (bytesReceived > 0) {
            string msg(buffer.data(), bytesReceived);
            if (!msg.empty() && msg[0] == '/') {
                if (msg == "/exit") {
                    send(clientFd, "SERVER_SHUTDOWN", 16, 0);
                    break;
                } else if (msg == "/help") {
                    processHelpCmd(clientFd);
                    LOG_INFO("Sent list of commands to " + clientList.getUsernameFromFd(clientFd),
                             logger);
                } else if (msg == "/list") {
                    processListCmd(clientFd, clientList);
                    LOG_INFO("Sent list of connected users to " +
                                 clientList.getUsernameFromFd(clientFd),
                             logger);
                } else if (msg.substr(0, 6) == "/name ") {
                    processNameCmd(clientFd, clientList, msg.substr(6), USERNAME_MAX_LENGTH);
                } else if (msg.substr(0, 5) == "/msg ") {
                    processMsgCmd(clientFd, clientList, msg.substr(5));
                }
            } else {
                string fullMsg = clientList.getUsernameFromFd(clientFd) + ": " + msg;
                LOG_INFO("Broadcasting message from " + clientList.getUsernameFromFd(clientFd) +
                             " to other clients...",
                         logger);
#ifdef RECEIVE_OWN_MESSAGE
                clientList.broadcastMessage(fullMsg, -1);
#else

                clientList.broadcastMessage(fullMsg, clientFd);
#endif
            }
        } else if (bytesReceived == 0) {
            LOG_DEBUG("Client " + clientList.getUsernameFromFd(clientFd) + " disconnected.",
                      logger);
            break; // Client disconnected
        } else if (bytesReceived == -1) {
            if (!run && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break; // Server is shutting down, exit thread
            }
            // Otherwise, keep looping (timeout)
        }
    }
    LOG_INFO("Client " + clientList.getUsernameFromFd(clientFd) + " has disconnected.", logger);
    close(clientFd);
    clientList.deleteClient(clientFd);
}

int main(int argc, char *argv[]) {
    string port = "5223"; // Default port
    logger.open("server.log");

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

    int serverFd;
    int yes = 1;
    for (p = servInfo; p != nullptr; p = p->ai_next) {
        serverFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (serverFd == -1) {
            cerr << "Error creating socket" << endl;
            continue;
        }
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            cerr << "Error setting SO_REUSEADDR" << endl;
            close(serverFd);
            continue;
        }

        if (::bind(serverFd, p->ai_addr, p->ai_addrlen) == -1) {
            cerr << "Error binding socket" << endl;
            close(serverFd);
            continue;
        }
        break;
    }
    freeaddrinfo(servInfo);

    if (p == nullptr) {
        LOG_ERROR("Failed to bind.", logger);
        return EXIT_FAILURE;
    }

    if (listen(serverFd, BACKLOG) == -1) {
        LOG_ERROR("Error listening on socket", logger);
        close(serverFd);
        return EXIT_FAILURE;
    }

    int flags = fcntl(serverFd, F_GETFL, 0);
    fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);

    LOG_INFO("Server started and listening on port " + port, logger);
    cout << "Press Ctrl + C to stop the server" << endl;

    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Main accept loop
    vector<thread> client_threads;
    while (run) {
        int clientFd = accept(serverFd, nullptr, nullptr);
        if (!run) {
            break;
        }
        if (clientFd == -1) {
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
            cerr << "Error accepting client connection" << endl;
            continue;
        }
        clientList.addClient(clientFd, "Client" + to_string(clientFd));
        thread t([clientFd]() { handleClient(clientFd); });
        t.detach();
    }
    const string shutdownMsg = "SERVER_SHUTDOWN";
    LOG_DEBUG("Termination signal received. Shutting down...", logger);
    vector<int> fds = clientList.getAllFds();
    for (int fd : fds) {
        send(fd, shutdownMsg.c_str(), shutdownMsg.size(), 0);
        LOG_DEBUG("Closing client with fd " + to_string(fd), logger);
        close(fd);
    }
    close(serverFd);
    LOG_INFO("Server shut down.", logger);
    return EXIT_SUCCESS;
}