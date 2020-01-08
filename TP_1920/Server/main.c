#include "serverHeader.h"

bool Exit, Filter;
pid_t childPID;
pClient clientList;
pText textList;
pTopic topicList;

pthread_mutex_t client_lock, temp_text_lock, topic_lock, text_lock;

void terminateServer(int num)
{
    fprintf(stderr, "\n\nServer recieved SIGINT\n");
    
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    fprintf(stderr, "\nThe server is shutting down\n");
    
    printf("Signalling all clients...\n");
    killAllClients(clientList);

    getchar();
    exit (EXIT_SUCCESS);
}

void clientSignals(int sigNum, siginfo_t *info, void* extras)
{
    
}

void SIGUSR1_Handler(int sigNum, siginfo_t* info, void* extra)
{
    union sigval val = (union sigval) extra;
    sigqueue(info->si_pid, SIGUSR1, val); //nao da para mandar null?

    //Find client pipe
    pClient aux_c = clientList;

    while(aux_c->next != NULL)
    {
        if(aux_c->c_PID == info->si_pid)
            break;
        aux_c = aux_c->next;
    }

    //Find text list
    pTopic aux_t = topicList;

    while(aux_t->next != NULL)
    {
        if(aux_t->id == val.sival_int)
            break;
        aux_t = aux_t->next;
    }

    write(aux_c->s_pipe, aux_t->TextStart, sizeof(pText));
}

/* ===== SERVER ===== */

int main(int argc, char** argv)
{
    if(createServerFiles() == -1)
        exit(EXIT_FAILURE);
    
    /* ===== GLOBAL VARIABLES ===== */

    Exit = false;
    Filter = true;
    clientList = NULL;
       
    /* ===== LOCAL VARIABLES ===== */

    int maxMessage, maxNot;
    char *wordNot, cmd[CMD_SIZE];
    struct sigaction cAlarm, cUSR1;
    
    /* ===== SIGNAL HANDLING ===== */

    signal(SIGINT, terminateServer);
    
    cUSR1.sa_flags = SA_SIGINFO;
    cUSR1.sa_sigaction = &SIGUSR1_Handler;
    sigaction(SIGUSR1, &cUSR1, NULL);

    /*cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    sigaction(SIGUSR1, &cSignal, NULL);*/
    
    /*cAlarm.sa_flags = SA_SIGINFO;
    cAlarm.sa_sigaction = &getClientPid;
    sigaction(SIGALRM, &cAlarm, NULL);*/
    
    /* ===== ENV VARS ===== */

    if(getenv("MAXMSG") != NULL)
        sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    if(getenv("MAXNOT") != NULL)
        sscanf(getenv("MAXNOT"), "%d", &maxNot);
    if(getenv("WORDNOT") != NULL)
        wordNot = getenv("WORDNOT");
    
    /* === PIPES === */

    int parent_read_pipe[2], parent_write_pipe[2];

    childPID = initializeVerifier(parent_write_pipe, parent_read_pipe);

    /* ===== MUTEXES ===== */
    
    pthread_mutex_init(&client_lock, NULL);
    pthread_mutex_init(&temp_text_lock, NULL);
    pthread_mutex_init(&topic_lock, NULL);
    pthread_mutex_init(&text_lock, NULL);
    
    /* ===== THREADS ===== */
    
    pthread_t newClientThread;
    pthread_t verifyMessageThread;
    
    int pipes_fd[2] = {parent_read_pipe[0], parent_write_pipe[1]};
    
    pthread_create(&newClientThread, NULL, &newClientThreadHandler, NULL);
    pthread_create(&verifyMessageThread, NULL, &verifyMessagesHandler, (void*) pipes_fd);

    /* ===== SERVER MAIN LOOP ===== */

    fprintf(stdout, "'help' para ajuda\n");
    while(!Exit)
    {
        fprintf(stdout, "\nCommand: ");
        fgets(cmd, CMD_SIZE, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        
        char **parsedCmd = stringParser(cmd);
    
        if(parsedCmd == NULL)
            fprintf(stdout, "Invalid command\n");

        if(strcmp(parsedCmd[0], "shutdown") == 0)
        {
            Exit = true;
            
            killAllClients(clientList);
        }
        else if(strcmp(parsedCmd[0], "help") == 0)
        {
            fprintf(stdout, "     shutdown - Server shutdown\n");
            fprintf(stdout, "        users - List all users\n");
            fprintf(stdout, "  kick <user> - Exclude a user\n");
            fprintf(stdout, "          msg - List all messages\n");
            fprintf(stdout, "    del <msg> - Delete a message\n");
            fprintf(stdout, "       topics - List all topics\n");
            fprintf(stdout, "topic <topic> - List messages in topic\n");
            fprintf(stdout, "        prune - Delete empty topics\n");
        }
        else if(strcmp(parsedCmd[0], "users") == 0)
        {
            listAllUsers(clientList);
        }
        else if(strcmp(parsedCmd[0], "msg") == 0)
        {
            listAllMesages(topicList);
        }
        else if(strcmp(parsedCmd[0], "topics") == 0)
        {
            listAllTopics(topicList);
        }
        else if(strcmp(parsedCmd[0], "prune") == 0)
        {
            deleteEmptyTopics(topicList);
        }
        else if(strcmp(parsedCmd[0], "filter") == 0)
        {
            if(strcmp(parsedCmd[1], "on") == 0)
                Filter = true;
            else if(strcmp(parsedCmd[1], "off") == 0)
                Filter = false;
            else
                fprintf(stdout, "Filter option not recognized\n");
        }
        /*else if(strcmp(parsedCmd[0], "addclient") == 0)
        {
            clientList = addTestClient(clientList); //just to test signalling
        }
        else if(strcmp(parsedCmd[0], "test") == 0)
        {
            testVerifier(parent_write_pipe[1], parent_read_pipe[0], NULL);
        }*/
        else
            fprintf(stdout, "Invalid command\n");
    }
    
    /* ===== SERVER SHUTDOWN ===== */
    
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    
    pthread_join(newClientThread, NULL);
    pthread_join(verifyMessageThread, NULL);
    
    pthread_mutex_destroy(&client_lock);
    pthread_mutex_destroy(&temp_text_lock);
    
    fprintf(stdout, "\nVerifier shutdown\n");
    fprintf(stdout, "\nServer will shutdown\n");

    return (EXIT_SUCCESS);
}