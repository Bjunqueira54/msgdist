#ifndef SERVERTHREADHANDLES_H
#define SERVERTHREADHANDLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"

    void ThreadKill(int);
    void* newClientThreadHandler(void*);
    void* awaitClientHandler(void*);
    void* verifyMessagesHandler(void*);
    
    void* keepAliveThreadHandler(void*);
    
#ifdef __cplusplus
}
#endif

#endif