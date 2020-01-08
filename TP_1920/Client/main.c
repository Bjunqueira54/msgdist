#include "clientHeader.h"

bool Exit;

pthread_mutex_t mlock;

//Client
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        fprintf(stderr, "You need to indicate your username\n");
        fprintf(stdout, "Example usage: './client -u user1337'\n");
        exit (EXIT_FAILURE);
    }

    char username[MAXUSERLEN] = "";

    char c = getopt(argc, argv, "u:");

    switch(c)
    {
        case 'u':
            memcpy(username, optarg, strlen(optarg));
            fprintf(stdout,"Your username is: %s\n", username);
            break;
        default:
            fprintf(stderr, "I don't recognize that launch argument\n");
            exit (EXIT_FAILURE);
            break;
    }

    if(*username == 0)
    {
        fprintf(stderr, "You didn't specify a username!\nEXITING!\n");
        exit (EXIT_FAILURE);
    }

    ////////////////////////
    ///Init all variables///
    ////////////////////////

    Exit = false;
    pTopic TopicList = NULL;
    
    //////////////////////////
    ///Init signal handling///
    //////////////////////////

    struct sigaction sigUSR1;

    sigUSR1.sa_flags = SA_SIGINFO;
    sigUSR1.sa_sigaction = &SIGUSR1_Handler;
    sigaction(SIGUSR1, &sigUSR1, NULL);
    
    signal(SIGUSR2, SIGUSR2_Handler);
    signal(SIGINT, SIGINT_Handler);

    ///////////
    ///Pipes///
    ///////////

    createPipes(username);
    
    ///////////////////
    ///Start ncurses///
    ///////////////////
    
    initscr();              //Starts the main ncurses screen 'stdscr'
    start_color();          //Turns terminal colors on
    noecho();               //Turns off character echoing
    curs_set(0);            //Turns off terminal cursor
    keypad(stdscr, true);   //Turns on the keypad
    
    ////////////////
    ///Mutex Init///
    ////////////////

    pthread_mutex_init(&mlock, NULL);

    //////////////////
    ///Thread Start///
    //////////////////

    pthread_t serverReadThread, userInputThread;
    
    pthread_create(&userInputThread, NULL, &UserInputThreadHandler, TopicList);
    pthread_create(&serverReadThread, NULL, &receiveTopicList, TopicList);
    
    drawBox(stdscr);
    mvwaddstr(stdscr, 1, 1, "Welcome to MSGDIST!");
    wrefresh(stdscr);
    
    while(!Exit) { sleep(1); }
    
    pthread_kill(serverReadThread, SIGINT);
    pthread_join(serverReadThread, NULL);
    
    pthread_kill(userInputThread, SIGINT);
    pthread_join(userInputThread, NULL);
    
    endwin();
    
    return (EXIT_SUCCESS);
}