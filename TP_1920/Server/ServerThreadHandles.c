#include "ServerThreadHandles.h"

void ThreadKill(int sig)
{
    pthread_exit((void*) NULL);
}

void* newClientThreadHandle(void* arg)
{
    signal(SIGINT, ThreadKill);
    
    pClient clientList = (pClient) arg;
    
    char main_pipe[15], main_pipe_path[25];
    
    snprintf(main_pipe, 15, MAIN_PIPE, getpid());
    snprintf(main_pipe_path, 25, "%s/%s", MSGDIST_DIR, main_pipe);
    
    mkfifo(main_pipe_path, 0600);
    
    int pipe_fd = open(main_pipe_path, O_RDONLY);
    fd_set fd_set;
    struct timeval timeout;
    
    char buffer[50] = "";
    
    while(!Exit)
    {
        FD_ZERO(&fd_set);
        FD_SET(pipe_fd, &fd_set);
        
        timeout.tv_sec = 1; //seconds
        timeout.tv_usec = 0; //microseconds
        
        if(select(pipe_fd + 1, &fd_set, NULL, NULL, &timeout) > 0)
        {
            if(FD_ISSET(pipe_fd, &fd_set))
            {
                pthread_mutex_lock(&client_lock);
                
                pClient newClient = malloc(sizeof(Client));
                
                int n = read(pipe_fd, buffer, sizeof(char) * 50);
                
                if(n > 0)
                {
                    buffer[n-1] = '\0';
                    sscanf(buffer, "%d", &newClient->c_PID);
                    memset(buffer, 0, sizeof(char) * 50);
                }
                
                n = read(pipe_fd, buffer, sizeof(char) * 50);
                
                if(n > 0)
                {
                    buffer[n-1] = '\0';
                    sscanf(buffer, "%s", newClient->username);
                    memset(buffer, 0, sizeof(char) * 50);
                }
                
                newClient = createNewClientPipes(newClient);
                addNewClient(clientList, newClient);
                
                pthread_mutex_unlock(&client_lock);
            }
        }
    }
    
    ThreadKill(SIGINT);
}