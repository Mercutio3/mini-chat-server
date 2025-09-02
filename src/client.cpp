/*
client.cpp - Client program and its client-side operations
*/

#include "../include/log.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

using namespace std;

atomic<bool> run(true);
ChatLogger logger;

void recvLoop(int sockFd) {
    char buffer[1024];
    while (run) {
        // Receive message from server
        ssize_t bytesReceived = recv(sockFd, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';

            // Close socket if server sent shutdown instruction
            if (strcmp(buffer, "SERVER_SHUTDOWN") == 0) {
                LOG_INFO("Server has closed. Exiting...", logger);
                break;
            } else {
                cout << buffer << endl;
#ifdef INSTRUCTIONS
                cout << "----------------------------------------" << endl;
                cout << "Write message for server, or '/help' for a list of commands:" << endl;
#endif
            }
        } else if (bytesReceived == 0) { // Client either disconnected or error occurred
            LOG_INFO("Server disconnected.", logger);
            run = false;
            break;
        } else if (bytesReceived == -1) {
            if (errno == ECONNRESET || errno == ENOTCONN) {
                LOG_ERROR("Connection closed by server.", logger);
                run = false;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int sockFd;

    // logger.open("chat.log");

    // Print usage if no IP provided
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <hostname> <port>" << endl;
        LOG_ERROR("Usage: ./client <server ip/hostname> <port>", logger);
        return EXIT_FAILURE;
    }

    int intPort = atoi(argv[2]);
    if (intPort < 1024 || intPort > 65535) {
        LOG_ERROR("Invalid port. Port must be between 1024 and 65535.", logger);
        return EXIT_FAILURE;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return EXIT_FAILURE;
    }

    // Loop through results and bind to first available address
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        // Create socket
        sockFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockFd == -1) {
            cerr << "Error creating socket" << endl;
            continue;
        }

        inet_ntop(p->ai_family,
                  (p->ai_family == AF_INET) ? (void *)&((sockaddr_in *)p->ai_addr)->sin_addr
                                            : (void *)&((sockaddr_in6 *)p->ai_addr)->sin6_addr,
                  s, sizeof s);
        LOG_DEBUG("Attempting connection to " + string(s), logger);

        // Connect to server
        if (connect(sockFd, p->ai_addr, p->ai_addrlen) == -1) {
            cerr << "Error connecting to server" << endl;
            close(sockFd);
            continue;
        }
        break;
    }

    if (p == nullptr) {
        LOG_ERROR("Failed to connect.", logger);
        freeaddrinfo(servinfo);
        return EXIT_FAILURE;
    }

    inet_ntop(p->ai_family,
              (p->ai_family == AF_INET) ? (void *)&((sockaddr_in *)p->ai_addr)->sin_addr
                                        : (void *)&((sockaddr_in6 *)p->ai_addr)->sin6_addr,
              s, sizeof s);
    LOG_INFO("Connected to " + string(s) + " on port " + string(argv[2]), logger);

    freeaddrinfo(servinfo); // Servinfo not used after here

    cout << "----------------------------------------" << endl;
    cout << "Write message for server, or '/help' for a list of commands:" << endl;

    thread recvThread(recvLoop, sockFd);

    while (run) {
        string message;
        getline(cin, message);
        if (!run) {
            break;
        }
        send(sockFd, message.c_str(), message.length(), 0);
#ifdef LOCAL_EXIT
        if (message == "exit") {
            run = false;
        }
#endif
        if (!run) {
            break;
        }
    }
    recvThread.join();
    close(sockFd);
    // logger.close();
    return EXIT_SUCCESS;
}