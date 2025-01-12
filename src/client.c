#include "client.h"

static struct termios original_termios;

static pthread_t input_tid, display_tid;
static volatile int running = 1;
static int my_snake_index = -1;
static int server_fd = -1;

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

void setup_terminal() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &original_termios);

    atexit(restore_terminal);

    new_termios = original_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void *client_input_thread(void *arg) {
    char user_input;

    setup_terminal();

    while (running) {
        user_input = getchar();

        if (user_input == 'w' || user_input == 'a' || user_input == 's' || user_input == 'd' || user_input == 'q') {
            send_message(server_fd, &user_input);
        }

        if (user_input == 'q') {
            printf("Exiting client...\n");
            running = false;
            break;
        }
    }

    restore_terminal();

    pthread_exit(NULL);

}


void *client_display_thread(void *arg) {
    IPCResources *ipc = (IPCResources*)arg;

    while (running) {
        sem_wait(ipc->sem_game);
		render_game(ipc->world);
        sem_post(ipc->sem_game);

        usleep(20000);
    }

    return NULL;
}

void stop_client() {
    restore_terminal();
}

void run_client(IPCResources *ipc, bool new_game) {
    signal(SIGINT, client_signal_handler);
    signal(SIGTERM, client_signal_handler);

    server_fd = create_client_socket();

    if (new_game) {
        sem_wait(ipc->sem_game);
        game_menu(ipc->world);
        sem_post(ipc->sem_game);
    }

    setup_terminal();

    sem_wait(ipc->sem_game);
    my_snake_index = ipc->world->num_snakes;
    sem_post(ipc->sem_game);

    sem_post(ipc->sem_clients);

    printf("[CLIENT] creating input thread.\n");
    pthread_create(&input_tid, NULL, client_input_thread, NULL);
    printf("[CLIENT] input thread created.\n");

    printf("[CLIENT] creating display thread.\n");
    pthread_create(&display_tid, NULL, client_display_thread, ipc);
    printf("[CLIENT] display thread created.\n");

    pthread_join(input_tid, NULL);
    printf("[CLIENT] input thread finished.\n");

    pthread_join(display_tid, NULL);
    printf("[CLIENT] display thread finished.\n");

    printf("[CLIENT] Končím.\n");

    stop_client();
}
