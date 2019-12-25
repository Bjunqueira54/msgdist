#include "serverHeader.h"

int server_file;

void initializeVerifier(int *parent_to_child, int *child_to_parent) {
    pipe(parent_to_child); // Server-to-Child pipe (Server write)
    pipe(child_to_parent); // Child-to-Server (Server Read)

    pid_t child_pid = fork();
    
    if(child_pid == 0) { // Child
        /* Fechar estremidades nao usadas */
        close(child_to_parent[0]);
        close(parent_to_child[1]);
        
        close(STDIN_FILENO); // 0
        dup(parent_to_child[0]);
        close(parent_to_child[0]);
        
        close(STDOUT_FILENO); // 1
        dup(child_to_parent[1]);
        close(child_to_parent[1]);
        
        if(execlp("./verificador", "verificador", BADWORDS, (char *) NULL) == -1)
            fprintf(stderr, "Error starting the verifier!\n");
    }
    else { // Parent
        close(child_to_parent[1]);
        close(parent_to_child[0]);
    }
}

char** stringParser(const char* string) {
    if(string == NULL)
        return NULL;
    
    int spaces = 0;
    int maxWordSize = 0;
    int currentWordSize = 0;
    
    for(int i = 0; true; i++) {
        if(string[i] == ' ') {
            spaces++;
            
            if(currentWordSize >= maxWordSize) {
                maxWordSize = currentWordSize;
                currentWordSize = 0;
            }
            continue;
        }
        else if(string[i] == '\0') {
            if(currentWordSize >= maxWordSize) {
                maxWordSize = currentWordSize;
                currentWordSize = 0;
            }
            
            break;
        }  
        currentWordSize++;
    }
    
    int arraySize = spaces + 2; //words = spaces + 1 (+ 1 for NULL)
    
    char** parsedStrings = calloc(arraySize, sizeof(char*));
    
    if(parsedStrings == NULL)
        return NULL;
    
    for(int i = 0; i < arraySize; i++) {
        parsedStrings[i] = calloc(maxWordSize, sizeof(char));
        
        if(parsedStrings[i] == NULL) {
            printf("Allocation Error!\n");
            
            for(int j = 0; j < i; j++)
                free(parsedStrings[j]);
            
            return NULL;
        }
    }
    
    if(arraySize == 2) { //Only 1 word + NULL 
        for(int i = 0; string[i] != '\0'; i++)
        {
            parsedStrings[0][i] = string[i];
        }
        
        parsedStrings[1] = NULL;
    }
    else if(arraySize < 2) { //something went HORRIBLY WRONG!
        return NULL;
    }
    else {
        int i = 0;
        
        for(int y = 0; true; y++) {
            for(int x = 0; true; x++) {
                if(string[i] == ' ' || string[i] == '\0') {
                    parsedStrings[y][x] = '\0';
                    break;
                }
                else {
                    parsedStrings[y][x] = string[i];
                }
                i++;
            }

            if(string[i] == '\0')
                break;
            i++;
        }        
    }
    
    parsedStrings[arraySize - 1] = NULL;
    
    return parsedStrings;
}

void addNewMessage(pText first, pText newMsg) {
    if(first == NULL) {
        newMsg->next = newMsg->prev = NULL;
        first = newMsg;
        return;
    }
    
    pText aux = first;
    
    if(aux->next == NULL)
        aux->next = newMsg;
    else {
        while(aux->next != NULL)
            aux = aux->next;
        
        aux->next = newMsg;
    }
}

void addNewTopic(pTopic first, pTopic newTopic) {
    if(first == NULL) {
        newTopic->next = newTopic->prev = NULL;
        first = newTopic;
        return;
    }
    
    pText aux = first;
    
    if(aux->next == NULL)
        aux->next = newTopic;
    else {
        while(aux->next != NULL)
            aux = aux->next;
        
        aux->next = newTopic;
    }
}

void listAllUsers(pClient clientList) {
    if(clientList == NULL)
        return;
    
    printf("Currently Connected Users:\n");
    
    pClient aux = clientList;
    
    do {
        printf("%d\n", aux->c_PID);
        aux = aux->next;
    } while(aux != NULL);
}

void listAllMesages()
{
    printf("All Messages on the server: \n");
}

void listAllTopics()
{
    printf("Current Topics:\n");
}

void deleteEmptyTopics() 
{
    printf("Deleting all empty topics from the server.\n");
}

void killAllClients(pClient clientList)
{
    union sigval value;
    value.sival_int = 0;
    
    pClient aux = clientList;

    while(aux != NULL)
    {
        sigqueue(aux->c_PID, SIGINT, value);
        aux = aux->next;
    }
}

int createServerFiles()
{
    struct stat tmpstat = {0};
    
    if(stat(MSGDIST_DIR, &tmpstat) == -1)
    {
        if(mkdir(MSGDIST_DIR, 0744) == -1)
        {
            printf("Directory Creation: %d\n", errno);
            return -1;
        }
    }
    
    server_file = open(SERVER_PID, O_RDWR);
    
    if(server_file != -1)
    {
        fprintf(stderr, "Server already running\n");
        exit (EXIT_FAILURE);
    }
    
    server_file = open(SERVER_PID, O_RDWR | O_CREAT, 0644);
    
    if(server_file == -1)
    {
        printf("File Creation: %d\n", errno);
        return -1;
    }
    
    pid_t self_pid = getpid();
    
    char pid[6];
    
    snprintf(pid, sizeof(char)*6, "%d", self_pid);
    
    write(server_file, pid, strlen(pid));

    return 0;
}

int deleteServerFiles() 
{    
    if (remove(SERVER_PID) != 0)
    {
        fprintf(stderr, "Erro ao apagar o ficheiro\n"); 
        return -1;
    }
    
    system("rmdir /tmp/msgdist");
    
    return 0; 
}