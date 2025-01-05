#ifndef IPC_H
#define IPC_H

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/snake_game_socket"
#define BUFFER_SIZE 1024

int create_server_socket();
int create_client_socket();
void send_message(int socket_fd, const char *message);
void receive_message(int socket_fd, char *buffer, size_t buffer_size);

#endif // IPC_H
