#ifndef IPC_H
#define IPC_H

#include "game_logic.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/game_socket"
#define BUFFER_SIZE 1024

#define SHM_NAME "/game_shm"
#define SEM_NAME "/game_sem"
#define SEM_CLIENTS "/game_clients_sem"

#define SHM_SIZE sizeof(GameWorld)

typedef struct {
    int shm_fd;
    sem_t *sem_game;
    sem_t *sem_clients;
    GameWorld *world;
} IPCResources;

int create_server_socket();
int create_client_socket();
void send_message(int socket_fd, const char *message);
void receive_message(int socket_fd, char *buffer, size_t buffer_size);

int init_ipc(IPCResources *ipc, int create);
void cleanup_ipc(IPCResources *ipc);

void server_signal_handler(int signum);
void client_signal_handler(int signum);

#endif // IPC_H
