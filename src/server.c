#include "server.h"

#define MAX_CLIENT_THREADS 10

pthread_t g_server_logic_thread;
pthread_t g_client_threads[MAX_CLIENT_THREADS];
int g_server_thread_count = 0;
int g_server_running = 1;
static int server_fd = -1;

void *server_logic_thread(void *arg) {
    IPCResources *ipc = (IPCResources *)arg;
    printf("[SERVER] Herná logika spustená.\n");

	time_t start_time = time(NULL);

    while (g_server_running) {
        sem_wait(ipc->sem_game);

        if (ipc->world->game_duration > 0 && ipc->world->elapsed_time >= ipc->world->game_duration) {
            printf("[SERVER] Time limit reached! Game over.\n");
            stop_server(ipc);
            sem_post(ipc->sem_game);
            break;
        }

        move_snakes(ipc->world);

        ipc->world->elapsed_time = (int) (time(NULL) - start_time);

        sem_post(ipc->sem_game);

        usleep(200000);
    }

    printf("[SERVER] Herná logika ukončená.\n");
    return NULL;
}

void *server_client_thread(void *arg) {
    ClientArgs *args = (ClientArgs *)arg;
    int client_fd = args->client_fd;
    int client_id = args->client_id;
    IPCResources *ipc = args->ipc;
    free(args);
    char buffer[BUFFER_SIZE];
    printf("Client thread started.\n");
    Snake *snake = &ipc->world->snakes[client_id];

    while (g_server_running) {
        receive_message(client_fd, buffer, BUFFER_SIZE);
        sem_wait(ipc->sem_game);

        switch (buffer[0]) {
            case 'w': if (snake->direction != DOWN) snake->direction = UP; break;
            case 's': if (snake->direction != UP)   snake->direction = DOWN; break;
            case 'a': if (snake->direction != RIGHT) snake->direction = LEFT; break;
            case 'd': if (snake->direction != LEFT) snake->direction = RIGHT; break;
        }

        if (buffer[0] == 'q') {
            remove_snake(ipc->world, snake);
            printf("Client %d requested to disconnect.\n", client_id);
            close(client_fd);
            break;
        }

        sem_post(ipc->sem_game);
    }

    printf("Closing connection with client.\n");
    close(client_fd);
    pthread_exit(NULL);
}

void stop_server(IPCResources *ipc) {
    printf("[SERVER] Ukončujem...\n");

    g_server_running = 0;

    for (int i = 0; i < g_server_thread_count; i++) {
        pthread_cancel(g_client_threads[i]);
        pthread_join(g_client_threads[i], NULL);
        printf("[SERVER]  server client thread finished.\n");
    }
    pthread_cancel(g_server_logic_thread);
    pthread_join(g_server_logic_thread, NULL);
    printf("[SERVER]  server logic thread finished.\n");
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_CLIENTS);
    close(server_fd);
    cleanup_ipc(ipc);
    _exit(0);
}

void start_server(IPCResources *ipc) {
    signal(SIGINT, server_signal_handler);
    signal(SIGTERM, server_signal_handler);

    server_fd = create_server_socket();

    printf("[SERVER] creating server logic thread.\n");

    if (pthread_create(&g_server_logic_thread, NULL, server_logic_thread, ipc) != 0) {
        perror("[SERVER] Chyba pri vytváraní server_logic_thread");
        exit(1);
    }

    while (g_server_running) {
        sem_wait(ipc->sem_clients);
        if (!g_server_running) break;
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Error accepting client connection");
            continue;
        }

        sem_wait(ipc->sem_game);
        int client_id = ipc->world->num_snakes;

        if (g_server_thread_count >= MAX_CLIENT_THREADS) {
            printf("Maximum clients reached. Connection rejected.\n");
            close(client_fd);
            continue;
        }

        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->client_fd = client_fd;
        args->client_id = client_id;
        args->ipc = ipc;

        spawn_snake(ipc->world, client_id);
		spawn_fruit(ipc->world);
        sem_post(ipc->sem_game);

        printf("[SERVER] creating server client thread.\n");
        if (pthread_create(&g_client_threads[g_server_thread_count], NULL, server_client_thread, (void *)args) != 0) {
            perror("Error creating client thread");
            close(client_fd);
            free(args);
            continue;
        }

        g_server_thread_count++;
    }

    for (int i = 0; i < g_server_thread_count; i++) {
        pthread_join(g_client_threads[i], NULL);
    }

    pthread_join(g_server_logic_thread, NULL);

    printf("[SERVER] Ukončenie servera.\n");

    stop_server(ipc);

    _exit(0);
}
