/**
 * @file client.cpp
 * @brief Client program to connect to the chat server and handle user input/output.
*/

#include "../include/log.hpp"
#include "../include/socketRAII.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <csignal>
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

/// The maximum allowed message length.
constexpr int MAX_MSG_LENGTH = 512;

/// Indicates if the client should keep running.
atomic<bool> run(true);

/// Logger instance.
ChatLogger logger;

/// Client-side username variable for logging, updated with /name command.
string username = "Unknown";

string clientPrefix(const string &username, const string &msg) {
    return "(Client [" + username + "]) " + msg;
}

/**
 * @brief Thread function to handle user input and send messages to the server.
 * 
 * Continuously reads user input from stdin, sends messages to server,
 * and processes local commands. Exits when user types "exit" or when
 * the server disconnects.
 * 
 * @param sockFd Reference to the SocketRAII object representing the server socket.
 */
void inputLoop(SocketRAII &sockFd) {
    while (run) {
        if (!run) {
            break;
        }
        string message;
        if (!getline(cin, message)) {
            break;
        }
        if (!run) {
            break;
        }
        if (message.empty()) {
            continue;
        }
        if (message.length() > MAX_MSG_LENGTH) {
            LOG_ERROR(clientPrefix(username, "Message too long."), logger);
            continue;
        }
        size_t sent = send(sockFd.get(), message.c_str(), message.length(), 0);
        if (sent == static_cast<size_t>(-1)) {
            LOG_ERROR(clientPrefix(username, "Error sending message to server."), logger);
            run = false;
            break;
        }
#ifdef LOCAL_EXIT
        if (message == "exit") {
            run = false;
        }
#endif
        if (!run) {
            break;
        }
    }
}

/** 
 * @brief Thread function to receive messages from the server and display them.
 * 
 * Continuously listens for messages from server, displays them to stdout,
 * and handles server shutdown messages. Exits when server disconnects
 * or sends a shutdown instruction.
 * 
 * @param sockFd Socket file descriptor connected to the server.
 */
void recvLoop(int sockFd) {
    char buffer[1024];
    while (run) {
        // Receive message from server
        ssize_t bytesReceived = recv(sockFd, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string msg(buffer);

            string prefix = "Your name has been changed to";
            if (msg.rfind(prefix, 0) == 0) {
                username = msg.substr(prefix.length() + 1);
            }

            // Close socket if server sent shutdown instruction
            if (strcmp(buffer, "SERVER_SHUTDOWN") == 0) {
                LOG_INFO(clientPrefix(username, "Server has closed. Exiting..."), logger);
                run = false;
                break;
            } else {
                cout << buffer << endl;
#ifdef INSTRUCTIONS
                cout << "----------------------------------------" << endl;
                cout << "Write message for server, or '/help' for a list of commands:" << endl;
#endif
            }
        } else if (bytesReceived == 0) { // Client either disconnected or error occurred
            LOG_INFO(clientPrefix(username, "Server disconnected."), logger);
            run = false;
            break;
        } else if (bytesReceived == -1) {
            if (errno == ECONNRESET || errno == ENOTCONN) {
                LOG_ERROR(clientPrefix(username, "Connection closed by server."), logger);
                run = false;
                break;
            }
        }
    }
}

/**
 * @brief Main client startup function.
 * 
 * Initializes client, connects to server, and starts input/output threads
 * for user interaction, with graceful shutdown on disconnection or user
 * exit command.
 * 
 * @param argc Argument count.
 * @param argv Argument vector. Requires server IP/hostname and port number.
 * @return EXIT_SUCCESS on successful execution, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[]) {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    SocketRAII sockFd;

    if (!logger.open("server.log")) {
        cerr << "Failed to open log file. Exiting." << endl;
        return EXIT_FAILURE;
    }

    // Print usage if no IP provided
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <hostname> <port>" << endl;
        LOG_ERROR(clientPrefix("Incoming client connection",
                               "Usage: ./client <server ip/hostname> <port>"),
                  logger);
        return EXIT_FAILURE;
    }

    int intPort = atoi(argv[2]);
    if (intPort < 1024 || intPort > 65535) {
        LOG_ERROR(clientPrefix("Incoming client connection",
                               "Invalid port. Port must be between 1024 and 65535."),
                  logger);
        return EXIT_FAILURE;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        LOG_ERROR(
            clientPrefix("Incoming client connection", "getaddrinfo: " + string(gai_strerror(rv))),
            logger);
        return EXIT_FAILURE;
    }

    // Loop through results and bind to first available address
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        // Create socket
        sockFd = SocketRAII(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (sockFd.get() == -1) {
            LOG_ERROR(clientPrefix("Incoming client connection",
                                   "Error creating socket: " + string(strerror(errno))),
                      logger);
            continue;
        }

        inet_ntop(p->ai_family,
                  (p->ai_family == AF_INET) ? (void *)&((sockaddr_in *)p->ai_addr)->sin_addr
                                            : (void *)&((sockaddr_in6 *)p->ai_addr)->sin6_addr,
                  s, sizeof s);
        LOG_DEBUG(
            clientPrefix("Incoming client connection", "Attempting connection to " + string(s)),
            logger);

        // Connect to server
        if (connect(sockFd.get(), p->ai_addr, p->ai_addrlen) == -1) {
            LOG_ERROR(clientPrefix("Incoming client connection",
                                   "Error connecting to server: " + string(strerror(errno))),
                      logger);
            continue;
        }
        break;
    }

    if (p == nullptr) {
        LOG_ERROR(clientPrefix("Incoming client connection", "Failed to connect."), logger);
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

    thread recvThread(recvLoop, sockFd.get());
    thread inputThread(inputLoop, ref(sockFd));

    while (run) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    inputThread.join();
    recvThread.join();
    cout << "Client exiting." << endl;
    return EXIT_SUCCESS;
}