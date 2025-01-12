#ifndef CLIENT_H
#define CLIENT_H

#include "ipc.h"
#include "game_logic.h"

#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

void run_client(IPCResources *ipc, bool new_game);

void setup_terminal();
void restore_terminal();
void stop_client();

#endif // CLIENT_H
