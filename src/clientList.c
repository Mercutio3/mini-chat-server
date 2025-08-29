#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/clientList.h"

//#define DEBUG

struct clientNode* createClientNode(int fd){
    struct clientNode* newClient = (struct clientNode*)malloc(sizeof(struct clientNode));
    if(newClient == NULL){
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }
    newClient->fd = fd;
    snprintf(newClient->username, sizeof(newClient->username), "Client%d", fd);
    printf("Client with username %s created.\n", newClient->username);
    newClient->next = NULL;
    return newClient;
}

struct clientNode* addClient(struct clientNode* head, int fd){
    #ifdef DEBUG
        printf("Adding client with fd %d to linked list...\n", fd);
    #endif
    struct clientNode* newClient = createClientNode(fd);
    
    //Adding first element
    if(head == NULL){
        #ifdef DEBUG
            printf("Head is NULL, returning new client as head.\n");
        #endif
        return newClient;
    }
    //Adding subsequent elements
    struct clientNode* current = head;
    while(current->next != NULL){
        current = current->next;
    }
    current->next = newClient;
    #ifdef DEBUG
        printf("Added client with fd %d to linked list.\n", fd);
    #endif
    return head;
}

struct clientNode* deleteClient(struct clientNode* head, int fd){
    #ifdef DEBUG
        printf("Deleting client with fd %d from linked list...\n", fd);
    #endif
    if(head == NULL){
        return head;
    }

    //Delete head node
    if(head->fd == fd){
        struct clientNode* temp = head;
        head = head->next;
        free(temp);
        #ifdef DEBUG
            printf("Deleted client with fd %d from linked list.\n", fd);
        #endif
        return head;
    }
    
    //Delete a node that is not the head
    struct clientNode* current = head;
    while(current->next != NULL && current->next->fd != fd){
        current = current->next;
    }
    if(current->next != NULL){
        struct clientNode* temp = current->next;
        current->next = current->next->next;
        free(temp);
    }
    #ifdef DEBUG
        printf("Deleted client with fd %d from linked list.\n", fd);
    #endif
    return head;
};

//Traverse linked list and print all file descriptors
void printList(struct clientNode* linked_list){
    printf("Printing list of clients...\n");
    while(linked_list != NULL){
        printf("%d ", linked_list->fd);
        linked_list = linked_list->next;
    }
    printf("\n");
}

char* getUserNameFromFD(struct clientNode* head, int fd){
    struct clientNode* current = head;
    while(current != NULL){
        if(current->fd == fd){
            return current->username;
        }
        current = current->next;
    }
    return NULL; //Not found
}