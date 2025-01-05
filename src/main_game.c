#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>
#include "game_logic.h"

struct termios original_termios;

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

void handle_input(Snake *snake) {
    char input;
    if (read(STDIN_FILENO, &input, 1) > 0) {
        switch (input) {
            case 'w': if (snake->direction != DOWN) snake->direction = UP; break;
            case 's': if (snake->direction != UP) snake->direction = DOWN; break;
            case 'a': if (snake->direction != RIGHT) snake->direction = LEFT; break;
            case 'd': if (snake->direction != LEFT) snake->direction = RIGHT; break;
        }
    }
}

int main(int argc, char *argv[]) {
    system("clear");
    printf("\033[%d;0H", WORLD_HEIGHT + 5);
    GameWorld world;

    initialize_game(&world, argc > 1 ? argv[1] : NULL);
    spawn_snake(&world, 0);
    spawn_fruit(&world);
    render_game(&world);

    char ch;
    printf("Press Enter to start\n");
    scanf("%c",&ch);

    struct timespec sleep_time = {0, 250000000L}; // 200ms
    time_t start_time = time(NULL);

    setup_terminal();

    while (world.snakes[0].alive) {
        time_t current_time = time(NULL);
        world.elapsed_time = (int) (current_time - start_time);

        render_game(&world);

        handle_input(&world.snakes[0]);

        move_snake(&world, &world.snakes[0]);

        nanosleep(&sleep_time, NULL);
    }

    printf("Game Over! Final Score: %d\n\n", world.snakes[0].score);
    return 0;
}
