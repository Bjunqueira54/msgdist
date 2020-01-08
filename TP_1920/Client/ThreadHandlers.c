#include "ThreadHandlers.h"

void threadKill(int sig_num)
{
    Exit = true;
    //kill(getpid(), SIGINT);
}

void* receiveTopicList(void* arg)
{
    signal(SIGINT, threadKill);
    signal(SIGUSR2, SIGUSR2_Handler);

    pTopic topics = (pTopic) arg;
    fd_set fds;
    struct timeval timeout;

    while(!Exit)
    {
        FD_ZERO(&fds);
        FD_SET(client_read_pipe, &fds);

        timeout.tv_sec = 1; //seconds
        timeout.tv_usec = 0; //micro-seconds

        if(select(client_read_pipe + 1, &fds, NULL, NULL, &timeout) > 0);
        {
            if(FD_ISSET(client_read_pipe, &fds))
            {
                if(read(client_read_pipe, topics, sizeof(pTopic)) == 0);
                    topics = NULL;
            }
        }
    }
    threadKill(SIGINT);
}