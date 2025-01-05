#include "game_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void initialize_game(GameWorld *world, const char *file_path) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            world->grid[y][x] = TILE_EMPTY;
        }
    }

    FILE *file = fopen(file_path, "r");
    if (file) {
        char line[WORLD_WIDTH + 2];
        int y = 0;

        while (fgets(line, sizeof(line), file) && y < WORLD_HEIGHT) {
            for (int x = 0; x < WORLD_WIDTH && line[x] != '\n' && line[x] != '\0'; x++) {
                if (line[x] == '#') {
                    world->grid[y][x] = TILE_OBSTACLE;
                }
            }
            y++;
        }
        fclose(file);

        printf("Game world loaded from file: %s\n", file_path);
    } else {
        printf("Could not load world file '%s'. Falling back to an empty world.\n", file_path);
    }

    world->num_snakes = 0;
    world->num_fruits = 0;

    srand(time(NULL));
}

void spawn_snake(GameWorld *world, int snake_index) {
    if (snake_index >= MAX_SNAKES) {
        printf("Maximum number of snakes reached.\n");
        return;
    }

    Snake *snake = &world->snakes[snake_index];
    snake->length = 3;
    snake->alive = true;
    snake->direction = LEFT;
    snake->score = 0;

    int x, y;
    do {
        x = rand() % (WORLD_WIDTH - snake->length);
        y = rand() % WORLD_HEIGHT;
    } while (world->grid[y][x] != TILE_EMPTY || world->grid[y][x + 1] != TILE_EMPTY || world->grid[y][x + 2] != TILE_EMPTY);

    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = x + i;
        snake->body[i].y = y;
        world->grid[y][x + i] = TILE_SNAKE;
    }

    world->num_snakes++;
    printf("Snake %d spawned at (%d, %d).\n", snake_index, x, y);
}

void spawn_fruit(GameWorld *world) {
    while (world->num_fruits < world->num_snakes) {
        int x, y;
        do {
            x = rand() % WORLD_WIDTH;
            y = rand() % WORLD_HEIGHT;
        } while (world->grid[y][x] != TILE_EMPTY);

        world->fruits[world->num_fruits].position.x = x;
        world->fruits[world->num_fruits].position.y = y;
        world->fruits[world->num_fruits].active = true;

        world->grid[y][x] = TILE_FRUIT;
        world->num_fruits++;
    }
}

void render_game(const GameWorld *world) {
    printf("\033[H");

    printf("Time: %d seconds\n", world->elapsed_time);
    for (int i = 0; i < world->num_snakes; i++) {
        if (world->snakes[i].alive) {
            printf("Snake %d Score: %d\n", i + 1, world->snakes[i].score);
        }
    }
    printf("\n");

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            bool is_head = false;

            for (int i = 0; i < world->num_snakes; i++) {
                if (world->snakes[i].alive &&
                    world->snakes[i].body[0].x == x &&
                    world->snakes[i].body[0].y == y) {
                    is_head = true;
                    break;
                }
            }

            if (is_head) {
                printf("\u263A ");
            } else {
                switch (world->grid[y][x]) {
                    case TILE_EMPTY: printf(". "); break;
                    case TILE_SNAKE: printf("\u25CB "); break;
                    case TILE_FRUIT: printf("\u2665 "); break;
                    case TILE_OBSTACLE: printf("# "); break;
                }
            }
        }
        printf("\n");

    }

    printf("\n");
    printf("\033[%d;0H", WORLD_HEIGHT + 5);
}

void handle_collision(GameWorld *world, Snake *snake) {
    snake->alive = false;

    printf("\n Snake collided with itself or an obstacle and died!\n");

    for (int i = 0; i < snake->length; i++) {
        int x = snake->body[i].x;
        int y = snake->body[i].y;
        world->grid[y][x] = TILE_EMPTY;
    }

    world->num_snakes--;
    render_game(world);
}

bool check_collision(GameWorld *world, Snake *snake, int *next_x, int *next_y) {
    if (*next_x < 0) {
        *next_x = WORLD_WIDTH - 1;
    } else if (*next_x >= WORLD_WIDTH) {
        *next_x = 0;
    }
    if (*next_y < 0) {
        *next_y = WORLD_HEIGHT - 1;
    } else if (*next_y >= WORLD_HEIGHT) {
        *next_y = 0;
    }

    for (int i = 0; i < snake->length; i++) {
        if (snake->body[i].x == *next_x && snake->body[i].y == *next_y) {
            return true;
        }
    }

    if (world->grid[*next_y][*next_x] == TILE_OBSTACLE || world->grid[*next_y][*next_x] == TILE_SNAKE) {
        return true;
    }

    return false;
}


void move_snake(GameWorld *world, Snake *snake) {
    if (!snake->alive) {
        return;
    }

    int next_x = snake->body[0].x;
    int next_y = snake->body[0].y;

    switch (snake->direction) {
        case UP:    next_y--; break;
        case DOWN:  next_y++; break;
        case LEFT:  next_x--; break;
        case RIGHT: next_x++; break;
    }

    if (check_collision(world, snake, &next_x, &next_y)) {
        handle_collision(world, snake);
        return;
    }

    int old_tail_x = snake->body[snake->length - 1].x;
    int old_tail_y = snake->body[snake->length - 1].y;

    bool ate_fruit = (world->grid[next_y][next_x] == TILE_FRUIT);

    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0].x = next_x;
    snake->body[0].y = next_y;

    world->grid[next_y][next_x] = TILE_SNAKE;

    if (ate_fruit) {
        if (snake->length < MAX_SNAKE_LENGTH) {
            snake->length++;
	    snake->body[snake->length - 1].x = old_tail_x;
            snake->body[snake->length - 1].y = old_tail_y;
        }
        snake->score++;
        world->num_fruits--;
        spawn_fruit(world);
    } else {
        world->grid[old_tail_y][old_tail_x] = TILE_EMPTY;
    }
}
