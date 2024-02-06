#include <fstream>
#include <filesystem>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "memory.h"
#include "display.h"

class Chippie {
public:

    void fetch_and_execute_opcode(){

        const uint16_t pc = _memory._get_program_counter();

        const uint8_t first = *(_memory._get_rom_offset(pc));
        const uint8_t second = *(_memory._get_rom_offset(pc + 1));

        spdlog::info("first: 0x{:x}, second: 0x{:x}", first, second);


        uint16_t opcode = first;
        // put first byte into higher, second byte lower
        opcode = (opcode << 8) | second;

        //spdlog::info("both: {:x}", opcode);
        //spdlog::info("0x200: {:x}", *_memory._get_rom_offset(0));


        constexpr uint16_t op_mask = 0xF000;
        constexpr uint16_t x_mask = 0x0F00; // 2nd nibble
        constexpr uint16_t y_mask = 0x00F0; // 3rd nibble
        constexpr uint16_t n_mask = 0x000F; // 4th nibble

        // 8-bit immediate value
        constexpr uint16_t nn_mask = 0x00FF; // 3rd & 4th nibble
        // 12-bit immediate value
        constexpr uint16_t nnn_mask = 0x0FFF; // 2nd, 3rd, & 4th nibble

        uint8_t instruction = (opcode & op_mask) >> 12;
        //spdlog::info("both: {:x}", instruction);


        switch(instruction){
        case 0xD:
        {
            // render display

            uint16_t x_nibble = (opcode & x_mask) >> 8;
            uint16_t y_nibble = (opcode & y_mask) >> 4;
            uint16_t n_rows = (opcode & n_mask);

            //uint16_t n_rows = dummy & n_mask;

            uint16_t x_pos = _memory._get_vregister(x_nibble) % 64;
            uint16_t y_pos = _memory._get_vregister(y_nibble) % 32;
            //uint16_t x_pos = 0;
            //uint16_t y_pos = 0;

            // set vf to 0
            _memory._set_vregister(15, 0);


            uint8_t bit_mask = 0b10000000;
            // height of sprite
            for(size_t i = 0; i < n_rows; i++){
                uint8_t sprite = _memory._get_index_register();

                // width of sprite
                for(size_t j = 0; j < 8; j++){
                    spdlog::info("bit: {:d}", sprite & bit_mask);

                    if((sprite & bit_mask) && _display.pixels()[i][j]){
                        _display.pixels()[i][j] = false;
                        _memory._set_vregister(15, 1);
                        spdlog::info("x");
                    }else{
                        _display.pixels()[i][j] = true;
                        spdlog::info(".");
                    }

                    bit_mask >>= 1;
                    x_pos++;
                }
                // reset bitmask
                bit_mask = 0b10000000;
                y_pos++;

                spdlog::info("!");


            }

            _display.render();

            break;
        }

        case 0xA:
        {
            // set index register i
            uint16_t  val = opcode & nnn_mask;
            _memory._set_index_register(val);
            break;
        }

        case 0x7:
        {
            // add immediate value nn to register vx
            uint16_t index = opcode & x_mask;
            uint16_t current_val = _memory._get_vregister(index);
            _memory._set_vregister(opcode & x_mask,
                                   current_val + (opcode & nn_mask));
            break;
        }

        case 0x6:
            // set register to immediate value nn
            spdlog::info("x: 0x{:x}", (opcode & x_mask) >> 8);
            spdlog::info("nn: 0x{:x}", opcode & nn_mask);
            _memory._set_vregister((opcode & x_mask) >> 8, opcode & nn_mask);
            break;
        case 0x1:
            // jump to immediate value nnn

            break;
        case 0x0:
            // candidate for clear display, continue checking
            if((opcode & nn_mask) == 0xE0){ // checking lower byte
                // clear screen
                _display.clear();

            }else{
                spdlog::critical("0x0 but not e0: 0x{:x}", opcode);
                spdlog::critical("rest: 0x{:x}", opcode & nn_mask);

            }
            break;
        default:
            spdlog::critical("bad instruction: 0x{:x}", opcode);

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
    Display _display{1.0, BLACK, RED}; // graphics helpers
    Memory _memory{};      // internal chip8 memory
};
