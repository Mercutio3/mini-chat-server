struct clientNode* createClientNode(int fd);
struct clientNode* addClient(struct clientNode* head, int fd);
struct clientNode* deleteClient(struct clientNode* head, int fd);
void printList(struct clientNode* linked_list);
char* getUserNameFromFD(struct clientNode* head, int fd);

typedef struct clientNode{
    int fd; //File descriptor
    char username[32]; //Username
    struct clientNode *next;
} clientNode;