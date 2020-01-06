#include "PipeFunctions.h"

int client_read_pipe;
int server_write_pipe;

void createPipes(const char* username)
{
    pid_t server_pid;
    pid_t self_pid = getpid();
    
    int serverpidfd;
    
    char pid[6]; //server pid string
    char server_main_pipe[25];  //server main pipe path string
    char client_info[MAXUSERLEN + 10] = ""; //client info string

    char pipe_name_temp[15]; //temporary string for pipe names
    char client_main_pipe_path[50]; //client main pipe path string
    char server_main_pipe_path[50]; //server main pipe path string
    
    serverpidfd = open(SERVER_PID, O_RDONLY);

    if(serverpidfd == -1)
    {
        fprintf(stderr, "Server not running!");
        getchar();
        exit (EXIT_FAILURE);
    }
    
    read(serverpidfd, pid, sizeof(char) * 6);
    
    sscanf(pid, "%d", &server_pid);
    
    snprintf(server_main_pipe, 25, "/tmp/msgdist/%d_main", server_pid);
    
    //Open Server Main Pipe and write client info
    
    client_read_pipe = open(server_main_pipe, O_WRONLY);
    
    if(client_read_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nServer Main Pipe open error: %s\n", strerror(errno));
        getchar();
    }
    
    snprintf(client_info, MAXUSERLEN + 10, "%s %d", username, getpid());
    
    write(client_read_pipe, client_info, strlen(client_info));
    
    close(client_read_pipe); //close this server main pipe to save up on File Descriptors;

    //Create and Open Client Pipe first
    
    snprintf(pipe_name_temp, 15, PIPE_CL, self_pid);
    snprintf(client_main_pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name_temp);
    
    if(mkfifo(client_main_pipe_path, 0600) == -1) //Creation error
    {
        fprintf(stderr, "Error while creating fifo {%s}\n", client_main_pipe_path);
        getchar();
        exit(EXIT_FAILURE);
    }
    
    client_read_pipe = open(client_main_pipe_path, O_RDONLY);
    
    if(client_read_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nclient_read_pipe open error: %s\n", strerror(errno));
    }
    
    //open Server Write Pipe first
    
    memset(pipe_name_temp, 0, sizeof(char) * 15);
    
    snprintf(pipe_name_temp, 15, PIPE_SV, self_pid);
    snprintf(server_main_pipe_path, 50, "%s/%s", MSGDIST_DIR, pipe_name_temp);
    
    server_write_pipe = open(server_main_pipe_path, O_WRONLY);
    
    if(server_write_pipe == -1)
    {
        fprintf(stderr, "Something went wrong\nserver_write_pipe open error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void SendTextToServer(char* TopicTitle, pText newText)
{
    //is it really this easy?
    write(server_write_pipe, newText, sizeof(Text));
    write(server_write_pipe, TopicTitle, strlen(TopicTitle));
}