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

    pid_t server_pid;
    pid_t self_pid = getpid();

    pthread_t notification_thread;
    Exit = false;
    pTopic TopicList = NULL;

    int current_topic_id = 0;
    int serverpidfd;
    int client_read_pipe;
    int server_write_pipe;
    
    char pid[6]; //server pid string
    char server_main_pipe[25];  //server main pipe path string
    char client_info[MAXUSERLEN + 10] = ""; //client info string
    char client_main_pipe[15]; //client main pipe name string
    char client_main_pipe_path[50]; //client main pipe path string

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
    
    read(serverpidfd, pid, sizeof(char) * 6);
    
    sscanf(pid, "%d", &server_pid);
    
    snprintf(server_main_pipe, 25, "/tmp/msgdist/%d_main", server_pid);
    
    //Open Server Main Pipe and write client info
    
    client_read_pipe = open(server_main_pipe, O_WRONLY);
    
    if(client_read_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nServer Main Pipe open error: %s\n", strerror(errno));
        getchar();
    }
    
    snprintf(client_info, MAXUSERLEN + 10, "%s %d", username, getpid());
    
    write(client_read_pipe, client_info, strlen(client_info));
    
    //open Server Write Pipe first.
    
    snprintf(client_main_pipe, 15, PIPE_SV, self_pid);
    snprintf(client_main_pipe_path, 50, "%s/%s", MSGDIST_DIR, client_main_pipe);
    
    server_write_pipe = open(client_main_pipe_path, O_WRONLY);
    
    if(server_write_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nserver_write_pipe open error: %s\n", strerror(errno));
    }
    
    //Open Client Read second.
    //Re-Using Variables client_main_pipe & client_main_pipe_path!
    
    memset(client_main_pipe, 0, sizeof(char) * 15);
    memset(client_main_pipe_path, 0, sizeof(char) * 25);
    
    snprintf(client_main_pipe, 15, PIPE_CL, self_pid);
    snprintf(client_main_pipe_path, 50, "%s/%s", MSGDIST_DIR, client_main_pipe);
    
    mkfifo(client_main_pipe_path, 0600);
    
    client_read_pipe = open(client_main_pipe_path, O_RDONLY);
    
    if(client_read_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nclient_read_pipe open error: %s\n", strerror(errno));
    }
    
    ///////////////////
    ///Start ncurses///
    ///////////////////

    initscr();              //Starts the main ncurses screen 'stdscr'
    start_color();          //Turns terminal colors on
    noecho();               //Turns off character echoing
    curs_set(0);            //Turns off terminal cursor
    keypad(stdscr, true);   //Turns on the keypad

    //////////////////////////
    ///Init signal handling///
    //////////////////////////
    
    struct sigaction sigUSR1, sigUSR2, sigALRM;
    
    sigUSR1.sa_flags = SA_SIGINFO;
    
    
    sigUSR2.sa_flags = SA_SIGINFO;
    
    
    sigALRM.sa_flags = SA_SIGINFO;
    //sigALRM.sa_sigaction = &SIGALRM_Handler;
    
    signal(SIGINT, SIGINT_Handler);
    
    //////////////////
    ///Thread Start///
    //////////////////
    
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