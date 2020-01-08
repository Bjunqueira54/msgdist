#include "client.h"

pClient addNewClient(pClient listStart, pClient newClient)
{
    
    if(listStart == NULL)
    {
        pthread_mutex_lock(&client_lock);   //Lock semaphore
        newClient->next = newClient->prev = NULL;
        listStart = newClient;
        pthread_mutex_unlock(&client_lock); //Unlock semaphore
        return listStart;
    }
    
    pClient aux = listStart;

    pthread_mutex_lock(&client_lock); //Lock semaphore
    
    if(aux->next == NULL)
        aux->next = newClient;
    else
    {
        int subfix = 1; //1
        
        while(aux->next != NULL)
        {
            if(strcmp(aux->username, newClient->username) == 0) //test existing name
            {
                char newUsername[MAXUSERLEN]; //2
                strcpy(newUsername, newClient->username); //3
                memset(newClient->username, 0, MAXUSERLEN); //4
                snprintf(newClient->username, MAXUSERLEN, "%s%d", newClient->username, subfix);
                aux = listStart; //5
                subfix++; //6
                continue; //7
            }

            aux = aux->next;
        }

        aux->next = newClient;
    }
    
    pthread_mutex_unlock(&client_lock); //Unlock semaphore
    
    return listStart;
}

pClient populateClientStruct(pClient newClient)
{
    char pipe_name[15], pipe_path[50];
    
    if(newClient->c_PID == 0)
        return NULL;
    
    ////////////////////////////
    ///Create FIFO pipe first///
    ////////////////////////////

    snprintf(pipe_name, 15, PIPE_SV, newClient->c_PID);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    if(mkfifo(pipe_path, 0600) == -1) //Creation error
    {
        fprintf(stderr, "Error while creating Client fifo {%s}\n", pipe_name);
        return NULL;
    }
    
    ////////////////////////////////
    ///Open Client read pipe next///
    ////////////////////////////////
    
    memset(pipe_name, 0, sizeof(char) * 15);
    memset(pipe_path, 0, sizeof(char) * 50);
    
    snprintf(pipe_name, 15, PIPE_CL, newClient->c_PID);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    newClient->c_pipe = open(pipe_path, O_WRONLY);
    
    if(newClient->c_pipe == -1) //Opening error
    {
        fprintf(stderr, "Error while opening Server fifo {%s}\nError: %s\n", pipe_name, strerror(errno));
        kill(SIGKILL, newClient->c_PID);
        
        free(newClient);
        
        return NULL;
    }
    
    ////////////////////////////////
    ///Open server read pipe last///
    ////////////////////////////////
    
    memset(pipe_name, 0, sizeof(char) * 15);
    memset(pipe_path, 0, sizeof(char) * 50);

    snprintf(pipe_name, 15, PIPE_SV, newClient->c_PID);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    newClient->s_pipe = open(pipe_path, O_RDONLY);
    
    if(newClient->s_pipe == -1) //Opening error
    {
        fprintf(stderr, "Error while opening fifo {%s}\nError: %s\n", pipe_name, strerror(errno));
        return NULL;
    }

    pthread_mutex_init(&newClient->pipe_lock, NULL);
    pthread_create(&newClient->KeepAliveThread, NULL, keepAliveThreadHandler, (void*) newClient);
    pthread_create(&newClient->c_thread, NULL, newMessageThreadHandler, (void*) newClient);

    newClient->next = NULL;
    newClient->prev = NULL;

    return newClient;
}

void removeClient(pClient cli)
{
    if(cli == NULL)
        return;
    else
    {
        pClient Next, Prev;
        
        if(cli->next == NULL)
            Next = NULL;
        else
            Next == cli->next;
        
        if(cli->prev == NULL)
            Prev = NULL;
        else
            Prev = cli->prev;
        
        if(Prev != NULL)
            Prev->next = Next;
        if(Next != NULL)
            Next->prev = Prev;
        
        pthread_kill(cli->c_thread, SIGINT);
        pthread_join(cli->c_thread, NULL);
        
        pthread_kill(cli->KeepAliveThread, SIGINT);
        pthread_join(cli->KeepAliveThread, NULL);
        
        pthread_mutex_unlock(&cli->pipe_lock);
        pthread_mutex_unlock(&cli->pipe_lock);
        pthread_mutex_destroy(&cli->pipe_lock);
        
        close(cli->c_pipe);
        close(cli->s_pipe);
        
        free(cli);
    }
}

pClient findClientByUsername(pClient listStart, char* username)
{
    if(listStart == NULL)
        return NULL;
    
    pClient aux = listStart;
    
    do
    {
        if(strcmp(aux->username, username) == 0)
            return aux;
    }
    while(aux->next != NULL);
    
    return NULL;
}

pClient findClientByPID(pClient listStart, pid_t PID)
{
    if(listStart == NULL)
        return NULL;
    
    pClient aux = listStart;
    
    do
    {
        if(aux->c_PID == PID)
            return aux;
    }
    while(aux->next != NULL);
    
    return NULL;
}

void serverBroadcastExit(pClient listStart)
{
    if(listStart == NULL)
        return;
    
    pClient aux = listStart;
    
    do
    {
        kill(aux->c_PID, SIGINT);
    }
    while(aux->next != NULL);
}
