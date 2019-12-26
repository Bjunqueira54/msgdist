#include "serverInterface.h"

void serverMainOutput(int flag)
{
    switch(flag)
    {
        case 0: // Command
            fprintf(stdout, "\nCommand: ");
            return;
        case 1: // Server Shutdown
            fprintf(stdout, "\nServer will shutdown\n");
            return;
        case 2: // Invalid Command
            fprintf(stdout, "Invalid command\n");
            return;
        case 3: // Help
            fprintf(stdout, "     shutdown - Server shutdown\n");
            fprintf(stdout, "        users - List all users\n");
            fprintf(stdout, "  kick <user> - Exclude a user\n");
            fprintf(stdout, "          msg - List all messages\n");
            fprintf(stdout, "    del <msg> - Delete a message\n");
            fprintf(stdout, "       topics - List all topics\n");
            fprintf(stdout, "topic <topic> - List messages in topic\n");
            fprintf(stdout, "        prune - Delete empty topics\n");
            return;
        case 4:
            fprintf(stdout, "\nVerifier shuting down\n");
            return;
        default:
            fprintf(stderr, "Error\n");
            return;
    }
}
