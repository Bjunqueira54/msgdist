#ifndef CLIENTS_H
#define CLIENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "serverHeader.h"
#include "ServerThreadHandles.h"

    struct client
    {
        char username[MAXUSERLEN];
        
        pid_t c_PID;
        pthread_t c_thread;
        int c_pipe; //Client Read - Server Write
        int s_pipe; //Server Read - Client Write
        
        pClient next;
        pClient prev;
    };
    
    pClient addNewClient(pClient, pClient);
    pClient createNewClientPipes(pClient);
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
