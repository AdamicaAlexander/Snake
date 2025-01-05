#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>

#define MAX_SNAKES 4
#define MAX_FRUITS 4
#define WORLD_WIDTH 20
#define WORLD_HEIGHT 20

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
    Position body[WORLD_WIDTH * WORLD_HEIGHT];
    int length;
    Direction direction;
    bool alive;
    int score;
} Snake;

typedef struct {
    Position position;
    bool active;
} Fruit;

typedef struct {
    TileType grid[WORLD_HEIGHT][WORLD_WIDTH];
    Snake snakes[MAX_SNAKES];
    Fruit fruits[MAX_FRUITS];
    int num_snakes;
    int num_fruits;
} GameWorld;

void initialize_game(GameWorld *world, const char *file_path);
void spawn_snake(GameWorld *world, int snake_index);
void spawn_fruit(GameWorld *world);
void render_game(const GameWorld *world);

#endif // GAME_LOGIC_H
