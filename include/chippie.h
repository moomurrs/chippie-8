#include <fstream>
#include <filesystem>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "memory.h"
#include "display.h"

class Chippie {
public:
    Chippie(){
        spdlog::info("chip8: initializing...");
    }

    void fetch_and_execute_opcode(){

        const uint16_t pc = _memory.pc();

        const uint8_t first = *(_memory._get_rom_offset(pc));
        const uint8_t second = *(_memory._get_rom_offset(pc + 1));
        spdlog::info("-----------------------------");
        spdlog::info("first: 0x{:x}, second: 0x{:x}", first, second);

        uint16_t opcode = first;
        // put first byte into higher, second byte lower
        opcode = (opcode << 8) | second;

        constexpr uint16_t op_mask = 0xF000;
        constexpr uint16_t x_mask = 0x0F00; // 2nd nibble
        constexpr uint16_t y_mask = 0x00F0; // 3rd nibble
        constexpr uint16_t n_mask = 0x000F; // 4th nibble

        // 8-bit immediate value
        constexpr uint16_t nn_mask = 0x00FF; // 3rd & 4th nibble
        // 12-bit immediate value
        constexpr uint16_t nnn_mask = 0x0FFF; // 2nd, 3rd, & 4th nibble

        uint8_t instruction = (opcode & op_mask) >> 12;

        switch(instruction){
        case 0xD:
        {
            // render display
            spdlog::info("DXYN");
            uint16_t x_nibble = (opcode & x_mask) >> 8;
            uint16_t y_nibble = (opcode & y_mask) >> 4;
            uint16_t n_rows = (opcode & n_mask);

            uint16_t x_pos_corner = _memory.v_reg(x_nibble) % 64;
            uint16_t y_pos_corner = _memory.v_reg(y_nibble) % 32;

            spdlog::info("x: {:d}, y: {:d}, n: {:d}", x_pos_corner, y_pos_corner, n_rows);

            // set vf to 0
            _memory.v_reg(15, 0);

            //spdlog::info("n address: {:d}", _memory._get_index_register());

            uint8_t bit_mask = 0b10000000;
            // height of sprite
            for(size_t i = 0; i < n_rows; i++){

                uint16_t index = _memory.i_reg() + i;
                //spdlog::info("index: {:d}", index);
                uint8_t sprite = *_memory._get_ram_offset(index);

                // width of sprite
                for(size_t j = 0; j < 8; j++){

                    bool pixel_request = sprite & bit_mask;
                    bool pixel_status = _display.pixels()[i][j];

                    //spdlog::info("requested: {:d}, status: {:d}", pixel_request, pixel_status);

                    if((sprite & bit_mask)){
                        // pixel ON requested
                        if(_display.pixels()[y_pos_corner + i][x_pos_corner + j]){
                            // display pixel on, turn off
                            _display.pixels()[y_pos_corner + i][x_pos_corner + j] = false;
                            _memory.v_reg(15, 1);
                        }else{
                            // display pixel OFF, turn ON
                            _display.pixels()[y_pos_corner + i][x_pos_corner + j] = true;
                        }
                    }

                    bit_mask >>= 1;

                }
                // reset bitmask
                bit_mask = 0b10000000;
            }

            _display.render();
            _memory.move_pc();
            break;
        }

        case 0xA:
        {
            // set index register i
            spdlog::info("ANNN");
            uint16_t  val = opcode & nnn_mask;
            _memory.i_reg(val);
            _memory.move_pc();
            break;
        }

        case 0x7:
        {
            // add immediate value nn to register vx
            uint8_t vx = (opcode & x_mask) >> 8;
            uint8_t nn = opcode & nn_mask;
            spdlog::info("7XNN");
            spdlog::info("x: 0x{:x}", vx);
            spdlog::info("nn: 0x{:x}", nn);

            uint16_t current_val = _memory.v_reg(vx);
            spdlog::info("current reg val: 0x{:x}", current_val);
            _memory.v_reg(vx, (current_val + nn) % 256);
            _memory.move_pc();
            break;
        }

        case 0x6:
            {
                // set register to immediate value nn
                uint8_t x = (opcode & x_mask) >> 8;
                uint8_t nn = opcode & nn_mask;
                spdlog::info("6XNN");
                spdlog::info("x: 0x{:x}", x);
                spdlog::info("nn: 0x{:x}", nn);
                _memory.v_reg(x, nn);
                _memory.move_pc();
                break;
            }

        case 0x1:
            {
                // jump to immediate value nnn
                uint16_t val = opcode & nnn_mask;
                _memory.pc(val);
                spdlog::info("1NNN: to location {:x}", val);
                // dont increment pc
                break;
            }

        case 0x0:
            {
                // candidate for clear display, continue checking
                if((opcode & nn_mask) == 0xE0){ // checking lower byte
                    // clear screen
                    spdlog::info("00E0: clear screen");
                    _display.clear();

                }else{
                    spdlog::critical("0x0 but not e0: 0x{:x}", opcode);
                    spdlog::critical("rest: 0x{:x}", opcode & nn_mask);

                }
                _memory.move_pc();
                break;
            }

        case 0x3:
        {

            // skip instruction(2 bytes) if vx == nn
            spdlog::info("3XNN");
            uint8_t x = (opcode & x_mask) >> 8;
            uint8_t nn = opcode & nn_mask;

            if(x == nn){
                _memory.move_pc();
            }

            spdlog::info("x: 0x{:x}", x);
            spdlog::info("nn: 0x{:x}", nn);
            _memory.move_pc();

            break;
        }

        case 0x4:
        {

            // skip instruction(2 bytes) if vx != nn
            spdlog::info("4XNN");
            uint8_t x = (opcode & x_mask) >> 8;
            uint8_t nn = opcode & nn_mask;

            if(x != nn){
                _memory.move_pc();
            }

            spdlog::info("x: 0x{:x}", x);
            spdlog::info("nn: 0x{:x}", nn);
            _memory.move_pc();

            break;
        }

        case 0x5:
        {

            // skip if vx == vy
            spdlog::info("5XN0");
            uint8_t vx = (opcode & x_mask) >> 8;
            uint8_t vy = opcode & y_mask >> 4;

            if(vx == vy){
                _memory.move_pc();
            }

            spdlog::info("vx: 0x{:x}", vx);
            spdlog::info("vy: 0x{:x}", vy);
            _memory.move_pc();

            break;
        }

        default:
            spdlog::critical("bad instruction or data: 0x{:x}", opcode);
        }

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
