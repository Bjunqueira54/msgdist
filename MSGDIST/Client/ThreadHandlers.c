#include "ThreadHandlers.h"

void threadKill(int sig_num)
{
    Exit = true;
    pthread_exit((void*) NULL);
}

void* receiveTopicList(void* arg)
{
    signal(SIGINT, threadKill);
    signal(SIGUSR2, SIGUSR2_Handler);
    
    fd_set fds;
    struct timeval t;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(client_read_pipe, &fds);

        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds

        if(select(client_read_pipe + 1, &fds, NULL, NULL, &t) > 0)
        {
            if(FD_ISSET(client_read_pipe, &fds))
            {
                pthread_mutex_lock(&mlock);

                pTopic newTopic = malloc(sizeof(Topic));

                if(read(client_read_pipe, newTopic, sizeof(Topic)) > 0)
                {
                    if(topicList == NULL)
                    {
                        topicList = newTopic;
                        topicList->next = NULL;
                        topicList->prev = NULL;
                        topicList->id = 1;
                    }
                    else
                    {
                        pTopic topic_it;
                        for(topic_it = topicList; topic_it->next != NULL;)
                            topic_it = topic_it->next;
                        
                        topic_it->next = newTopic;
                        newTopic->prev = topic_it;
                        newTopic->id = topic_it->id + 1;
                    }
                }
                else
                {
                    free(newTopic);
                }
            
                pthread_mutex_unlock(&mlock);
            }
        }

    }

    threadKill(SIGINT);
}