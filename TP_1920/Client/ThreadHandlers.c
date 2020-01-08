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

void* UserInputThreadHandler(void* arg)
{
    signal(SIGINT, threadKill);
    signal(SIGUSR2, SIGUSR2_Handler);
    
    pTopic TopicList = (pTopic) arg;
    int current_topic_id;
    
    //Just in case. There were instances of this thread accessing
    //stdscr before the main thread and printing out garbage.
    refresh();
    
    while(!Exit)
    {
        PrintMenu(TopicList);
        
        switch(tolower(getch()))
        {
            case 'n':
            {
                if(current_topic_id == 0)
                    newMessage(TopicList);
                else
                {
                    //How can current_topic_id not be 0 and this be NULL?
                    //You know what they say: Better safe than SegmentationFault
                    if(TopicList == NULL)
                        newMessage(TopicList);
                    else
                    {
                        pTopic aux = TopicList;
                        
                        while(aux != NULL)
                        {
                            if(aux->id == current_topic_id)
                                newTopicMessage(aux->title);
                        }
                    }
                }
                break;
            }
            case KEY_UP:
                clear();
                mvwaddstr(stdscr, 5, 5, "I pressed Arrow Up");
                refresh();
                break;
            case KEY_DOWN:
                clear();
                mvwaddstr(stdscr, 5, 5, "I pressed Arrow Down");
                refresh();
                break;
            case KEY_RIGHT: //KEY_RIGHT and KEY_LEFT aren't used right now
                clear();
                mvwaddstr(stdscr, 5, 5, "I pressed Arrow Right");
                refresh();
                break;
            case KEY_LEFT:  //but it won't hurt to have them here just in case.
                clear();
                mvwaddstr(stdscr, 5, 5, "I pressed Arrow Left");
                refresh();
                break;
            case 10:
                clear();
                mvwaddstr(stdscr, 5, 5, "I pressed enter");
                refresh();
                break;
            case 27:
                Exit = true;
                threadKill(SIGINT);
                break;
            default:
                break;
        }
    }
    
    threadKill(SIGINT);
}