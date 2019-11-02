#include "serverHeader.h"

bool Exit;
bool Filter;

//Server
int main(int argc, char** argv)
{
    if(createServerFiles() == -1)
        exit (EXIT_FAILURE);
    
    //fuck netbeans
    
    pClient clientList = NULL;
    Exit = false;
    Filter = true;
    childPID = 1;
    int maxMessage, maxNot;
    char* wordNot;
    char cmd[CMD_SIZE];
    struct sigaction cDisconnect, cSignal;
    
    cSignal.sa_flags = SA_SIGINFO;
    cSignal.sa_sigaction = &clientSignals;
    
    //sigaction(SIGINT, &cDisconnect, NULL);
    sigaction(SIGUSR1, &cSignal, NULL);
    
    //EnvVars
    if(getenv("MAXMSG") != NULL)
        sscanf(getenv("MAXMSG"), "%d", &maxMessage);
    if(getenv("MAXNOT") != NULL)
        sscanf(getenv("MAXNOT"), "%d", &maxNot);
    if(getenv("WORDNOT") != NULL)
        wordNot = getenv("WORDNOT");
  
    //escrever para a extremidade do pipe p[1]  
    //kill(childPID, SIGUSR2); -> para emergencias
    
    fprintf(stdout, "'help' para ajuda\n");
    
    //Server Main Loop
    while(!Exit) {
        serverMainOutput(0);
        fgets(cmd, CMD_SIZE, stdin);
        
        serverMainLoop(cmd, clientList);
    }
    
    serverMainOutput(1);
    
    
    return (EXIT_SUCCESS);
}
