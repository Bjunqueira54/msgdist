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
    int current_topic_id = 0;

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

    pthread_t serverReadThread;
    
    pthread_create(&serverReadThread, NULL, &receiveTopicList, TopicList);
    
    drawBox(stdscr);
    mvwaddstr(stdscr, 1, 1, "Welcome to MSGDIST!");
    wrefresh(stdscr);
    
    do
    {
        PrintMenu(TopicList);
        
        switch(tolower(getch()))
        {
        case 'n':
            if(current_topic_id == 0)
                newMessage(TopicList);
            else
            {
                //How can current_topic_id not be 0 and this be NULL?
                //You know what they say: Better safe than SegmentationFault
                if(TopicList == NULL)
                    newMessage(TopicList);
                else
                {
                    pTopic aux = TopicList;

                    while(aux != NULL)
                    {
                        if(aux->id == current_topic_id)
                            newTopicMessage(aux->title);
                    }
                }
            }
            break;
        case KEY_UP:
            clear();
            mvwaddstr(stdscr, 5, 5, "I pressed Arrow Up");
            refresh();
            break;
        case KEY_DOWN:
            clear();
            mvwaddstr(stdscr, 5, 5, "I pressed Arrow Down");
            refresh();
            break;
        case KEY_RIGHT: //KEY_RIGHT and KEY_LEFT aren't used right now
            clear();
            mvwaddstr(stdscr, 5, 5, "I pressed Arrow Right");
            refresh();
            break;
        case KEY_LEFT:  //but it won't hurt to have them here just in case.
            clear();
            mvwaddstr(stdscr, 5, 5, "I pressed Arrow Left");
            refresh();
            break;
        case 10:
            clear();
            mvwaddstr(stdscr, 5, 5, "I pressed enter");
            refresh();
            break;
        case 27:
            Exit = true;
            break;
        default:
            break;
        }   
        
        refresh();
    } while(!Exit);
    
    pthread_kill(serverReadThread, SIGINT);
    pthread_join(serverReadThread, NULL);
    
    endwin();
    
    printf("Client will shutdown\n");
    
    return (EXIT_SUCCESS);
}