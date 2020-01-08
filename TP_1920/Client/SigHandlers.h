#ifndef SIGHANDLERS_H
#define SIGHANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "clientHeader.h"
    
    void SIGINT_Handler(int);
    void SIGUSR1_Handler(int, siginfo_t*, void*);
    void SIGUSR2_Handler(int);
    
#ifdef __cplusplus
}
#endif

#endif