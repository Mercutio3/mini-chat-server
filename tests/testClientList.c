#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/clientList.h"

int main(void){
    struct clientNode* head = NULL;

    //Test adding clients, checking list integrity after each addition
    head = addClient(head, 3);
    assert(head != NULL && head->fd == 3 && head->next == NULL);
    head = addClient(head, 5);
    assert(head != NULL && head->fd == 3 && head->next != NULL && head->next->fd == 5 && head->next->next == NULL);
    head = addClient(head, 7);
    assert(head != NULL && head->fd == 3 && head->next != NULL && head->next->fd == 5 && head->next->next != NULL && head->next->next->fd == 7 && head->next->next->next == NULL);

    //Test deleting clients, checking list integrity after each deletion
    head = deleteClient(head, 10); //Delete nonexistent node
    assert(head != NULL && head->fd == 3 && head->next != NULL && head->next->fd == 5 && head->next->next != NULL && head->next->next->fd == 7 && head->next->next->next == NULL);
    head = deleteClient(head, 5); //Delete middle node
    assert(head != NULL && head->fd == 3 && head->next != NULL && head->next->fd == 7 && head->next->next == NULL);
    head = deleteClient(head, 3); //Delete head
    assert(head != NULL && head->fd == 7 && head->next == NULL);
    head = deleteClient(head, 7); //Delete last node
    assert(head == NULL);

    //Test deleting from empty list
    head = deleteClient(head, 10);
    assert(head == NULL);
    
    //Test adding after deletions (users join after others all leave)
    head = addClient(head, 10);
    assert(head != NULL && head->fd == 10 && head->next == NULL);
    head = addClient(head, 20);
    assert(head != NULL && head->fd == 10 && head->next != NULL && head->next->fd == 20 && head->next->next == NULL);

    //Test getUserNameFromFD (necessary for some server functionality)
    strncpy(head->username, "User1", sizeof(head->username));
    head->username[sizeof(head->username) -1] = '\0';
    strncpy(head->next->username, "User2", sizeof(head->next->username));
    head->next->username[sizeof(head->next->username) -1] = '\0';
    char* name = getUserNameFromFD(head, 10);
    assert(name != NULL && strcmp(name, "User1") == 0);
    name = getUserNameFromFD(head, 20);
    assert(name != NULL && strcmp(name, "User2") == 0);
    name = getUserNameFromFD(head, 30);
    assert(name == NULL);

    //Adding and deleting a large number of clients to check for memory issues
    for(int i = 21; i <= 120; i++){
        head = addClient(head, i);
    }
    int count = 0;
    struct clientNode* current = head;
    while(current != NULL){
        count++;
        current = current->next;
    }
    assert(count == 102);
    for(int i = 21; i <= 120; i++){
        head = deleteClient(head, i);
    }
    count = 0;
    current = head;
    while(current != NULL){
        count++;
        current = current->next;
    }
    assert(count == 2);

    //Cleanup
    head = deleteClient(head, 10);
    head = deleteClient(head, 20);
    assert(head == NULL);

    printf("Testing complete.\n");
    return 0;
}