#include "client.h"

void addNewClient(pClient clientList, pClient newClient)
{
    if(clientList == NULL)
    {
        newClient->next = newClient->prev = NULL;
        clientList = newClient;
        return;
    }
    
    pClient aux = clientList;

    if(aux->next == NULL)
        aux->next = newClient;
    else
    {
        while(aux->next != NULL)
            aux = aux->next;

        aux->next = newClient;
    }
}

pClient createNewClientPipes(pClient newClient)
{
    char pipe_name[15], pipe_path[50];
    
    //Create and Open server read pipe first.

    snprintf(pipe_name, 15, PIPE_SV, newClient->c_PID);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    if(mkfifo(pipe_path, 0600) == -1) //Creation error
    {
        fprintf(stderr, "Error while creating fifo {%s}\n", pipe_name);
        return NULL;
    }

    newClient->s_pipe = open(pipe_path, O_RDONLY);
    
    if(newClient->c_pipe == -1) //Opening error
    {
        fprintf(stderr, "Error while opening fifo {%s}\n", pipe_name);
        return NULL;
    }
    
    //Create and Open client write pipe second.

    snprintf(pipe_name, 15, PIPE_CL, newClient->c_PID);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    if(mkfifo(pipe_path, 0600) == -1) //Creation error
    {
        fprintf(stderr, "Error while creating fifo {%s}\n", pipe_name);
        return NULL;
    }

    newClient->c_pipe = open(pipe_path, O_WRONLY);
    
    if(newClient->c_pipe == -1) //Opening error
    {
        fprintf(stderr, "Error while opening fifo {%s}\n", pipe_name);
        return NULL;
    }

    //pthread_create(&newClient->c_thread, NULL, awaitClientHandler, (void*) newClient); <-- Need to create Function for i-Client Thread

    newClient->next = NULL;
    newClient->prev = NULL;

    return newClient;
}

void* awaitClientHandler(void* data)
{
    pClient cli = (pClient) data;
    fd_set fds;
    struct timeval t;

    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(cli->c_pipe, &fds); //colocar o pipe para escuta
        t.tv_sec = 1; //segundos
        t.tv_usec = 0; //micro segundos

        if(select(cli->c_pipe + 1, &fds, NULL, NULL, &t) > 0) //select positivo -> leu algo
        {
            if(FD_ISSET(cli->c_pipe, &fds)) //confirmar que select leu do pipe
            {
		pText newText = malloc(sizeof(Text));

                int n = read(cli->c_pipe, newText->title, sizeof(newText->title));
                
                if(n > 0)
                    newText->title[n-1] = '\0';
            }
        }
    }
}

void removeClient(pClient client)
{
    if(client == NULL || (client->next == NULL && client->prev == NULL))
        return;
    else
    {
        pClient Next, Prev;
        
        if(client->next == NULL)
            Next = NULL;
        else
            Next == client->next;
        
        if(client->prev == NULL)
            Prev = NULL;
        else
            Prev = client->prev;
        
        if(Next == NULL && Prev == NULL) //Something's wrong!
            return;
        
        Prev->next = Next;
        Next->prev = Prev;
        
        free(client);
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