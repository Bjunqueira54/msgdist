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
        
        bool Disconnect;
        
        pthread_t c_thread;
        pthread_t KeepAliveThread;
        
        pthread_mutex_t pipe_lock;
        
        int c_pipe; //Client Read - Server Write
        int s_pipe; //Server Read - Client Write
        
        pClient next;
        pClient prev;
    };
    
    pClient addNewClient(pClient, pClient);
    pClient populateClientStruct(pClient);
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
