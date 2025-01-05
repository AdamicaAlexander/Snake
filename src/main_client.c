#include "ipc.h"

int main() {
    int client_fd = create_client_socket();
    char buffer[BUFFER_SIZE];

    send_message(client_fd, "Hello, server!");
    receive_message(client_fd, buffer, BUFFER_SIZE);
    printf("Server replied: %s\n", buffer);

    send_message(client_fd, "exit");
    close(client_fd);
    return 0;
}
