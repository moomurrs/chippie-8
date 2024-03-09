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
constexpr auto SCREEN_HEIGHT = 550;


int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chippie-8");
    spdlog::set_level(spdlog::level::critical);

    // name and fully-qualified path of rom file
    std::string default_rom = "Brix_Gustafsson_1990.ch8";
    std::string file_qualified_path = "../roms/" + default_rom; // default rom
    std::string rom_text = default_rom; // default rom

    Chippie chippie{};
    chippie.load_rom_to_ram(file_qualified_path.c_str());

    int toggle_x = chippie.display().left_pixel(20);
    int toggle_y = chippie.display().bottom_pixel(10);
    int toggle_width = 90;

    int step_x = chippie.display().left_pixel(toggle_x + 120);
    int step_y = chippie.display().bottom_pixel(10);
    int step_width = 90;

    int op_text_x = step_x + 100;
    int op_text_y = step_y;
    int op_text_size = 25;

    int status_text_x = toggle_x;
    int status_text_y = SCREEN_HEIGHT - 100;
    int status_text_size = 30;

    int rom_text_x = status_text_x;
    int rom_text_y = status_text_y + 50;
    int rom_text_size = 25;


    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    bool exitWindow = false;
    //chippie.load_rom_to_ram("../test/1-chip8-logo.ch8");
    //chippie.memory().ram(0x1FF) = 1; // force input

    int halt_program = 0;
    bool step_forward = false;

    std::string status = "RUNNING";

    while (!WindowShouldClose()){

        exitWindow = WindowShouldClose();

        if (fileDialogState.SelectFilePressed){
            // Load ROM file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".ch8")){
                // grab rom's fully-qualified path + name, using the platform-specific path separator
                file_qualified_path = TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText);
                // load rom into ram
                chippie.load_rom_to_ram(file_qualified_path.c_str());
                rom_text = fileDialogState.fileNameText;
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
            status = "RUNNING";
            //DrawText(TextFormat("Status : %s", status.c_str()), status_text_x, status_text_y, status_text_size, GREEN);

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
                // execute instruction
                chippie.decode_execute_opcode();
                //update clocks
                chippie.tick();
            }
            status = "HALTED";
            DrawText(TextFormat("Status : %s", status.c_str()), status_text_x, status_text_y, status_text_size, GREEN);
        }
        DrawText(TextFormat("Status : %s", status.c_str()), status_text_x, status_text_y, status_text_size, GREEN);
        DrawText(TextFormat("ROM : %s", rom_text.c_str()), rom_text_x, rom_text_y, rom_text_size, SKYBLUE);
        DrawText("Made by Murun", SCREEN_WIDTH - 175, SCREEN_HEIGHT - 25, 20, MAROON);


        chippie.display().render();

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
