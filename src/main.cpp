#include <raylib.h>
#include "../include/chippie.h"

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 450;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    SetTargetFPS(5);

    Chippie chippie{};
    //chippie.load_rom_to_ram("../test/1-chip8-logo.ch8");
    chippie.load_rom_to_ram("../test/3-corax+.ch8");
    //chippie.load_rom_to_ram("../test/alien.ch8");


    ClearBackground(DARKGRAY);

    while (!WindowShouldClose()) {
        BeginDrawing();

        //ClearBackground(DARKGRAY);
        /*
        chippie.display().clear();
        //chippie.display().render();


        if(IsKeyDown(KEY_UP)){
            chippie.display().render_all(RED);
            }*/
        chippie.fetch_and_execute_opcode();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
