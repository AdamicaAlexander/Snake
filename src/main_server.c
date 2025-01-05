#include "ipc.h"

int main() {
    int server_fd = create_server_socket();
    int client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0) {
        perror("Server accept failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    receive_message(client_fd, buffer, BUFFER_SIZE);
    printf("Received from client: %s\n", buffer);

    send_message(client_fd, "Hello, client!");
    close(client_fd);
    close(server_fd);
    return 0;
}
