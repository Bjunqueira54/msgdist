#include "SigHandlers.h"

//Find another way to handle this signal.
//exit() is currently here because
//main() hangs on getch() and doesn't
//recheck Exit. Not too serious, but not
//perfect either.
void SIGINT_Handler(int arg)
{
    Exit = true;
    printf("Server sent a SIGINT\n");
}

void SIGUSR1_Handler(int signal, siginfo_t* info, void* extra) //Client
{
    pText text;

    read(client_read_pipe, text, sizeof(pText));

    //continue
}

void SIGUSR2_Handler(int signal) //Client
{
    char c = 'A';
    pthread_mutex_lock(&mlock);
    write(server_write_pipe, &c, sizeof(char));
    sleep(1);
    pthread_mutex_unlock(&mlock);
}