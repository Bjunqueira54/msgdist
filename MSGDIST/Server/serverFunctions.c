#include "serverHeader.h"

int server_file;

void initializeVerifier(int *parent_to_child, int *child_to_parent, char* badwords)
{
    pipe(parent_to_child); //Server-to-Child pipe (Server write)
    pipe(child_to_parent); //Child-to-Server (Server Read)

    pid_t child_pid = fork();
    
    if(child_pid == 0)  //Child
    {
        close(child_to_parent[0]);
        close(parent_to_child[1]);
        
        close(STDIN_FILENO);
        dup(parent_to_child[0]);
        close(parent_to_child[0]);
        
        close(STDOUT_FILENO);
        dup(child_to_parent[1]);
        close(child_to_parent[1]);
        
        if(execlp("./verificador", "verificador", badwords, (char *) NULL) == -1)
            fprintf(stderr, "Error starting the verifier!\n");
    }
    else    //Parent
    {
        close(child_to_parent[1]);
        close(parent_to_child[0]);
    }
}

char** stringParser(const char* string)
{
    if(string == NULL)
        return NULL;
    
    int spaces = 0;
    int maxWordSize = 0;
    int currentWordSize = 0;
    
    for(int i = 0; true; i++)
    {
        if(string[i] == ' ')
        {
            spaces++;
            
            if(currentWordSize >= maxWordSize)
            {
                maxWordSize = currentWordSize;
                currentWordSize = 0;
            }
            continue;
        }
        else if(string[i] == '\0')
        {
            if(currentWordSize >= maxWordSize)
            {
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
    
    for(int i = 0; i < arraySize; i++)
    {
        parsedStrings[i] = calloc(maxWordSize, sizeof(char));
        
        if(parsedStrings[i] == NULL)
        {
            printf("Allocation Error!\n");
            
            for(int j = 0; j < i; j++)
                free(parsedStrings[j]);
            
            return NULL;
        }
    }
    
    if(arraySize == 2) //Only 1 word + NULL
    {
        for(int i = 0; string[i] != '\0'; i++)
        {
            parsedStrings[0][i] = string[i];
        }
        
        parsedStrings[1] = NULL;
    }
    else if(arraySize < 2) //something went HORRIBLY WRONG!
    {
        return NULL;
    }
    else
    {
        int i = 0;
        
        for(int y = 0; true; y++)
        {
            for(int x = 0; true; x++)
            {
                if(string[i] == ' ' || string[i] == '\0')
                {
                    parsedStrings[y][x] = '\0';
                    break;
                }
                else
                {
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

void listAllUsers(pClient clientList)
{
    if(clientList == NULL)
        return;
    
    printf("Currently Connected Users: \n");
    
    pClient aux = clientList;
    
    do
    {
        printf("%d\n", aux->c_PID);
        aux = aux->next;
    }
    while(aux != NULL);
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

void showEnvVars()
{
    getenv("MAXMSG") == NULL ? printf("MAXMSG: N/A\n") : printf("MAXMSG: %s\n", getenv("MAXMSG"));
    getenv("MAXNOT") == NULL ? printf("MAXNOT: N/A\n") : printf("MAXNOT: %s\n", getenv("MAXNOT"));
    getenv("WORDSNOT") == NULL ? printf("WORDSNOT: N/A\n") : printf("WORDSNOT: %s\n", getenv("WORDSNOT"));
}

void testVerifier(int parent_write_pipe, int parent_read_pipe)
{
    system("clear");
    
    printf("Write a message to send to the verifier (no need to write ##MSGEND##)\n");
    printf("Message: ");
    
    char test_message[1000];
    
    fgets(test_message, (1000 - 12), stdin); //1000 - 12 because I need to strcat " ##MSGEND##\n" to it
    
    test_message[strlen(test_message) - 1] = '\0';
    
    strncat(test_message, " ##MSGEND##\n", sizeof(char) * 12);
    
    write(parent_write_pipe, test_message, strlen(test_message));
    
    char verifier_response[5];
    char verifier_char;
    int i = 0;
    
    while(read(parent_read_pipe, &verifier_char, sizeof(char)) != 0)
    {
        verifier_response[i] = verifier_char;
        
        if(verifier_char == '\n')
            break;
        
        i++;
    }
    
    verifier_response[i] = '\0';
    
    printf("Number of wrong words: %s", verifier_response);
}

pClient addTestClient(pClient clientList)
{
    pClient newClient = calloc(1, sizeof(Client));
    
    printf("Enter client PID: ");
    char s_pid[10];
    fgets(s_pid, 10, stdin);
    
    s_pid[strlen(s_pid) - 1] = '\0';
    pid_t c_pid;
    
    sscanf(s_pid, "%d", &c_pid);
    
    newClient->c_PID = c_pid;
    newClient->c_pipe = 0;
    //newClient->c_thread = NULL;
    newClient->next = NULL;
    newClient->prev = NULL;
    //newClient->username = NULL;
    
    if(clientList == NULL)
        clientList = newClient;
    else
    {
        pClient aux = clientList;
        
        if(aux->next == NULL)
        {
            aux->next = newClient;
        }
        else
        {
            while(aux->next != NULL)
                aux = aux->next;

            aux->next = newClient;
        }
    }
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