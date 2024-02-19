#include <raylib.h>
#include <iostream>
#include "../include/chippie.h"

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 500;


int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    SetTargetFPS(600);

    Chippie chippie{};
    //chippie.load_rom_to_ram("../test/1-chip8-logo.ch8");
    //chippie.load_rom_to_ram("../test/2-ibm-logo.ch8");
    //chippie.load_rom_to_ram("../test/3-corax+.ch8");
    //chippie.load_rom_to_ram("../test/4-flags.ch8");
    //chippie.load_rom_to_ram("../test/5-quirks.ch8");
    //chippie.memory().ram(0x1FF) = 1; // force input
    //chippie.load_rom_to_ram("../test/6-keypad.ch8");
    //chippie.memory().ram(0x1FF) = 3; // force input
    chippie.load_rom_to_ram("../test/Pong (1 player).ch8");
    //chippie.load_rom_to_ram("../test/delay_timer_test.ch8");
    //chippie.load_rom_to_ram("../test/single_font.ch8");
    //chippie.load_rom_to_ram("../test/bcd_test.ch8");
    //chippie.load_rom_to_ram("../test/FX0A_only_test.ch8");
    //chippie.load_rom_to_ram("../test/7-beep.ch8");


    //const uint8_t zero = 0xF;
    ClearBackground(DARKGRAY);

    while (!WindowShouldClose()){

        BeginDrawing();

        // load next instruction into memory, update clocks
        chippie.fetch_and_update();

        //spdlog::info("next instruction: {:x}", chippie.get_instruction());
        /*
        if(chippie.get_second_half() != 0x0A){
            std::cin.get();
            }*/

        // execute instruction
        chippie.decode_execute_opcode();

        EndDrawing();

    }

    CloseWindow();
    return 0;
}
