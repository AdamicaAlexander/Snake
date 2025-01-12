#include "game_logic.h"

void initialize_game(GameWorld *world, const char *file_path) {
    for (int y = 0; y < world->world_height; y++) {
        for (int x = 0; x < world->world_width; x++) {
            world->grid[y][x] = TILE_EMPTY;
        }
    }

    FILE *file = fopen(file_path, "r");
    if (file) {
        char line[world->world_width + 2];
        int y = 0;

        while (fgets(line, sizeof(line), file) && y < world->world_height) {
            int len = strlen(line);

            if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                line[len - 1] = '\0';
                len--;
            }

            for (int x = 0; x < len && x < world->world_width; x++) {
                if (line[x] == '#') {
                    world->grid[y][x] = TILE_OBSTACLE;
                }
            }

            if (len > 0) {
                y++;
            }
        }
        fclose(file);
        printf("Game world loaded from file: %s\n", file_path);
    } else {
        if (file_path == NULL) {
            printf("Obstacle-free world created with dimensions %dx%d.\n", world->world_width, world->world_height);
        } else {
            printf("Could not load world file '%s'. Falling back to an empty world.\n", file_path);
        }
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
        x = rand() % (world->world_width - snake->length);
        y = rand() % world->world_height;
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
            x = rand() % world->world_width;
            y = rand() % world->world_height;
        } while (world->grid[y][x] != TILE_EMPTY);

        world->grid[y][x] = TILE_FRUIT;
        world->num_fruits++;
    }
}

void render_game(const GameWorld *world) {
	printf("\033[2J");
    printf("\033[H");

    for (int y = 0; y < world->world_height; y++) {
        for (int x = 0; x < world->world_width; x++) {
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
	printf("Time: %d seconds\n", world->elapsed_time);

    for (int i = 0; i < world->num_snakes; i++) {
        if (world->snakes[i].alive) {
            printf("Snake %d Score: %d\n", i + 1, world->snakes[i].score);
        }
    }
}

void remove_snake(GameWorld *world, Snake *snake) {
    snake->alive = false;

    printf("\n Snake died!\n");

    for (int i = 0; i < snake->length; i++) {
        int x = snake->body[i].x;
        int y = snake->body[i].y;
        world->grid[y][x] = TILE_EMPTY;
    }

    world->num_snakes--;
}

bool check_collision(GameWorld *world, Snake *snake, int *next_x, int *next_y) {
    if (*next_x < 0) {
        *next_x = world->world_width - 1;
    } else if (*next_x >= world->world_width) {
        *next_x = 0;
    }
    if (*next_y < 0) {
        *next_y = world->world_height - 1;
    } else if (*next_y >= world->world_height) {
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

void move_snakes(GameWorld *world) {
    for (int i = 0; i < world->num_snakes; i++) {
        Snake *snake = &world->snakes[i];
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
            remove_snake(world, snake);
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
            if (snake->length < world->world_height * world->world_width) {
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
}

void game_menu(GameWorld *world) {
    printf("Select game mode:\n");
    printf("1. Standard Mode\n");
    printf("2. Timed Mode\n");
    printf("Enter your choice (1 or 2): ");
    int mode_choice;
    scanf("%d", &mode_choice);

    if (mode_choice == 1) {
        world->game_duration = 0;
    } else if (mode_choice == 2) {
        int duration = 0;
        printf("Enter time limit in seconds: ");
        scanf("%d", &duration);

        if (duration <= 0) {
            printf("Invalid time limit! Defaulting to Standard Mode.\n");
            duration = 0;
        }
        world->game_duration = duration;
    } else {
        printf("Invalid choice! Defaulting to Standard Mode.\n");
        world->game_duration = 0;
    }

    int world_choice;
    printf("\nSelect world type:\n");
    printf("1. Obstacle-free world (custom dimensions)\n");
    printf("2. World with obstacles (20x20 world from file)\n");
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

        world->world_width = width;
        world->world_height = height;

        initialize_game(world, NULL);
    } else if (world_choice == 2) {
        char *file_path = "../../assets/world.txt";
        world->world_width = DEFAULT_WIDTH;
        world->world_height = DEFAULT_HEIGHT;
        initialize_game(world, file_path);
    } else {
        printf("Invalid choice! Defaulting to obstacle-free world.\n");
        world->world_width = DEFAULT_WIDTH;
        world->world_height = DEFAULT_HEIGHT;
        initialize_game(world, NULL);
    }
}
