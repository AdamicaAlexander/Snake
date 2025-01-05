#include "ipc.h"

int main() {
    int client_fd = create_client_socket();
    send_message(client_fd, "Hello, server!");

    char buffer[BUFFER_SIZE];
    receive_message(client_fd, buffer, BUFFER_SIZE);
    printf("Received from server: %s\n", buffer);

    close(client_fd);
    return 0;
}
