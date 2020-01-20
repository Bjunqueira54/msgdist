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
    struct timeval t;
    
    char buffer[50] = "";
    
    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(pipe_fd, &fds);
        
        t.tv_sec = 0; //seconds
        t.tv_usec = 2500; //microseconds
        
        select_result = select(pipe_fd + 1, &fds, NULL, NULL, &t);
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
                    
                    newClient = populateClientStruct(newClient);
                    
                    if(newClient != NULL)
                        clientList = addNewClient(clientList, newClient);
                }
                else
                    free(newClient);
            }
        }
        
        purgeClients(); //Remove all disconnected clients
    }
    
    pthread_exit((void*) NULL);
}

void* newMessageThreadHandler(void* arg)
{
    signal(SIGINT, ThreadKill);

    pClient cli = (pClient) arg;
    fd_set fds;
    struct timeval t;

    while(!Exit && !cli->Disconnect)
    {
        if(pthread_mutex_trylock(&cli->pipe_lock) != 0)
            continue;
        
        FD_ZERO(&fds);
        FD_SET(cli->s_pipe, &fds); //Set pipe into listening mode

        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds

        if(select(cli->s_pipe + 1, &fds, NULL, NULL, &t) > 0) //if > 0, something has been read
        {
            if(FD_ISSET(cli->s_pipe, &fds)) //Confirm that pipe was read
            {
                pText newText = malloc(sizeof(Text)); //Allocate memory dynamically for text
                
                if(newText == NULL)
                {
                    perror("Error alocating memory for a new Text Structure\n"
                                    "ServerThreadHandles.c - newText allocating Failed");
                    continue;
                }

                int bytes_read;
                
                bytes_read = read(cli->s_pipe, newText , sizeof(Text));
                
                if(bytes_read > 0)
                {
                    char topicTitle[MAXTITLELEN];
                    bytes_read = read(cli->s_pipe, topicTitle, sizeof(char) * MAXTITLELEN);
                    
                    if(bytes_read > 0)
                    {
                        pthread_mutex_unlock(&cli->pipe_lock);
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
                {
                    free(newText);
                    pthread_mutex_unlock(&cli->pipe_lock);
                }
            }
        }
        else
            pthread_mutex_unlock(&cli->pipe_lock);
    }

    pthread_exit((void*) NULL);
}

void* verifyMessagesHandler(void* arg)
{
    signal(SIGINT, ThreadKill);
    
    int* pipes_fd = (int*) arg;
    
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
            sendToClients();
            continue;
        }
        
        int existing_id = searchForTopic(newText->topic->title);

        //this should be impossible, but I have to test every
        //angle possible and prevent errors.
        //This is a multi-threaded application, errors are fatal
        if(existing_id < 1)
        {
            pTopic topic_it;

            pthread_mutex_lock(&temp_text_lock);
            for(topic_it = topicList; topic_it->next != NULL; topic_it = topic_it->next);

            topic_it->next = newText->topic;
            newText->topic->prev = topic_it;
            
            topic_it = topic_it->next;
            
            topic_it->TextStart = newText;
            topic_it->id = topic_it->prev->id + 1;
            
            pthread_mutex_unlock(&temp_text_lock);
            pthread_mutex_unlock(&topic_lock);
            sendToClients();
            continue;
        }
        else //Add to global linked list
        {
            pTopic topic_it;
            
            for(topic_it = topicList; topic_it->id !=  existing_id && topic_it->next != NULL; topic_it = topic_it->next);
            
            //Something went wrong, but that's ok
            if(existing_id != topic_it->id && topic_it->next == NULL)
            {
                topic_it->next = newText->topic;
                newText->topic->prev = topic_it;
                newText->topic->id = topic_it->id + 1;
                newText->topic->TextStart = newText;
            }
            else
            {
                pTopic free_topic_aux = newText->topic;
                newText->topic = topic_it;
                free(free_topic_aux);
                
                pText text_it;
                
                for(text_it = topic_it->TextStart; text_it->next != NULL; text_it = text_it->next);
                
                text_it->next = newText;
                newText->prev = text_it;
            }
        }
        pthread_mutex_unlock(&topic_lock);
        sendToClients();
    }
    pthread_exit((void*) NULL);
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
        aux = aux->next;
    }
    
    return -1;
}

void* keepAliveThreadHandler(void* arg)
{
    signal(SIGINT, ThreadKill);
    
    pClient KeepAlive_Client = (pClient) arg;
    
    //How?
    if(KeepAlive_Client == NULL)
    {
        pthread_exit((void*) NULL);
    }
    
    fd_set fds;
    struct timeval t;
    int KeepAlive_Timeout = 0;
    
    pthread_mutex_lock(&KeepAlive_Client->pipe_lock);
    kill(KeepAlive_Client->c_PID, SIGUSR2);
    
    while(!Exit && !KeepAlive_Client->Disconnect)
    {
        if(KeepAlive_Timeout >= 5) //Client timed out
        {
            removeClient(KeepAlive_Client);
            ThreadKill(SIGINT); //Just in Case
        }
        
        KeepAlive_Timeout++;
        
        FD_ZERO(&fds);
        FD_SET(KeepAlive_Client->s_pipe, &fds); //Set pipe into listening mode
        
        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds
        
        if(select(KeepAlive_Client->s_pipe + 1, &fds, NULL, NULL, &t) > 0) //if > 0, something has been read
        { 
            if(FD_ISSET(KeepAlive_Client->s_pipe, &fds)) //Confirm that pipe was read
            {
                char kab; //Keep Alive Buffer
                int bytes_read = read(KeepAlive_Client->s_pipe, &kab, sizeof(char)); //I don't care what was read
                
                if(bytes_read > 0) //all good!
                {
                    pthread_mutex_unlock(&KeepAlive_Client->pipe_lock);
                    sleep(10);
                    KeepAlive_Timeout = 0;
                    
                    pthread_mutex_lock(&KeepAlive_Client->pipe_lock);
                    kill(KeepAlive_Client->c_PID, SIGUSR2);
                }
                else
                {
                    removeClient(KeepAlive_Client);
                    pthread_exit((void*) NULL);
                }
            }
        }
    }
    
    ThreadKill(SIGINT);
}

void* textCountdownHandler(void* arg)
{
    while(!Exit) //thread cycle
    {
        if(topicList == NULL)
            continue;

        for(pTopic topic_it = topicList; topic_it != NULL;) //topic cycle
        {
            for(pText text_it = topic_it->TextStart; text_it != NULL;) //text cycle
            {
                text_it->duration--;

                if(text_it->duration == 0)
                {
                    if(text_it->next == NULL && text_it->prev == NULL)
                    {
                        free(text_it);
                        topic_it->TextStart = NULL;
                    }
                    else
                    {
                        pText Next, Prev;
                        
                        Next = text_it->next;
                        Prev = text_it->prev;
                        
                        if(Next != NULL)
                            Next->prev = Prev;
                        if(Prev != NULL)
                            Prev->next = Next;
                        
                        pText aux = text_it;
                        text_it = Next;
                        free(aux);
                    }
                    continue;
                }
                
                text_it = text_it;
                text_it = text_it->next;
            }
            
            topic_it = topic_it->next;
        }
        sleep(1);
    }
    pthread_exit((void*) NULL);
}