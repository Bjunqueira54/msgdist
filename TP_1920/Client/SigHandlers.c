#include "SigHandlers.h"

//Find another way to handle this signal.
//exit() is currently here because
//main() hangs on getch() and doesn't
//recheck Exit. Not too serious, but not
//perfect either.
void SIGINT_Handler(int arg)
{
    endwin();
    printf("Server sent a SIGINT\n");

    exit (EXIT_SUCCESS);
}

void SIGUSR1_Handler(int signal, siginfo_t* info, void* extra)
{
    pText text;

    read(ServerPipe, text, sizeof(pText));

    //continue
}

void SIGALRM_Handler(int signal, siginfo_t* info, void* extra)
{
    
}