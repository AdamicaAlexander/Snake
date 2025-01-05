#include "server.h"

int server_fd = -1;
bool server_running = true;
pthread_t client_threads[MAX_CLIENTS];
int client_count = 0;

void start_server() {
    server_fd = create_server_socket();

    printf("Server started. Waiting for clients...\n");

    while (server_running) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            perror("Error accepting client connection");
            continue;
        }

        if (client_count >= MAX_CLIENTS) {
            printf("Maximum clients reached. Connection rejected.\n");
            close(client_fd);
            continue;
        }

        if (pthread_create(&client_threads[client_count], NULL, handle_client, (void *)(long)client_fd) != 0) {
            perror("Error creating client thread");
            close(client_fd);
            continue;
        }

        printf("Client connected. Assigned to thread %d.\n", client_count + 1);
        client_count++;
    }

    stop_server();
}

void stop_server() {
    printf("Stopping server...\n");
    server_running = false;

    if (server_fd >= 0) {
        close(server_fd);
    }

    for (int i = 0; i < client_count; i++) {
        pthread_join(client_threads[i], NULL);
    }

    printf("Server stopped.\n");
}

void *handle_client(void *arg) {
    int client_fd = (int)(long)arg;
    char buffer[BUFFER_SIZE];

    printf("Client thread started.\n");

    while (server_running) {
        receive_message(client_fd, buffer, BUFFER_SIZE);
        printf("Received from client: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Client requested to disconnect.\n");
            break;
        }

        send_message(client_fd, "Message received.");
    }

    printf("Closing connection with client.\n");
    close(client_fd);
    pthread_exit(NULL);
}
