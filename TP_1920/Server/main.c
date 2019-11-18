#include "serverHeader.h"

bool Exit;
bool Filter;
int childPID;

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

//Server
int main(int argc, char** argv)
{
    if(createServerFiles() == -1)
        exit(EXIT_FAILURE);
    
    /* === GLOBAL VARIABLES === */
    Exit = false;
    Filter = true;
    childPID = 1;
       
    /* ======================== */

    /* === LOCAL VARIABLES === */
    pClient clientList = NULL;
    int maxMessage, maxNot;
    char* wordNot;
    char cmd[CMD_SIZE];
    struct sigaction cSignal, cAlarm, cSigInt;
    /* ======================= */
    
    /* === SIGNAL HANDLING=== */
    //signal(SIGINT, terminateServer);
    cSigInt.sa_flags = SA_SIGINFO;
    cSigInt.sa_sigaction = &terminateServer;
    siginfo_t sigint_info;
    sigint_info._sifields._rt.si_sigval.sival_ptr = clientList;
    sigaction(SIGINT, &cSigInt, NULL);
    
    cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    sigaction(SIGUSR1, &cSignal, NULL);
    
    cAlarm.sa_flags = SA_SIGINFO;
    cAlarm.sa_sigaction = &getClientPid;
    sigaction(SIGALRM, &cAlarm, NULL);
    /* ====================== */
    
    /* === ENV VARS === */
    if(getenv("MAXMSG") != NULL)
        sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    if(getenv("MAXNOT") != NULL)
        sscanf(getenv("MAXNOT"), "%d", &maxNot);
    if(getenv("WORDNOT") != NULL)
        wordNot = getenv("WORDNOT");
    /* ================ */
    
    /* === PIPES === */
    int parent_read_pipe[2], parent_write_pipe[2];
    
    initializeVerifier(parent_write_pipe, parent_read_pipe);
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
