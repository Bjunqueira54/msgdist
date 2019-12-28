#include "client.h"

void addNewClient(pClient listStart, pClient newClient)
{
    if(listStart == NULL)
    {
        newClient->next = newClient->prev = NULL;
        listStart = newClient;
        return;
    }
    
    pClient aux = listStart;

    if(aux->next == NULL)
        aux->next = newClient;
    else
    {
        while(aux->next != NULL)
            aux = aux->next;

        aux->next = newClient;
    }
}

pClient createNewClient(pid_t client_pid)
{
    pClient newClient = calloc(1, sizeof(Client));
    if(newClient == NULL) {
        fprintf(stderr, "Error allocating memory for new client\n");
        return NULL;
    }
    
    newClient->c_PID = client_pid;

    char pipe_name[10], pipe_path[50];

    snprintf(pipe_name, 10, PIPE_SV, client_pid);
    snprintf(pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name);

    if(mkfifo(pipe_path, 0600) == -1) //erro de cricao
    {
        fprintf(stderr, "Error while creating fifo {%s}\n", pipe_name);
        return NULL;
    }

    newClient->c_pipe = open(pipe_path, O_RDWR);
    if(newClient->c_pipe == -1) { //erro a abrir
        fprintf(stderr, "Error while opening fifo {%s}\n", pipe_name);
        return NULL;
    }

    pthread_create(&newClient->c_thread, NULL, awaitClientHandler, (void*) newClient);

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