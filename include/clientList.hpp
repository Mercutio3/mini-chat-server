/**
 * @file clientList.hpp
 * @brief Thread-safe list of connected clients.
 */

#ifndef CLIENTLIST_H
#define CLIENTLIST_H

#include <mutex>
#include <string>
#include <vector>

using namespace std;

/**
 * @brief Stores information for a connected client.
 */
struct ClientInfo {
    int fd;
    string username;
};

/**
 * @brief Thread-safe list of connected clients.
 *
 * Provides methods to add, remove, and query clients.
 */
class ThreadClientList {
  private:
    vector<ClientInfo> clients;
    mutex mtx;

  public:
    /**
     * @brief Add client to the list.
     * @param fd File descriptor of the client.
     * @param username Username of the client.
     */
    void addClient(int fd, const string &username);

    /**
     * @brief Remove client from the list by file descriptor.
     * @param fd File descriptor of the client to remove.
     */
    void deleteClient(int fd);

    /**
     * @brief Print the list of connected clients.
     */
    void printList();

    /**
     * @brief Get a list of all connected client file descriptors.
     * @return Vector of client file descriptors.
     */
    vector<int> getAllFds();

    /**
     * @brief Get a list of all connected usernames.
     * @return Vector of usernames.
     */
    vector<string> getUsernames();

    /**
     * @brief Get the username associated with a file descriptor.
     * @param fd File descriptor of the client.
     * @return Username of the client, or empty string if not found.
     */
    string getUsernameFromFd(int fd);

    /**
     * @brief Get the file descriptor associated with a username.
     * @param username Username of the client.
     * @return File descriptor of the client, or -1 if not found.
     */
    int getFdFromUsername(const string &username);

    /**
     * @brief Check if a username is already taken.
     * @param username Username to check.
     * @return True if the username is taken, false otherwise.
     */
    bool isUserTaken(const string &username);

    /**
     * @brief Change a client's username.
     *
     * Attempts to change a client's username. Validates that the new name
     * isn't empty, doesn't exceed maximum length, and isn't taken. Sends
     * success or error messages back to client accordingly.
     *
     * @param fd File descriptor of the client.
     * @param newName New username for the client.
     * @param maxLength Maximum length for the username.
     */
    void changeUsername(int fd, const string &newName, int maxLength);

    /**
     * @brief Broadcast a message to all clients except the sender.
     * @param message Message to send.
     * @param excludeFd File descriptor of the client to exclude.
     */
    void broadcastMessage(const string &message, int excludeFd);
};

#endif