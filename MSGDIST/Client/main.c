#include <sys/stat.h>

#include "clientHeader.h"

bool Exit;
int ServerPipe;

//Client
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        fprintf(stderr, "You need to indicate your username\n");
        fprintf(stdout, "Example usage: './client -u user1337'\n");
        exit (EXIT_FAILURE);
    }

    char username[MAXUSERLEN];
    *username = 0;

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

    ////////////////////////////////
    ///Iniciar todas as variaveis///
    ////////////////////////////////

    pid_t server_pid;
    pid_t self_pid = getpid();

    pthread_t notification_thread;
    Exit = false;
    pTopic TopicList = NULL;

    int current_topic_id = 0;
    int serverpidfd;
    int self_pipe_fd;
    int server_pipe_fd;
    
    char pid[6];
    char self_pipe_path[50];
    char server_pipe_path[50];

    ///////////
    ///Pipes///
    ///////////

    serverpidfd = open(SERVER_PID, O_RDONLY);

    if(serverpidfd == -1)
    {
        fprintf(stderr, "Server not running!");
        getchar();
        exit (EXIT_FAILURE);
    }
    
    read(serverpidfd, pid, sizeof(char)*6);
    
    sscanf(pid, "%d", &server_pid);

    if(server_pid == 0)
    {
        fprintf(stderr, "Something went wrong reading the server PID\n");
        exit (EXIT_FAILURE);
    }
    
    /*memset(self_pipe_path, 0, 50);
    snprintf(self_pipe_path, 50, "%s/client_%d_r\0", MSGDIST_DIR, self_pid);
    
    printf("Your pipe will be created in: %s\n", self_pipe_path);
    printf("(press enter to continue...)");
    getchar();
    
    mkfifo(self_pipe_path, S_IRUSR | S_IWUSR);
    self_pipe_fd = open(self_pipe_path, O_RDONLY);
    
    kill(server_pid, SIGALRM);
    
    snprintf(server_pipe_path, 50, "%s/client_%d_w\n", MSGDIST_DIR, self_pid);
    server_pipe_fd = open(server_pipe_path, O_WRONLY);*/

    ///////////////////////
    ///Iniciar o ncurses///
    ///////////////////////

    initscr();              //Inicia a janela principal 'stdscr' do ncurses
    start_color();          //Liga as cores
    noecho();               //Desliga o echo'ing de characteres
    curs_set(0);            //Desliga o piscar do cursor no terminal
    keypad(stdscr, true);   //Liga o keypad

    ////////////////////////////////////
    ///Iniciar o tratamento do sinais///
    ////////////////////////////////////
    
    struct sigaction sigUSR1, sigUSR2, sigALRM;
    
    sigUSR1.sa_flags = SA_SIGINFO;
    
    
    sigUSR2.sa_flags = SA_SIGINFO;
    
    
    sigALRM.sa_flags = SA_SIGINFO;
    //sigALRM.sa_sigaction = &SIGALRM_Handler;
    
    signal(SIGINT, SIGINT_Handler);
    
    ////////////////////////
    ///Iniciar as Threads///
    ////////////////////////
    
    drawBox(stdscr);
    mvwaddstr(stdscr, 1, 1, "Welcome to MSGDIST!");
    wrefresh(stdscr);
    
    while(!Exit)
    {
        PrintMenu(TopicList);
        
        switch(tolower(getch()))
        {
            case 'n':
            {
                if(current_topic_id == 0)
                    newMessage(TopicList);
                else
                {
                    //How can current_topic_id not be 0 and this be NULL?
                    //You know what they say: Better safe than SegFault
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
            }
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
    }
    
    endwin();
    
    return (EXIT_SUCCESS);
}