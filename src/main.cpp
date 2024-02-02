#include <raylib.h>
#include "../include/chippie.h"

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 450;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    SetTargetFPS(30);

    Chippie chippie{};


    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(DARKGRAY);

        chippie.render();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
