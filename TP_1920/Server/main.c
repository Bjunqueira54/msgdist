#include "serverHeader.h"

bool Exit, Filter;
pid_t childPID;
pClient clientList;
pText textList;
pTopic topicList;

void terminateServer(int num)
{
    fprintf(stderr, "\n\nServidor recebeu SIGINT\n");
    
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    fprintf(stderr, "\nGestor vai desligar\n");
    
    printf("Signalling all clients...\n");
    killAllClients(clientList);

    getchar();
    exit (EXIT_SUCCESS);
}

void clientSignals(int sigNum, siginfo_t *info, void* extras)
{
    
}

void getClientPid(int sigNum, siginfo_t *info, void* extras)
{
    pClient newClient = createNewClient(info->si_pid);
    addNewClient(clientList, newClient);
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

    pTopic topicList;
    pText textList;
    int maxMessage, maxNot;
    char *wordNot, cmd[CMD_SIZE];
    struct sigaction cSignal, cAlarm;
    
    /* ===== SIGNAL HANDLING ===== */

    signal(SIGINT, terminateServer);
    
    cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    sigaction(SIGUSR1, &cSignal, NULL);
    
    cAlarm.sa_flags = SA_SIGINFO;
    cAlarm.sa_sigaction = &getClientPid;
    sigaction(SIGALRM, &cAlarm, NULL);
    
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

    /* ===== THREADS ===== */

    /* ===== SERVER MAIN LOOP ===== */

    fprintf(stdout, "'help' para ajuda\n");
    while(!Exit)
    {
        fprintf(stdout, "\nCommand: ");
        fgets(cmd, CMD_SIZE, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        
        //serverMainLoop(cmd);
        
        char **parsedCmd = stringParser(cmd);
    
        if(parsedCmd == NULL)
            fprintf(stdout, "Invalid command\n");

        if(strcmp(parsedCmd[0], "shutdown") == 0)
        {
            killAllClients(clientList);

            Exit = true;
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
            listAllMesages(textList);
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
        else if(strcmp(parsedCmd[0], "addclient") == 0)
        {
            clientList = addTestClient(clientList); //just to test signalling
        }
        else if(strcmp(parsedCmd[0], "test") == 0)
        {
            testVerifier(parent_write_pipe[1], parent_read_pipe[0], NULL);
        }
        else
            fprintf(stdout, "Invalid command\n");
    }
    
    /* ===== SERVER SHUTDOWN ===== */
    
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    
    fprintf(stdout, "\nVerifier shutdown\n");
    fprintf(stdout, "\nServer will shutdown\n");

    return (EXIT_SUCCESS);
}
