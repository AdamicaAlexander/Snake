#include "client.h"

int server_fd = -1;
bool client_running = true;
pthread_t input_thread, listener_thread;

void start_client() {
    server_fd = create_client_socket();

    printf("Client started. Connected to server.\n");

    if (pthread_create(&input_thread, NULL, input_handler, NULL) != 0) {
        perror("Error creating input thread");
        stop_client();
        return;
    }

    if (pthread_create(&listener_thread, NULL, server_listener, NULL) != 0) {
        perror("Error creating listener thread");
        stop_client();
        return;
    }

    pthread_join(input_thread, NULL);
    pthread_join(listener_thread, NULL);

    stop_client();
}

void stop_client() {
    printf("Stopping client...\n");
    client_running = false;

    if (server_fd >= 0) {
        close(server_fd);
    }

    printf("Client stopped.\n");
}

void *input_handler(void *arg) {
    char user_input[MAX_USER_INPUT];

    while (client_running) {
        printf("Enter command: ");
		
        if (fgets(user_input, MAX_USER_INPUT, stdin) == NULL) {
            printf("Error reading input.\n");
            break;
        }

        user_input[strcspn(user_input, "\n")] = '\0';
        send_message(server_fd, user_input);

        if (strcmp(user_input, "exit") == 0) {
            printf("Exiting client...\n");
            client_running = false;
            break;
        }
    }

    pthread_exit(NULL);
}

void *server_listener(void *arg) {
    char buffer[BUFFER_SIZE];

    while (client_running) {
        receive_message(server_fd, buffer, BUFFER_SIZE);
        printf("Server: %s\n", buffer);
    }

    pthread_exit(NULL);
}
