#include "include/ipc.h"

int create_server_socket() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Socket listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server socket created and listening at %s\n", SOCKET_PATH);
    return server_fd;
}

int create_client_socket() {
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (client_fd < 0) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client socket connected to server at %s\n", SOCKET_PATH);
    return client_fd;
}

void send_message(int socket_fd, const char *message) {
    if (send(socket_fd, message, strlen(message), 0) < 0) {
        perror("Message send failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void receive_message(int socket_fd, char *buffer, size_t buffer_size) {
    memset(buffer, 0, buffer_size);
    ssize_t bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);

    if (bytes_received < 0) {
        perror("Message receive failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0';
}
