#include "ThreadHandlers.h"

// pthread_mutex_t topicLock;

void threadKill(int sig_num)
{
    pthread_exit((void*) NULL);
}

void* receiveTopicList(void* list)
{
    signal(SIGINT, threadKill);

    pTopic topics = (pTopic) list;
    fd_set fds;
    struct timeval timeout;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(ServerPipe, &fds);

        timeout.tv_sec = 1; //seconds
        timeout.tv_usec = 0; //micro-seconds

        if(select(ServerPipe + 1, &fds, NULL, NULL, &timeout) > 0);
        {
            if(FD_ISSET(ServerPipe, &fds))
            {
                pthread_mutex_lock(&topicLock);

                if(read(ServerPipe, topics, sizeof(pTopic)) == 0);
                    topics = NULL;

                pthread_mutex_unlock(&topicLock);
            }
        }
    }
    
    threadKill(SIGINT);
}