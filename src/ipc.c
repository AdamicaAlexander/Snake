#include "ipc.h"

extern pthread_t g_server_logic_thread;
extern pthread_t g_client_threads[];
extern int g_server_thread_count;
extern int g_server_running;

int init_ipc(IPCResources *ipc, int create) {
    if (create) {
        ipc->shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (ipc->shm_fd < 0) {
            perror("[IPC] shm_open (server create)");
            return -1;
        }

        struct stat sb;
        if (fstat(ipc->shm_fd, &sb) == -1) {
            perror("[IPC] fstat failed");
            return -1;
        }

        if (sb.st_size == 0) {
            if (ftruncate(ipc->shm_fd, SHM_SIZE) < 0) {
                perror("[IPC] ftruncate");
                return -1;
            }
        }

        if (ftruncate(ipc->shm_fd, SHM_SIZE) < 0) {
            perror("[IPC] ftruncate");
            return -1;
        }

        ipc->world = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ipc->shm_fd, 0);
        if (ipc->world == MAP_FAILED) {
            perror("[IPC] mmap (server create)");
            return -1;
        }

        ipc->sem_game = sem_open(SEM_NAME, O_CREAT, 0666, 1);
        if (ipc->sem_game == SEM_FAILED) {
            perror("[IPC] sem_open SEM_NAME");
            return -1;
        }

        ipc->sem_clients = sem_open(SEM_CLIENTS, O_CREAT, 0666, 0);
        if (ipc->sem_clients == SEM_FAILED) {
            perror("[IPC] sem_open SEM_CLIENTS");
            return -1;
        }
    } else {
        ipc->shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (ipc->shm_fd < 0) {
            perror("[IPC] shm_open (client open)");
            return -1;
        }

        ipc->world = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ipc->shm_fd, 0);
        if (ipc->world == MAP_FAILED) {
            perror("[IPC] mmap (client open)");
            return -1;
        }

        ipc->sem_game = sem_open(SEM_NAME, 0);
        if (ipc->sem_game == SEM_FAILED) {
            perror("[IPC] sem_open SEM_NAME (client)");
            return -1;
        }

        ipc->sem_clients = sem_open(SEM_CLIENTS, 0);
        if (ipc->sem_clients == SEM_FAILED) {
            perror("[IPC] sem_open SEM_CLIENTS (client)");
            return -1;
        }
    }
    return 0;
}

void cleanup_ipc(IPCResources *ipc) {
    if (ipc->world != NULL && ipc->world != MAP_FAILED) {
        munmap(ipc->world, SHM_SIZE);
        ipc->world = NULL;
    }

    if (ipc->shm_fd >= 0) {
        close(ipc->shm_fd);
        ipc->shm_fd = -1;
    }

    if (ipc->sem_game) {
        sem_close(ipc->sem_game);
        ipc->sem_game = NULL;
    }

    if (ipc->sem_clients) {
        sem_close(ipc->sem_clients);
        ipc->sem_clients = NULL;
    }
}

void server_signal_handler(int signum) {
    printf("[SERVER] Prijatý signál %d. Ukončujem...\n", signum);

    g_server_running = 0;

    for (int i = 0; i < g_server_thread_count; i++) {
        pthread_cancel(g_client_threads[i]);
        pthread_join(g_client_threads[i], NULL);
    }
    pthread_cancel(g_server_logic_thread);
    pthread_join(g_server_logic_thread, NULL);

    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_CLIENTS);

    _exit(0);
}

void client_signal_handler(int signum) {
    printf("[KLIENT] Prijatý signál %d, ukončujem...\n", signum);
    exit(0);
}

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