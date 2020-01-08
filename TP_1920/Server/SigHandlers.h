#ifndef SIGHANDLERS_H
#define SIGHANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"
    
    void SIGUSR1_Handler(int sigNum, siginfo_t* info, void* extra);
    void SIGALRM_Handler(int sigNum, siginfo_t* info, void* extra);
    
#ifdef __cplusplus
}
#endif

#endif