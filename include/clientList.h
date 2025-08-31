#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

typedef struct clientNode clientNode;

clientNode *createClientNode(int fd);
clientNode *addClient(clientNode *head, int fd);
clientNode *deleteClient(clientNode *head, int fd);
void printList(clientNode *linked_list);

struct clientNode {
    int fd; // File descriptor
    char username[32];
    clientNode *next;
};

#endif