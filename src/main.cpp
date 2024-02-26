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
    spdlog::set_level(spdlog::level::critical);

    Chippie chippie{};
    chippie.load_rom_to_ram("../test/6-keypad.ch8");


    int toggle_x = chippie.display().left_pixel(20);
    int toggle_y = chippie.display().bottom_pixel(10);
    int toggle_width = 90;

    int step_x = chippie.display().left_pixel(toggle_x + 120);
    int step_y = chippie.display().bottom_pixel(10);
    int step_width = 90;

    int op_text_x = step_x + 100;
    int op_text_y = step_y;
    int op_text_size = 24;



    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    bool exitWindow = false;

    // name and fully-qualified path of rom file
    std::string file_qualified_path{};

    //chippie.load_rom_to_ram("../test/1-chip8-logo.ch8");

    //chippie.memory().ram(0x1FF) = 1; // force input

    int halt_program = 0;
    bool step_forward = false;

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
                    (float) step_x,
                    (float) step_y,
                    (float) step_width, 30 },
                "Step Forward");
            // execute instruction
            chippie.decode_execute_opcode();
            //update clocks
            chippie.tick();
            DrawText("next opcode: 0x----", op_text_x, op_text_y, op_text_size, GRAY);

        }else{
            // program halted
            // enable step-forward
            GuiSetState(STATE_NORMAL);
            // get current state of button
            step_forward = GuiButton((Rectangle){
                    (float) step_x,
                    (float) step_y,
                    (float) step_width, 30 },
                "Step Forward");

            DrawText(TextFormat("next opcode: 0x%04x", chippie.get_instruction()), op_text_x, op_text_y, op_text_size, ORANGE);

            if(step_forward){
                if(chippie.is_draw_instruction()){
                    //spdlog::critical("drawing...");

                }else{

                }
                // execute instruction
                chippie.decode_execute_opcode();
                //update clocks
                chippie.tick();
            }
        }

        chippie.display().render();

        DrawText(file_qualified_path.c_str(), 450, GetScreenHeight() - 20, 10, RED);

        GuiSetState(STATE_NORMAL); // always draw load button
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
