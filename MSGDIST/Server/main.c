#include "serverHeader.h"

bool Exit;
bool Filter;
int childPID;

//THIS IS THE MASTER BRANCH!

//This is global, but only accessible within main.c
pClient clientList;

//This sig handling functions needs to be here to
//access the main.c only global clientList.
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

//Server
int main(int argc, char** argv)
{
    if(createServerFiles() == -1)
        exit(EXIT_FAILURE);
    
    /* === GLOBAL VARIABLES === */
    Exit = false;
    Filter = true;
    childPID = 1;
    clientList = NULL;
       
    /* ======================== */

    /* === LOCAL VARIABLES === */
    int maxMessage, maxNot;
    char* wordsNot;
    char cmd[CMD_SIZE];
    struct sigaction cSignal, cAlarm;
    /* ======================= */
    
    /* === SIGNAL HANDLING=== */
    signal(SIGINT, terminateServer);
    
    cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    sigaction(SIGUSR1, &cSignal, NULL);
    
    cAlarm.sa_flags = SA_SIGINFO;
    cAlarm.sa_sigaction = &getClientPid;
    sigaction(SIGALRM, &cAlarm, NULL);
    /* ====================== */
    
    /* === ENV VARS === */
    /*if(getenv("MAXMSG") != NULL)
        sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    if(getenv("MAXNOT") != NULL)
        sscanf(getenv("MAXNOT"), "%d", &maxNot);
    if(getenv("WORDNOT") != NULL)
        wordsNot = getenv("WORDSNOT");*/
    
    getenv("MAXMSG") == NULL ? maxMessage = MAXMSG_DEFAULT : sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    getenv("MAXNOT") == NULL ? maxNot = MAXNOT_DEFAULT : sscanf(getenv("MAXNOT"), "%d", &maxNot);
    wordsNot = getenv("WORDSNOT") == NULL ? WORDSNOT_DEFAULT : getenv("WORDSNOT");
    /* ================ */
    
    /* === PIPES === */
    int parent_read_pipe[2], parent_write_pipe[2];
    
    initializeVerifier(parent_write_pipe, parent_read_pipe, wordsNot);
    /* ============= */

    /* === SERVER MAIN LOOP === */
    fprintf(stdout, "'help' para ajuda\n");
    while(!Exit)
    {
        serverMainOutput(0);
        fgets(cmd, CMD_SIZE, stdin);
        cmd[strlen(cmd) - 1] = '\0';
        
        //serverMainLoop(cmd);
        
        char **parsedCmd = stringParser(cmd);
    
        if(parsedCmd == NULL)
            serverMainOutput(2);

        if(strcmp(parsedCmd[0], "shutdown") == 0)
        {
            killAllClients(clientList);

            Exit = true;
        }
        else if(strcmp(parsedCmd[0], "help") == 0)
        {
            serverMainOutput(3);
        }
        else if(strcmp(parsedCmd[0], "users") == 0)
        {
            listAllUsers(clientList);
        }
        else if(strcmp(parsedCmd[0], "msg") == 0)
        {
            listAllMesages();
        }
        else if(strcmp(parsedCmd[0], "topics") == 0)
        {
            listAllTopics();
        }
        else if(strcmp(parsedCmd[0], "prune") == 0)
        {
            deleteEmptyTopics();
        }
        else if(strcmp(parsedCmd[0], "filter") == 0)
        {
            if(strcmp(parsedCmd[1], "on") == 0)
                Filter = true;
            else if(strcmp(parsedCmd[1], "off") == 0)
                Filter = false;
            else
                printf("Filter option not recognized\n");
        }
        else if(strcmp(parsedCmd[0], "addclient") == 0)
        {
            clientList = addTestClient(clientList); //just to test signalling
        }
        else if(strcmp(parsedCmd[0], "test") == 0)
        {
            testVerifier(parent_write_pipe[1], parent_read_pipe[0]);
        }
        else if(strcmp(parsedCmd[0], "envvars") == 0)
        {
                showEnvVars();
        }
        else
            serverMainOutput(2);
    }
    /* ======================== */
    
    /* === SERVER SHUTDOWN === */
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    serverMainOutput(4); //nao é necessario
    serverMainOutput(1);
    /* ======================= */

    return (EXIT_SUCCESS);
}
