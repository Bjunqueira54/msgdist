#include "SigHandlers.h"

//Find another way to handle this signal.
//exit() is currently here because
//main() hangs on getch() and doesn't
//recheck Exit. Not too serious, but not
//perfect either.
void SIGINT_Handler(int arg) //MUDANCA FEITA AQUI PARA O CLIENTE RECEBER SIGINT <----------
{
    //Exit = true;
    printf("Server sent a SIGINT\n");
    printf("Client will shutdown\n");
    pthread_kill(serverReadThread, SIGINT);
    pthread_join(serverReadThread, NULL);
    endwin();
    close(client_read_pipe);
    close(server_write_pipe);
    exit(EXIT_SUCCESS);
}

void SIGUSR1_Handler(int signal, siginfo_t* info, void* extra) //Client
{
    pText text;

    //tem de ler todas
    read(client_read_pipe, text, sizeof(Text));

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