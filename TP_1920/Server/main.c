#include "serverHeader.h"

bool Exit;
bool Filter;
int childPID;

//Server
int main(int argc, char** argv)
{
    if(createServerFiles() == -1)
        exit(EXIT_FAILURE);
    
    /* === GLOBAL VARIABLES === */
    Exit = false;
    Filter = true;
    childPID = 1;

    /* === LOCAL VARIABLES === */
    pClient clientList = NULL;    
    int maxMessage, maxNot, veriPipe[2], servPipe[2];
    char* wordNot;
    char cmd[CMD_SIZE];
    struct sigaction cSignal, cAlarm;
    
    signal(SIGINT, terminateServer);
    
    cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    sigaction(SIGUSR1, &cSignal, NULL);
    
    cAlarm.sa_flags = SA_SIGINFO;
    cAlarm.sa_sigaction = &getClientPid;
    sigaction(SIGALRM, &cAlarm, NULL);
    
    /* === ENV VARS === */
    if(getenv("MAXMSG") != NULL)
        sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    if(getenv("MAXNOT") != NULL)
        sscanf(getenv("MAXNOT"), "%d", &maxNot);
    if(getenv("WORDNOT") != NULL)
        wordNot = getenv("WORDNOT");
    
    /* ===================================================== */
    /* ================ CONFIGURAR O SIGINT ================ */
    /* ===================================================== */    
    
    /* === PIPES === */
    pipe(servPipe);
    pipe(veriPipe);
    initializeVerifier(servPipe, veriPipe);

    fprintf(stdout, "'help' para ajuda\n");

    /* === SERVER MAIN LOOP === */
    while(!Exit) {
        serverMainOutput(0);
        fgets(cmd, CMD_SIZE, stdin);
        serverMainLoop(cmd, clientList);
        verifyNewMessage(servPipe, veriPipe);
    }
    
    
    deleteServerFiles();
    kill(childPID, SIGUSR2);
    
    serverMainOutput(4);
    serverMainOutput(1);

    return (EXIT_SUCCESS);
}
