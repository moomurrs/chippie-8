#include <fstream>
#include <filesystem>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "memory.h"
#include "display.h"

class Chippie {
public:

    void fetch_and_execute_opcode(){
        const uint8_t first = *(_memory._get_rom_offset(_memory._get_program_counter()));
        const uint8_t second =  *(_memory._get_rom_offset(_memory._get_program_counter() + 1));

        uint16_t opcode = first;
        // put first byte into higher, second byte lower
        opcode = (opcode << 8) | second;

        //spdlog::info("both: {:x}", opcode);

        constexpr uint16_t op_mask = 0xF000;
        constexpr uint16_t x_mask = 0x0F00; // 2nd nibble
        constexpr uint16_t y_mask = 0x00F0; // 3rd nibble
        constexpr uint16_t n_mask = 0x000F; // 4th nibble

        // 8-bit immediate value
        constexpr uint16_t nn_mask = 0x00FF; // 3rd & 4th nibble
        // 12-bit immediate value
        constexpr uint16_t nnn_mask = 0x0FFF; // 2nd, 3rd, & 4th nibble

        uint8_t instruction = (opcode & op_mask) >> 12;

        uint16_t index = 0;
        uint16_t val = 0;

        switch(instruction){
        case 0xD:
            // render display

            break;
        case 0xA:
            // set index register i
            val = opcode & nnn_mask;
            _memory._set_index_register(val);
            break;

        case 0x7:
            // add immediate value nn to register vx
            index = opcode & x_mask;
            val = _memory._get_vregister(index);
            _memory._set_vregister(opcode & x_mask, val + (opcode & nn_mask));
            break;
        case 0x6:
            // set register to immediate value nn
            _memory._set_vregister(opcode & x_mask, opcode & nn_mask);
            break;
        case 0x1:
            // jump to immediate value nnn

            break;
        case 0x00:
            // candidate for clear display, continue checking
            if((opcode & nn_mask) == 0xE0){ // checking lower byte
                // clear screen
                _display.clear();
                break;
            }

        }

        // increment pc
        _memory._set_program_counter(_memory._get_program_counter() + 2);

    }

    void load_rom_to_ram(const char* file_path){
        spdlog::info("Loading into ram from rom.");
        // zero out registers for fresh start
        _memory.erase_memory(false); // false since it'll be overwritten anyway
        // load rom in binary
        std::ifstream rom{file_path, std::ios::binary};
        if(!rom.is_open()){
            // error
            std::string err{"ERROR: failed to open file "};
            spdlog::error(err);
            throw std::runtime_error{err};
        }
        // file opened successfully
        rom.exceptions(std::ifstream::badbit); // fail stream on error
        // get rom size
        const size_t rom_size = std::filesystem::file_size(file_path);
        // copy to into chippie ram
        rom.read((char*)_memory.get_ram_rom_start_location(), rom_size);
        // close rom file
        rom.close();
    }

    Display& display(){
        return _display;
    }

    // for testing
    Memory& memory(){
        return _memory;
    }

private:
    Display _display{1.0, DARKGRAY, RED}; // graphics helpers
    Memory _memory{};      // internal chip8 memory
};
