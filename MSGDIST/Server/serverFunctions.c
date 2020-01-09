#include "serverHeader.h"

int server_file, maxMessages;

pid_t initializeVerifier(int *parent_to_child, int *child_to_parent)
{
    pipe(parent_to_child); //Server-to-Child pipe (Server write)
    pipe(child_to_parent); //Child-to-Server (Server Read)

    pid_t child_pid = fork();
    
    if(child_pid == 0) // Child
    { 
        /* Close unused ends */
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
    else // Parent
    {
        close(child_to_parent[1]);
        close(parent_to_child[0]);
        
        return child_pid;
    }
}

void* receiveMsgHandler(void* data)
{
    pText msg = (pText) data;

    msg->article[strlen(msg->article) - 1] = '\0';

    strncat(msg->article, " ##MSGEND##\n", sizeof(char) * 12);

    //start reading from pipe
}

void receiveClientMsg()
{

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
        return NULL;
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
                    parsedStrings[y][x] = string[i];
                
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

void addNewMessage(pText first, pText newMsg)
{
    if(first == NULL)
    {
        newMsg->next = newMsg->prev = NULL;
        first = newMsg;
        return;
    }
    
    pText aux = first;
    
    if(countMsgs(aux) == maxMessages)
    {
        fprintf(stderr, "Max number of messages reached\n");
        return;
    }

    if(aux->next == NULL)
        aux->next = newMsg;
    else
    {
        while(aux->next != NULL)
            aux = aux->next;
        
        aux->next = newMsg;
    }

    fprintf(stderr, "New message added\n");
}

int countMsgs(pText m)
{
    int num = 0;

    if(m == NULL)
        return num;
    
    do
    {
        m = m->next;
        num++;
    }
    while(m != NULL);

    return num;
}

void sendToClients()
{
    pthread_mutex_lock(&topic_lock);
    for(pClient aux_c = clientList; aux_c != NULL; aux_c = aux_c->next)
    {
        for(pTopic topic_it = topicList; topic_it != NULL; topic_it = topic_it->next)
        {
            Topic aux;
            
            aux.id = topic_it->id;
            strcpy(aux.title, topic_it->title);
            
            write(aux_c->c_pipe, &aux, sizeof(Topic));
        }
    }
    pthread_mutex_unlock(&topic_lock);
}

void listAllUsers(pClient clientList)
{
    if(clientList == NULL)
    {
        printf("No users");
        return;
    }
    
    printf("\nCurrently Connected Users:\n");
    
    pClient aux = clientList;
    
    do
    {
        printf("User: %s - PID: %d\n", aux->username, aux->c_PID);
        aux = aux->next;
    }
    while(aux != NULL);
}

void listAllMesages(pTopic list)
{
    if(list == NULL)
    {
        printf("There are no messages on the server\n");
        return;
    }
    printf("\nAll Messages on the server:\n");

    pthread_mutex_lock(&topic_lock);
    for(pTopic topic_run = list; topic_run != NULL; topic_run = topic_run->next)
    {
        for(pText text_run = topic_run->TextStart; text_run != NULL; text_run = text_run->next)
        {
            fprintf(stdout, "%s (%s)\n", text_run->title, topic_run->title);
        }
    }
    pthread_mutex_unlock(&topic_lock);
}

void listAllTopics(pTopic list)
{
    if(list == NULL)
    {
        printf("There are no topics on the server\n");
        return;
    }

    pTopic aux = list;
    
    printf("\nCurrent Topics:\n");

    do
    {
        printf("%s\n", aux->title);
        aux = aux->next;
    }
    while(aux != NULL);
}

void deleteMsg(pTopic list)
{
    printf("\nComando reconhecido\n");
}

void deleteEmptyTopics(pTopic list)
{
    if(list == NULL)
        return;

    pTopic aux_prev = NULL, aux_act = list;

    printf("Deleting all empty topics from the server.\n");

    if(list->next == NULL)
    {
        free(aux_act);
        list = NULL;
    }

    aux_prev = aux_act;
    aux_act = aux_act->next;

    do
    {
        if(aux_act->TextStart == NULL)
        {
            pTopic tmp = aux_act;
            aux_prev->next = aux_act->next;
            free(tmp);
            aux_act = aux_act->next;
        }

        aux_prev = aux_act;
        aux_act = aux_act->next;
    }
    while(aux_act != NULL);
}

void killAllClients(pClient clientList)
{
    if(clientList == NULL)
        return;
    
    union sigval value;
    value.sival_int = 0;
    
    for(pClient aux = clientList; aux != NULL;)
    {
        kill(aux->c_PID, SIGINT);
        
        if(aux->c_thread != 0)
        {
            pthread_kill(aux->c_thread, SIGINT);
            pthread_join(aux->c_thread, NULL);
            
            pthread_kill(aux->KeepAliveThread, SIGINT);
            pthread_join(aux->KeepAliveThread, NULL);
            
            pthread_mutex_destroy(&aux->pipe_lock);
            
            close(aux->c_pipe);
            close(aux->s_pipe);
        }
        
        pClient aux2 = aux;
        aux = aux->next;
        free(aux2);
    }
}

void purgeClients()
{
    if(clientList == NULL)
        return;
    
    if(clientList->next == NULL && clientList->Disconnect)
        clientList = NULL;
    
    for(pClient aux = clientList; aux != NULL;)
    {
        if(aux->Disconnect)
        {
            pClient Next, Prev;
            
            Next = aux->next;
            Prev = aux->prev;
            
            if(Prev != NULL)
                Prev->next = Next;
            if(Next != NULL)
                Next->prev = Prev;
            
            free(aux);
            aux = Next;
            continue;
        }
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
    
    system("rm /tmp/msgdist/*");
    system("rmdir /tmp/msgdist");
    
    return 0; 
}

int sendTextToVerifier(int parent_read_pipe, int parent_write_pipe, pText newText)
{
    char aux[MAXTEXTLEN + 12]; //MAXTEXTLEN + ##MSGEND## + \n + \0
    strcpy(aux, newText->article);
    strncat(aux, " ##MSGEND##\n", sizeof(char) * 12);
    
    write(parent_write_pipe, aux, strlen(aux));
    
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
    
    return atol(verifier_response);
}