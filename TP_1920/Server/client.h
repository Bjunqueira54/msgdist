#ifndef CLIENTS_H
#define CLIENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"

    struct client
    {
        char username[8];
        
        pid_t c_PID;
        pthread_t c_thread;
        int c_pipe;
        
        pClient next;
        pClient prev;
    };
    
    void addNewClient(pClient, pClient);
    pClient createNewClient(pid_t);
    void removeClient(pClient);
    pClient findClientByUsername(pClient, char*);
    pClient findClientByPID(pClient, pid_t);
    void serverBroadcastExit(pClient);
    void clientSignals(int, siginfo_t*, void*);
    void getClientPid(int, siginfo_t*, void*);
    
#ifdef __cplusplus
}
#endif

#endif
