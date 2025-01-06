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
    GameWorld world;

    printf("Select game mode:\n");
    printf("1. Standard Mode\n");
    printf("2. Timed Mode\n");
    printf("Enter your choice (1 or 2): ");
    int mode_choice;
    scanf("%d", &mode_choice);

    if (mode_choice == 1) {
        world.game_duration = 0;
    } else if (mode_choice == 2) {
        int duration = 0;
        printf("Enter time limit in seconds: ");
        scanf("%d", &duration);

        if (duration <= 0) {
            printf("Invalid time limit! Defaulting to Standard Mode.\n");
            duration = 0;
        }

        world.game_duration = duration;

    } else {
        printf("Invalid choice! Defaulting to Standard Mode.\n");
        world.game_duration = 0;
    }

    int world_choice;
    printf("Select world type:\n");
    printf("1. Obstacle-free world (custom dimensions)\n");
    printf("2. World with obstacles (20x20 world loaded from file)\n");
    printf("Enter your choice (1 or 2): ");
    scanf("%d", &world_choice);

    if (world_choice == 1) {
        int width, height;
        printf("Enter world width (max %d): ", MAX_WIDTH);
        scanf("%d", &width);
        printf("Enter world height (max %d): ", MAX_HEIGHT);
        scanf("%d", &height);

        if (width <= 0 || width > MAX_WIDTH || height <= 0 || height > MAX_HEIGHT) {
            printf("Invalid dimensions! Using default %dx%d.\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
            width = DEFAULT_WIDTH;
            height = DEFAULT_HEIGHT;
        }

        world.world_width = width;
        world.world_height = height;

        initialize_game(&world, NULL);
    } else if (world_choice == 2) {
        const char *file_path = argc > 1 ? argv[1] : "../../assets/world.txt";
        world.world_width = DEFAULT_WIDTH;
        world.world_height = DEFAULT_HEIGHT;
        initialize_game(&world, file_path);
    } else {
        printf("Invalid choice! Defaulting to obstacle-free world.\n");
        world.world_width = DEFAULT_WIDTH;
        world.world_height = DEFAULT_HEIGHT;
        initialize_game(&world, NULL);
    }

    spawn_snake(&world, 0);
    spawn_fruit(&world);

    system("clear");
    printf("\033[%d;0H", world.world_height + 5);
    render_game(&world);

    char ch;
    printf("Press Enter to start\n");
    scanf("%c",&ch);

    struct timespec sleep_time = {0, 250000000L};
    time_t start_time = time(NULL);

    setup_terminal();

    while (world.snakes[0].alive && (world.game_duration == 0 || world.game_duration - world.elapsed_time > 0)) {
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
