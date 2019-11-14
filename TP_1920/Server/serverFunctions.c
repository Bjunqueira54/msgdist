#include "serverHeader.h"

int server_file;

void initializeVerifier(int *servPipe, int *veriPipe)
{
    childPID = fork();
    if(childPID == 0) {
        dup2(veriPipe[1], 1);
        close(veriPipe[1]);
        dup2(servPipe[0],0);
        close(servPipe[0]);

        execlp("./verificador", "verificador", BADWORDS, NULL);

        fprintf(stderr, "\nErro! Impossivel executar programa\n");
        exit(0); 
    }
}

void serverMainLoop(char *cmd, pClient aux)
{
    if(stringCompare(cmd, "shutdown")) {
        union sigval value;
        value.sival_int = 0;

        while(aux != NULL)
        {
            sigqueue(aux->c_PID, SIGINT, value);
            aux = aux->next;
        }

        Exit = true;
        return;
    }
    else
        if (parseCommands(cmd))
            return;
        else
            serverMainOutput(2);
}

bool parseCommands(char cmd[])
{
    if(stringCompare(cmd, "help")) {
        serverMainOutput(3);
        return true;
    }
    else if (stringCompare(cmd, "msg") ) {
        listAllMesages();
        return true;
    }
    else if (stringCompare(cmd, "users")) {
        listAllUsers();
        return true;        
    }
    else if (stringCompare(cmd, "topics")) {
        listAllTopics();
        return true;
    }
    else if (stringCompare(cmd, "prune")) {
        deleteEmptyTopics();
        return true;
    }
    else if (stringCompare(cmd, "filter on")) {
        if (Filter == false)
            Filter = true;
        return true;
    }
    else if (stringCompare(cmd, "filter off")) {
        if (Filter == true)
            Filter = false;
        return true;
    }
    else
        if(parseOptionCommands(cmd))
            return true;
        
    return false;
}

bool parseOptionCommands(char cmd[])
{
    int i = 0;
    char *options[2], *opt = strtok(cmd, " ");
    
    while(opt != NULL) {
        options[i++] = opt;
        opt = strtok(NULL, " ");
    }
    
    switch(getopt(i, options, "m:t:u:")) {
        case 'm':
            if(stringCompare(options[0],"del")) {
                opt = optarg; //nome
                return true;
            }
            return false;
        case 't':
            if(stringCompare(options[0],"topic")) {
                opt = optarg; //nome
                return true;
            }
            return false;
        case 'u':
            if(stringCompare(options[0],"kick")) {
                opt = optarg; //nome
                return true;
            }
            return false;
        default:
            return false;
    }

    return false;
}

bool stringCompare(char *str1, char *str2) //TEM UM BUG
{
    if(strlen(str1) <= 1)
        return false;
        
    for (int i = 0; i < strlen(str1) - 1; i++)
        if(str1[i] != str2[i])
            return false;
    return true;
}

void listAllUsers() 
{
    printf("users recognized\n");
}

void listAllMesages()
{
    printf("msg recognized\n");
}

void listAllTopics()
{
    printf("topics recognized\n");
}

void deleteEmptyTopics() 
{
    printf("prune recognized\n");
}

bool verifyNewMessage(int *servPipe, int *veriPipe) { //recebe sinal do cliente que meteu uma nova mensagem. esta funcao cuida de verificar
    //recebe mensagem do pipe
    char word[20];
    
    do {
        printf("\nPalavra: ");
        scanf("%s", word);

        write(servPipe[1], word, strlen(word));
        printf("<GESTOR> sent\n");
        
    } while(strcmp(word,"##MSGEND##"));
    
    printf("<GESTOR> waiting\n");
    read(veriPipe[0], word, strlen(word));
    printf("\n%d", atoi(word));
    
}

int createServerFiles()
{
    struct stat tmpstat = {0};
    
    if(stat(MSGDIST_DIR, &tmpstat) == -1)
    {
        if(mkdir(MSGDIST_DIR, 0744) == -1)
        {
            printf("Directory Creation: %d\n", errno);
            return -1;
        }
    }
    
    server_file = open(SERVER_PID, O_RDWR);
    
    if(server_file != -1)
    {
        fprintf(stderr, "Server already running\n");
        exit (EXIT_FAILURE);
    }
    
    server_file = open(SERVER_PID, O_RDWR | O_CREAT, 0644);
    
    if(server_file == -1)
    {
        printf("File Creation: %d\n", errno);
        return -1;
    }
    
    pid_t self_pid = getpid();
    
    char pid[6];
    
    snprintf(pid, sizeof(char)*6, "%d", self_pid);
    
    write(server_file, pid, strlen(pid));

    return 0;
}

int deleteServerFiles() 
{    
    if (remove(SERVER_PID) != 0) {
        fprintf(stderr, "Erro ao apagar o ficheiro\n"); 
        return -1;
    }
    return 0; 
}