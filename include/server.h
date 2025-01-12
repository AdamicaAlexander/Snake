#ifndef SERVER_H
#define SERVER_H

#include "ipc.h"
#include "game_logic.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int client_fd;
    int client_id;
    IPCResources *ipc;
} ClientArgs;

extern pthread_t g_server_logic_thread;
extern pthread_t g_client_threads[];
extern int g_server_thread_count;
extern int g_server_running;

void start_server(IPCResources *ipc);
void *server_logic_thread(void *arg);
void *server_client_thread(void *arg);
void stop_server(IPCResources *ipc);

#endif // SERVER_H
