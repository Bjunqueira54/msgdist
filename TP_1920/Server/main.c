#include "serverHeader.h"

bool Exit;
bool Filter;
int childPID;
pClient clientList;

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
    char* wordNot;
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
        
        serverMainLoop(cmd);
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
