#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"
#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "../include/gui_window_file_dialog.h"
#include <iostream>
#include "../include/chippie.h"
#include <string>

constexpr auto SCREEN_WIDTH  = 850;
constexpr auto SCREEN_HEIGHT = 500;

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    bool exitWindow = false;

    // name and fully-qualified path of rom file
    std::string file_qualified_path{};

    Chippie chippie{};
    //chippie.load_rom_to_ram("../test/1-chip8-logo.ch8");
    chippie.load_rom_to_ram("../test/6-keypad.ch8");
    //chippie.memory().ram(0x1FF) = 1; // force input

    int halt_program = 0;
    bool step_forward = false;
    int step_button_width = 90;

    std::string draw_status = "";
    std::string halting = "";

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
        //ClearBackground(DARKGRAY);

        // load next instruction into memory
        chippie.fetch();

        DrawText(TextFormat("next: %x", chippie.get_first_half()), 500, 410, 30, ORANGE);

        GuiSetState(STATE_NORMAL); // always draw toggle
        // get current state of toggle
        GuiToggleSlider((Rectangle){
                (float)chippie.display().left_pixel(20),
                (float)chippie.display().bottom_pixel(10),
                90, 30 },
            "RUN;HALT", &halt_program);

        // execute opcode if toggle slider is ON
        if(!halt_program){
            // resume program normally
            // draw disable step button
            GuiSetState(STATE_DISABLED);
            GuiButton((Rectangle){
                    (float)chippie.display().left_pixel(140),
                    (float)chippie.display().bottom_pixel(10),
                    (float)step_button_width, 30 },
                "Step Forward");
            // execute instruction
            chippie.decode_execute_opcode();
            //update clocks
            chippie.tick();
            halting = "running";
        }else{
            halting = "halting";
            ClearBackground(DARKGRAY); // clear screen to update texts

            // program halted
            // enable step-forward
            GuiSetState(STATE_NORMAL);
            // get current state of button
            step_forward = GuiButton((Rectangle){
                    (float)chippie.display().left_pixel(140),
                    (float)chippie.display().bottom_pixel(10),
                    (float)step_button_width, 30 },
                "Step Forward");

            if(step_forward){
                if(chippie.is_draw_instruction()){
                    //spdlog::critical("drawing...");
                    draw_status = "drawing...";
                }else{
                    draw_status = "----";
                }
                // execute instruction
                chippie.decode_execute_opcode();
                //update clocks
                chippie.tick();
            }
            // force render pixel buffer (so it persists on pausing)
            chippie.display().render();
        }

        DrawText(TextFormat("draw status: %s", draw_status.c_str()), 500, 350, 30, BLUE);
        DrawText(TextFormat("halt status: %s", halting.c_str()), 500, 390, 30, GREEN);

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
