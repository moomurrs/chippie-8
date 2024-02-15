#include <raylib.h>
#include "../include/chippie.h"

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 500;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    SetTargetFPS(60);

    Chippie chippie{};
    //chippie.load_rom_to_ram("../test/2-ibm-logo.ch8");
    //chippie.load_rom_to_ram("../test/3-corax+.ch8");
    //chippie.load_rom_to_ram("../test/4-flags.ch8");
    //chippie.load_rom_to_ram("../test/5-quirks.ch8");
    chippie.load_rom_to_ram("../test/6-keypad.ch8");
    chippie.memory().ram(0x1FF) = 1; // force input test on rom 6


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
        // update input buffer
        chippie.update_input();
        // execute
        chippie.fetch_and_execute_opcode();
        //bool k = chippie.input().is_pressing((Input::Keys)10);
        //spdlog::info("{:d}", k);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
