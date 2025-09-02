/*
clientList.h - ThreadClientList class definition
*/

#ifndef CLIENTLIST_H
#define CLIENTLIST_H

#include <mutex>
#include <string>
#include <vector>

using namespace std;

struct ClientInfo {
    int fd;
    string username;
};

class ThreadClientList {
  private:
    vector<ClientInfo> clients;
    mutex mtx;

  public:
    void addClient(int fd, const string &username);
    void deleteClient(int fd);
    void printList();
    vector<int> getAllFds();
    vector<string> getUsernames();
    string getUsernameFromFd(int fd);
    int getFdFromUsername(const string &username);
    bool isUserTaken(const string &username);
    void changeUsername(int fd, const string &newName, int maxLength);
    void broadcastMessage(const string &message, int excludeFd);
};

#endif