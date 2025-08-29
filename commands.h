void processHelpCmd(int client_fd);
void processListCmd(int client_fd, struct clientNode* head, int maxLength);
void processNameCmd(int client_fd, struct clientNode* head, const char* newName, int maxLength);
void processMsgCmd(int client_fd, struct clientNode* head, char* message, int maxLength);