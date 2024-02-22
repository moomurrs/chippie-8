#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "../include/gui_window_file_dialog.h"
#include <iostream>
#include "../include/chippie.h"
#include <string>

constexpr auto SCREEN_WIDTH  = 800;
constexpr auto SCREEN_HEIGHT = 500;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    //SetTargetFPS(600);

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    bool exitWindow = false;

    // name and fully-qualified path of rom file
    std::string file_qualified_path{};

    Chippie chippie{};
    chippie.load_rom_to_ram("../test/Pong (1 player).ch8");

    while (!WindowShouldClose()){

        exitWindow = WindowShouldClose();

        if (fileDialogState.SelectFilePressed){
            // Load ROM file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".ch8")){
                // grab rom's fully-qualified path + name, using the platform-specific path separator
                file_qualified_path = TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText);
                // load rom into ram
                chippie.load_rom_to_ram(file_qualified_path.c_str());
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
        DrawText(file_qualified_path.c_str(), 208, GetScreenHeight() - 20, 10, GRAY);

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
