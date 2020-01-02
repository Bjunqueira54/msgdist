#include "ServerThreadHandles.h"

pClient clientList;

void ThreadKill(int sig)
{
    pthread_exit((void*) NULL);
}

void* newClientThreadHandle(void* arg)
{
    signal(SIGINT, ThreadKill);
    
    char main_pipe[15], main_pipe_path[25];
    int select_result;
    
    snprintf(main_pipe, 15, MAIN_PIPE, getpid());
    snprintf(main_pipe_path, 25, "%s/%s", MSGDIST_DIR, main_pipe);
    
    mkfifo(main_pipe_path, 0600);
    
    int pipe_fd = open(main_pipe_path, O_RDONLY);
    fd_set fds;
    struct timeval timeout;
    
    char buffer[50] = "";
    
    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(pipe_fd, &fds);
        
        timeout.tv_sec = 1; //seconds
        timeout.tv_usec = 0; //microseconds
        
        select_result = select(pipe_fd + 1, &fds, NULL, NULL, &timeout);
        if(select_result > 0)
        {
            if(FD_ISSET(pipe_fd, &fds))
            {
                pthread_mutex_lock(&client_lock);
                
                pClient newClient = malloc(sizeof(Client));
                
                int n = read(pipe_fd, buffer, sizeof(char) * 50);
                
                if(n > 0)
                {
                    char** client_info = stringParser(buffer);
                    
                    strncpy(newClient->username, client_info[0], MAXUSERLEN);
                    sscanf(client_info[1], "%d", &newClient->c_PID);
                }
                
                newClient = createNewClientPipes(newClient);
                clientList = addNewClient(clientList, newClient);
                
                pthread_mutex_unlock(&client_lock);
            }
        }
        /*else if(select_result == 0)
        {
            printf(stdout, "(newClientThread) select fd result: %d\n", select_result);
        }*/
    }
    
    ThreadKill(SIGINT);
}