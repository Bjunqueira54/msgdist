#ifndef SERVERTHREADHANDLES_H
#define SERVERTHREADHANDLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"

    void ThreadKill(int);
    void* newClientThreadHandler(void*);
    void* newMessageThreadHandler(void*);
    void* verifyMessagesHandler(void*);
    
    void* keepAliveThreadHandler(void*);

    void* textCountdownHandler(void* arg);
    
#ifdef __cplusplus
}
#endif

#endif