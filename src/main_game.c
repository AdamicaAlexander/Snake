#include "game_logic.h"

int main() {
    GameWorld world;

    initialize_game(&world, "../../assets/world.txt");

    spawn_snake(&world, 0);
    spawn_fruit(&world);

    render_game(&world);

    return 0;
}
