#include "ThreadHandlers.h"

void threadKill(int sig_num)
{
    pthread_exit((void*) NULL);
}

void* receiveTopicList(void* list)
{
    signal(SIGINT, threadKill);

    pTopic topics = (pTopic) list;
    fd_set fds;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(ServerPipe, &fds);

        if(select(ServerPipe + 1, &fds, NULL, NULL, NULL) > 0);
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