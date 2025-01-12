#include "ipc.h"
#include "game_logic.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    IPCResources ipc;

    if (init_ipc(&ipc, 1) < 0) {
        fprintf(stderr, "[SERVER] Nepodarilo sa inicializovať IPC.\n");
        exit(1);
    }

    printf("[SERVER] Zdieľaná pamäť a semafory inicializované.\n");

    start_server(&ipc);

    cleanup_ipc(&ipc);

    return 0;
}
