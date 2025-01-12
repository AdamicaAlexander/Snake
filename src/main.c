#include "server.h"
#include "client.h"

int main() {
    IPCResources ipc;
    bool new_game = false;
    int running = 1;

    while (running) {
        int client_choice;
        printf("1. New Game\n");
        printf("2. Join game\n");
        printf("3. Quit\n");
        printf("Pick a choice: ");
        scanf("%d", &client_choice);
        while (getchar() != '\n');

        switch (client_choice) {
            case 1:
                new_game = true;
                pid_t pid = fork();
                if (pid < 0) {
                    perror("[MAIN] fork");
                    exit(1);
                }
                if (pid == 0) {
                    execl("./server", "server", (char *)NULL);
                    perror("[MAIN] execl failed");
                    exit(1);
                } else {
                    sleep(1);

                    if (init_ipc(&ipc, 0) < 0) {
                        exit(1);
                    }

                    run_client(&ipc, new_game);
                    cleanup_ipc(&ipc);
                }
                break;
            case 2:
                if (init_ipc(&ipc, 0) < 0) {
                    printf("[MAIN] Nepodarilo sa pripojiť k serveru.\n");
                    break;
                }

                run_client(&ipc, new_game);
                cleanup_ipc(&ipc);
                break;
            case 3:
                running = 0;
                break;
            default:
                printf("[MAIN] Neplatná voľba.\n");
                break;
        }
    }

    return 0;
}
