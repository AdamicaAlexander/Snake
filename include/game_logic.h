#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stdio.h>

#define MAX_SNAKES 4
#define MAX_FRUITS 4
#define DEFAULT_WIDTH 20
#define DEFAULT_HEIGHT 20
#define MAX_WIDTH 50
#define MAX_HEIGHT 50
#define MAX_SNAKE_LENGTH 2500

typedef enum {
    TILE_EMPTY,
    TILE_SNAKE,
    TILE_FRUIT,
    TILE_OBSTACLE
} TileType;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct {
    int x, y;
} Position;

typedef struct {
    Position body[MAX_SNAKE_LENGTH];
    int length;
    Direction direction;
    bool alive;
    int score;
} Snake;

typedef struct {
    TileType grid[MAX_HEIGHT][MAX_WIDTH];
    Snake snakes[MAX_SNAKES];
    int num_snakes;
    int num_fruits;
    int elapsed_time;
    int world_width;
    int world_height;
    int game_duration;
} GameWorld;

bool check_collision(GameWorld *world, Snake *snake, int *next_x, int *next_y);
void handle_collision(GameWorld *world, Snake *snake);
void move_snake(GameWorld *world, Snake *snake);
void initialize_game(GameWorld *world, const char *file_path);
void spawn_snake(GameWorld *world, int snake_index);
void spawn_fruit(GameWorld *world);
void render_game(const GameWorld *world);

#endif // GAME_LOGIC_H
