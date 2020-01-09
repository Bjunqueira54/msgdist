#include "SigHandlers.h"

void SIGALRM_Handler(int sigNum, siginfo_t *info, void *extra)
{
    
}

void SIGUSR1_Handler(int sigNum, siginfo_t* info, void* extra)
{
    union sigval val = (union sigval) extra;
    sigqueue(info->si_pid, SIGUSR1, val); 

    //Find client pipe
    pClient aux_c = clientList;

    while(aux_c->next != NULL)
    {
        if(aux_c->c_PID == info->si_pid)
            break;
        aux_c = aux_c->next;
    }
    
    //Find text list
    pTopic aux_t = topicList;

    while(aux_t->next != NULL)
    {
        if(aux_t->id == info->si_value.sival_int)
            break;
        aux_t = aux_t->next;
    }

    write(aux_c->s_pipe, aux_t->TextStart, sizeof(pText));
}