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

    pTopic topics = (pTopic) arg;
    fd_set fds;
    struct timeval t;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(client_read_pipe, &fds);

        t.tv_sec = 1; //seconds
        t.tv_usec = 0; //micro-seconds

        if(select(client_read_pipe + 1, &fds, NULL, NULL, &t) > 0);
        {
            if(FD_ISSET(client_read_pipe, &fds))
            {
                pthread_mutex_lock(&mlock);

                if(read(client_read_pipe, topics, sizeof(pTopic)) == 0);
                    topics = NULL;
            
                pthread_mutex_unlock(&mlock);
            }
        }
    }
    
    threadKill(SIGINT);
}