#include "ServerThreadHandles.h"

pClient clientList;
int searchForTopic(const char*);

void ThreadKill(int sig)
{
    pthread_exit((void*) NULL);
}

void* newClientThreadHandler(void* arg)
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
        
        timeout.tv_sec = 0; //seconds
        timeout.tv_usec = 2500; //microseconds
        
        select_result = select(pipe_fd + 1, &fds, NULL, NULL, &timeout);
        if(select_result > 0)
        {
            if(FD_ISSET(pipe_fd, &fds))
            {
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
                    free(newClient);
            }
        }
    }
    
    ThreadKill(SIGINT);
}

void* awaitClientHandler(void* arg)
{
    signal(SIGINT, ThreadKill);

    pClient client = (pClient) arg;
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
                pText newText = malloc(sizeof(Text)); //Allocate memory dynamically for text
                
                if(newText == NULL)
                {
                    perror("Error alocating memory for a new Text Structure\n"
                                    "ServerThreadHandles.c - newText allocating Failed");
                    continue;
                }

                int bytes_read;
                
                bytes_read = read(client->s_pipe, newText , sizeof(Text));
                
                if(bytes_read > 0)
                {
                    char topicTitle[MAXTITLELEN];
                    bytes_read = read(client->s_pipe, topicTitle, sizeof(char) * MAXTITLELEN);
                    
                    if(bytes_read > 0)
                    {
                        newText->topic = malloc(sizeof(Topic));
                        
                        if(newText->topic == NULL)
                        {
                            perror("Error allocating memory for a new Topic Structure\n"
                                    "ServerThreadHandles.c - newTopic allocating Failed");
                            continue;
                        }
                        strcpy(newText->topic->title, topicTitle);
                    }
                    
                    pthread_mutex_lock(&temp_text_lock);
                    
                    if(textList == NULL)
                    {
                        textList = newText;
                        pthread_mutex_unlock(&temp_text_lock);
                        continue;
                    }
                    
                    pText aux = textList;

                    while (aux->next != NULL) //search until the end of the list
                        aux = aux->next;

                    aux->next = newText;
                    pthread_mutex_unlock(&temp_text_lock);
                }
                else
                    free(newText);
            }
        }
    }

    ThreadKill(SIGINT);
}

void* verifyMessagesHandler(void* pipes)
{
    signal(SIGINT, ThreadKill);
    
    int* pipes_fd = (int*) pipes;
    
    //pipes_fd[0]; The Read from Verifier Pipe
    //pipes_fd[1]; The Write to Verifier Pipe
    
    while(!Exit)
    {
        if(textList == NULL)
            continue;
        
        pText newText = textList;
        
        //I just need to lock this specific part
        //After extracting a Text from the list,
        //other threads can add more while this thread
        //works on the extracted text.
        pthread_mutex_lock(&temp_text_lock);
        
        if(textList->next != NULL)
        {
            textList = textList->next;
            textList->prev = NULL;
            newText->next = newText->prev = NULL;
        }
        else
            textList = NULL;
        
        pthread_mutex_unlock(&temp_text_lock);
        
        int wrong_words = sendTextToVerifier(pipes_fd[0], pipes_fd[1], newText);
        
        //Leave at 3 for now, fix env_vars later
        if(wrong_words > 3)
        {
            pTopic free_topic_aux = newText->topic;
            newText->topic = NULL;
            free(free_topic_aux);
            free(newText);
            continue;
        }
        
        pthread_mutex_lock(&topic_lock);
        
        if(topicList == NULL)
        {
            topicList = newText->topic;

            topicList->TextStart = newText;
            topicList->id = 1;  //ids start from 1
            topicList->next = topicList->prev = NULL;
            pthread_mutex_unlock(&topic_lock);
            continue;
        }
        
        int existing_id = searchForTopic(newText->topic->title);

        //this should be impossible, but I have to test every
        //angle possible and prevent errors.
        //This is a multi-threaded application, errors are fatal
        if(existing_id < 1)
        {
            perror("Error in verifyMessagesHandler\n"
                    "existing_id less than 1 but topicList is NULL\n");

            pText text_aux;

            pthread_mutex_lock(&temp_text_lock);
            for(text_aux = textList; text_aux->next != NULL; text_aux = text_aux->next);

            text_aux->next = newText;
            newText->prev = text_aux;
            pthread_mutex_unlock(&temp_text_lock);
            pthread_mutex_unlock(&topic_lock);
            continue;
        }
        else //Add to global linked list
        {
            pTopic topic_run;
            
            for(topic_run = topicList; topic_run->id !=  existing_id && topic_run->next != NULL; topic_run = topic_run->next);
            
            //Something went wrong, but that's ok
            if(existing_id != topic_run->id && topic_run->next == NULL)
            {
                topic_run->next = newText->topic;
                newText->topic->prev = topic_run;
                newText->topic->id = topic_run->id + 1;
                newText->topic->TextStart = newText;
            }
            else
            {
                pTopic free_topic_aux = newText->topic;
                newText->topic = topic_run;
                free(free_topic_aux);
                
                pText text_run;
                
                for(text_run = topic_run->TextStart; text_run->next != NULL; text_run = text_run->next);
                
                text_run->next = newText;
                newText->prev = text_run;
            }
        }
        pthread_mutex_unlock(&topic_lock);
    }
    ThreadKill(SIGINT);
}

int searchForTopic(const char* TopicTitle)
{
    if(TopicTitle == "")
        return -1;
    if(topicList == NULL)
        return -1;
    
    pTopic aux = topicList;
    
    while(aux != NULL)
    {
        if(strcmp(aux->title, TopicTitle) == 0)
            return aux->id;
    }
    
    return -1;
}

void* keepAliveThreadHandler(void* arg)
{
    signal(SIGINT, ThreadKill);
    
    pClient client = (pClient) arg;
    
    //How?
    if(client == NULL)
    {
        ThreadKill(SIGINT);
        return ((void*) NULL);
    }
    
    fd_set fds;
    struct timeval t;
    int timeout = 0;
    
    while(!Exit)
    {
        if(timeout >= 10) //Client timed out
        {
            removeClient(client);
        }
        
        FD_ZERO(&fds);
        FD_SET(client->s_pipe, &fds); //Set pipe into listening mode
        
        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds
        
        if(select(client->s_pipe + 1, &fds, NULL, NULL, &t) > 0) //if > 0, something has been read
        { 
            if(FD_ISSET(client->s_pipe, &fds)) //Confirm that pipe was read
            {
                
            }
        }
        
        sleep(10);
    }
    
    ThreadKill(SIGINT);
}