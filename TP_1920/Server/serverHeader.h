#ifndef SERVERHEADER_H
#define SERVERHEADER_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include "../typedefines.h" //MUST BE INCLUDED FIRST
typedef struct client Client, *pClient;

#include "../msgdist_defaults.h"
#include "../topic.h"
#include "../text.h"
#include "client.h"
#include "ServerThreadHandles.h"
#include "SigHandlers.h"

#define BADWORDS "badwords.txt"

extern int maxMessages, server_file;
extern bool Exit, Filter;
extern pid_t childPID;
extern pText textList;
extern pTopic topicList;
extern pClient clientList;
extern pthread_mutex_t client_lock, temp_text_lock, topic_lock, text_lock;

pid_t initializeVerifier(int* , int *);
void serverMainLoop(char*);
bool stringCompare(char *, char *);

void* receiveMsgHandler(void*);
void sendMsgToVerifier();
void receiveClientMsg();
void addNewMessage(pText, pText);
int countMsgs(pText);
void sendToClients();
void deleteMsg(pTopic);

int sendTextToVerifier(int, int, pText);

void listAllTopics(pTopic);
void listAllUsers(pClient);
void listAllMesages(pTopic);
void deleteEmptyTopics(pTopic);
void killAllClients(pClient);
void purgeClients();

int deleteServerFiles();
int createServerFiles();

/*Custom made string parser.\n
 * Usage: Takes *string* (Null-Terminated) as an argument and\n
 * attempts to break it down into all of it's words.
 * SPACE is the only accepted delimiter. Any other symbols
 * will be counted towards the total number of words.\n
 * Returns: 2-dimension array with words+1 lines
 * and each line is occupied by a Null-terminated
 * word of *string*.*/
char** stringParser(const char* string);

#ifdef __cplusplus
}
#endif

#endif
