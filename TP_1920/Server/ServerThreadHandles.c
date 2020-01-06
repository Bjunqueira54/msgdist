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
                    
                    newClient = createNewClientPipes(newClient);
                    clientList = addNewClient(clientList, newClient);
                }
                else
                {
                    free(newClient);
                }
                
                pthread_mutex_unlock(&client_lock);
            }
        }
    }
    
    ThreadKill(SIGINT);
}

void* awaitClientHandler(void* data)
{
    signal(SIGINT, ThreadKill);

    pClient client = (pClient) data;
    fd_set fds;
    struct timeval t;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(client->s_pipe, &fds); //Set pipe into listening mode

        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds

        if(select(client->s_pipe + 1, &fds, NULL, NULL, &t) > 0) //if > 0, something has been read
        { 
            if(FD_ISSET(client->s_pipe, &fds)) //Confirm that pipe was read
            { 
                pthread_mutex_lock(&temp_text_lock);

                pText newText = malloc(sizeof(Text)); //Allocate memory dynamically for text
                
                if(newText == NULL)
                {
                    fprintf(stderr, "Error while allocating memory for new text\n");
                    return NULL;
                }

                int bytes_read;
                
                bytes_read = read(client->s_pipe, newText , sizeof(Text));
                
                
                if(bytes_read > 0)
                {
                    if(textList == NULL)
                    {
                        textList = newText;
                        continue;
                    }
                    
                    pText aux = textList;

                    while (aux->next != NULL) //search until the end of the list
                        aux = aux->next;

                    aux->next = newText;
                }
                else
                    free(newText);

                pthread_mutex_unlock(&temp_text_lock);
            }
        }
    }

    ThreadKill(SIGINT);
}