#ifndef CLIENTHEADER_H
#define CLIENTHEADER_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>
#include <ncurses.h>

#include "../typedefines.h"

#include "../msgdist_defaults.h"
#include "../topic.h"
#include "../text.h"

#include "Functions.h"
#include "SigHandlers.h"
#include "ThreadHandlers.h"
#include "PipeFunctions.h"

    extern bool Exit;
    extern int client_read_pipe;
    extern int server_write_pipe;

#ifdef __cplusplus
}
#endif

#endif