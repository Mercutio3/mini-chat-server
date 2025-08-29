#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../include/commands.h"
#include "../include/clientList.h"

int main(){
    struct clientnode* head = NULL;
    head = addClient(head, 1);
    head = addClient(head, 2);
    
    //Test help command
    processHelpCmd(1);

    //Test list command
    processListCmd(1, head, 64); //With few clients
    for(int i = 2; i <= 10; i++){
        head = addClient(head, i);
    }
    processListCmd(1, head, 64); //With many clients
    for(int i = 2; i <= 10; i++){
        head = deleteClient(head, i);
    }

    //Test name command
    processNameCmd(1, head, "UserA", 64);
    processNameCmd(2, head, "UserB", 64);
    processNameCmd(1, head, "UserB", 64); //Taken name
    processNameCmd(1, head, "", 64); //Empty name
    processNameCmd(1, head, "User A", 64); //Name with space
    processNameCmd(1, head, "Insanely long username that exceeds the default maximum length allowed by the server", 64);

    //Test msg command
    processMsgCmd(1, head, "/msg UserB testing private msg", 64); //Valid
    processMsgCmd(1, head, "/msg UserB ", 64); //Empty message
    processMsgCmd(1, head, "/msg UserC testing private msg", 64); //Nonexistent username
    processMsgCmd(1, head, "/msg", 64); //No arguments

    //Cleanup
    head = deleteClient(head, 1);
    head = deleteClient(head, 2);
    assert(head == NULL);

    printf("Testing complete.\n");
    return 0;
}