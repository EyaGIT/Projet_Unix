#ifndef COMMON_H
#define COMMON_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>



typedef struct

{
    int client_id;
    int op;
    char buff[4096];
    struct timespec client_connection_time;
} msg;

#endif

