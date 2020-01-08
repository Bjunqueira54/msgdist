#ifndef SIGHANDLERS_H
#define SIGHANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "clientHeader.h"
    
    void SIGINT_Handler(int);
    void SIGUSR1_Handler(int signal, siginfo_t* info, void* extra);
    void SIGALRM_Handler(int, siginfo_t *, void*);
    
#ifdef __cplusplus
}
#endif

#endif