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
    snake->direction = RIGHT;
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
    if (world->num_fruits >= MAX_FRUITS) {
        printf("Maximum number of fruits reached.\n");
        return;
    }

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

    printf("Fruit spawned at (%d, %d).\n", x, y);
}

void render_game(const GameWorld *world) {
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            switch (world->grid[y][x]) {
                case TILE_EMPTY: printf(". "); break;
                case TILE_SNAKE: printf("S "); break;
                case TILE_FRUIT: printf("F "); break;
                case TILE_OBSTACLE: printf("# "); break;
            }
        }
        printf("\n");
    }
    printf("\n");
}
