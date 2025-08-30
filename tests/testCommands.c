#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../include/commands.h"
#include "../include/clientList.h"

ssize_t mock_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags;
    printf("MOCKING SEND with fd %d and message: \"%.*s\"\n", sockfd, (int)len, (char *)buf);
    return len; //Return the length of the buffer for testing purposes
}

int main(){
    struct clientNode* head = NULL;
    head = addClient(head, 1);
    strncpy(head->username, "UserA", sizeof(head->username));
    head->username[sizeof(head->username) -1] = '\0';
    
    //Test help command
    processHelpCmd(1);

    //Test list command with one client
    processListCmd(1, head, 64);

    head = addClient(head, 2);
    strncpy(head->next->username, "UserB", sizeof(head->next->username));
    head->next->username[sizeof(head->next->username) -1] = '\0';

    //Test list command with more than one client
    processListCmd(1, head, 64);

    //Test valid name command
    processNameCmd(1, head, "NewUserA", 64);
    assert(strcmp(head->username, "NewUserA") == 0);

    //Test invalid name commands
    processNameCmd(1, head, "UserB", 64); //Taken
    processNameCmd(1, head, "", 64); //Invalid
    processNameCmd(1, head, "New UserA", 64); //Has space
    processNameCmd(1, head, "An extremely and unreasonably long username that will exceed the server default of sixty four character", 64);
    assert(strcmp(head->username, "NewUserA") == 0); //None of the invalids resulted in a change

    //Test message command
    char validMsg[] = "/msg UserB Private message test!";
    processMsgCmd(1, head, validMsg, 64);

    char selfMsg[] = "/msg NewUserA Private message test!";
    processMsgCmd(1, head, selfMsg, 64);

    char userNotFound[] = "/msg UserC Private message test!";
    processMsgCmd(1, head, userNotFound, 64);

    char noArguments[] = "/msg";
    processMsgCmd(1, head, noArguments, 64);

    char emptyMsg[] = "/msg UserB";
    processMsgCmd(1, head, emptyMsg, 64);

    //Cleanup
    head = deleteClient(head, 1);
    head = deleteClient(head, 2);
    assert(head == NULL);

    printf("Testing complete.\n");
    return 0;
}