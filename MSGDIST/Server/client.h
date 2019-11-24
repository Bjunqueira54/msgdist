#ifndef CLIENTS_H
#define CLIENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"

    struct client
    {
        char username[MAXUSERLEN];
        
        pid_t c_PID;
        pthread_t c_thread;
        int c_pipe;
        
        pClient next;
        pClient prev;
    };
    
    void addNewClient(pClient listStart, pClient newClient);
    void removeClient(pClient client);
    pClient findClientByUsername(pClient listStart, char* username);
    pClient findClientByPID(pClient listStart, pid_t PID);
    void serverBroadcastExit(pClient listStart);
    void clientSignals(int, siginfo_t*, void*);
    void getClientPid(int, siginfo_t*, void*);
    
#ifdef __cplusplus
}
#endif

#endif
