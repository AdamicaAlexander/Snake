#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdbool.h>
#include "ipc.h"

#define MAX_CLIENTS 10

void start_server();
void stop_server();
void *handle_client(void *arg);

#endif // SERVER_H
