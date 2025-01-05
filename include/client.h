#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <stdbool.h>
#include "ipc.h"

#define MAX_USER_INPUT 256

void start_client();
void stop_client();
void *input_handler(void *arg);
void *server_listener(void *arg);

#endif // CLIENT_H
