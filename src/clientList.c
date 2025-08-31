/*
clientList.c - Client linked list management functions
*/

#include "../include/clientList.h"
#include "../include/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clientNode *createClientNode(int fd) {
    struct clientNode *newClient = (struct clientNode *)malloc(sizeof(struct clientNode));
    if (newClient == NULL) {
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }
    newClient->fd = fd;
    snprintf(newClient->username, sizeof(newClient->username), "Client%d", fd);
    LOG_INFO("Client with username %s created.", newClient->username);
    newClient->next = NULL;
    return newClient;
}

struct clientNode *addClient(struct clientNode *head, int fd) {
    LOG_DEBUG("Adding client with fd %d to linked list...", fd);
    struct clientNode *newClient = createClientNode(fd);

    // Adding first element
    if (head == NULL) {
        LOG_DEBUG("Head is NULL, returning new client as head.");
        return newClient;
    }
    // Adding subsequent elements
    struct clientNode *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newClient;
    LOG_DEBUG("Added client with fd %d to linked list.", fd);
    return head;
}

struct clientNode *deleteClient(struct clientNode *head, int fd) {
    LOG_DEBUG("Deleting client with fd %d from linked list...", fd);
    if (head == NULL) {
        return head;
    }

    // Delete head node
    if (head->fd == fd) {
        struct clientNode *temp = head;
        head = head->next;
        free(temp);
        LOG_DEBUG("Deleted client with fd %d from linked list.", fd);
        return head;
    }

    // Delete a node that is not the head
    struct clientNode *current = head;
    while (current->next != NULL && current->next->fd != fd) {
        current = current->next;
    }
    if (current->next != NULL) {
        struct clientNode *temp = current->next;
        current->next = current->next->next;
        free(temp);
    }
    LOG_DEBUG("Deleted client with fd %d from linked list.", fd);
    return head;
};

// Traverse linked list and print all file descriptors
void printList(struct clientNode *linked_list) {
    LOG_INFO("Printing list of clients...");
    while (linked_list != NULL) {
        printf("%d ", linked_list->fd);
        linked_list = linked_list->next;
    }
    printf("\n");
}