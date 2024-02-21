#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#include <iostream>
#include "../include/chippie.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "../include/gui_window_file_dialog.h"

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 500;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    //SetTargetFPS(600);

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    bool exitWindow = false;

    char fileNameToLoad[512] = { 0 };

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

        exitWindow = WindowShouldClose();

        if (fileDialogState.SelectFilePressed)
        {

            // Load ROM file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".ch8"))
            {
                strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                //UnloadTexture(texture);
                //texture = LoadTexture(fileNameToLoad);
            }

            fileDialogState.SelectFilePressed = false;
        }

        BeginDrawing();

        ClearBackground(DARKGRAY);

        // load next instruction into memory
        chippie.fetch();
        // execute instruction
        chippie.decode_execute_opcode();
        //update clocks
        chippie.tick();

        //chippie.render_display();
        DrawText(fileNameToLoad, 208, GetScreenHeight() - 20, 10, GRAY);

        // raygui: controls drawing
        //----------------------------------------------------------------------------------
        if (fileDialogState.windowActive) GuiLock();
        if (GuiButton((Rectangle){ 20, 20, 140, 30 }, GuiIconText(ICON_FILE_OPEN, "Open ROM"))) fileDialogState.windowActive = true;
        GuiUnlock();
        // GUI: Dialog Window
        //--------------------------------------------------------------------------------
        GuiWindowFileDialog(&fileDialogState);

        // end frame
        EndDrawing();

    }

    CloseWindow();
    return 0;
}
