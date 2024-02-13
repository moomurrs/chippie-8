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

        const uint16_t first = *(_memory.ram_offset(pc));
        const uint8_t second = *(_memory.ram_offset(pc + 1));
        spdlog::info("-----------------------------");
        spdlog::info("first: 0x{:x}, second: 0x{:x}", first, second);

        // put first byte into higher, second byte lower
        const uint16_t opcode = (first << 8) | second;

        constexpr uint16_t op_mask = 0xF000;
        constexpr uint16_t x_mask = 0x0F00; // 2nd nibble
        constexpr uint16_t y_mask = 0x00F0; // 3rd nibble
        constexpr uint16_t n_mask = 0x000F; // 4th nibble

        // 8-bit immediate value
        constexpr uint16_t nn_mask = 0x00FF; // 3rd & 4th nibble
        // 12-bit immediate value
        constexpr uint16_t nnn_mask = 0x0FFF; // 2nd, 3rd, & 4th nibble

        const uint8_t instruction = (opcode & op_mask) >> 12;

        switch(instruction){
        case 0xD:
        {
            // render display
            spdlog::info("DXYN");
            const uint16_t x_nibble = (opcode & x_mask) >> 8;
            const uint16_t y_nibble = (opcode & y_mask) >> 4;
            const uint16_t n_rows = (opcode & n_mask);

            const uint16_t x_pos_corner = _memory.v_reg(x_nibble) % 64;
            const uint16_t y_pos_corner = _memory.v_reg(y_nibble) % 32;

            spdlog::info("x: {:d}, y: {:d}, n: {:d}", x_pos_corner, y_pos_corner, n_rows);

            // set vf to 0
            _memory.v_reg(15, 0);

            //spdlog::info("n address: {:d}", _memory._get_index_register());

            uint8_t bit_mask = 0b10000000;
            // height of sprite
            for(size_t i = 0; i < n_rows; i++){

                const uint16_t index = _memory.i_reg() + i;
                //spdlog::info("index: {:d}", index);
                const uint8_t sprite = *_memory.ram_offset(index);

                // width of sprite
                for(size_t j = 0; j < 8; j++){

                    const bool pixel_request = sprite & bit_mask;
                    const bool pixel_status = _display.pixels()[i][j];

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
            const uint16_t  val = opcode & nnn_mask;
            _memory.i_reg(val);
            _memory.move_pc();
            break;
        }



        case 0x1:
        {
            // jump to immediate value nnn
            const uint16_t val = opcode & nnn_mask;
            _memory.pc(val);
            spdlog::info("1NNN: to location {:x}", val);
            // dont increment pc
            break;
        }

        case 0x2:
        {
            // run subroutine at nnn, cache current pc on stack
            spdlog::info("2NNN");
            const uint16_t  sub_routine = opcode & nnn_mask;

            const uint16_t current_sp = _memory.sp();
            const uint16_t current_pc = _memory.pc();

            spdlog::info("stack index: {:d} saving pc: {:d}", current_sp,
                         current_pc);

            // push program counter to stack
            _memory.stack_layer(current_sp, current_pc);
            // increment sp
            _memory.sp(current_sp + 1);
            // fast-forward program counter to subroutine
            // will be executed on next cycle, do not move pc
            _memory.pc(sub_routine);

            spdlog::info("new stack: {:d}, new pc: {:d}", _memory.sp(),
                         sub_routine);

            break;
        }

        case 0x0:
        {
            // candidate for clear display, continue checking
            if((opcode & nn_mask) == 0xE0){ // checking lower byte
                // clear screen
                spdlog::info("00E0: clear screen");
                _display.clear();
                _memory.move_pc();

            }else if((opcode & nn_mask) == 0xEE){
                // return from subroutine
                spdlog::info("00EE: return subroutine");
                // pop stack depth
                const uint16_t current_sp = _memory.sp();
                const uint16_t return_pc = _memory.stack_layer(current_sp - 1);
                spdlog::info("previous stack: {:d}, returning to pc: {:d}",
                             current_sp - 1,
                             return_pc);

                _memory.pc(_memory.stack_layer(current_sp - 1));
                // pop stack
                const uint16_t new_sp = current_sp - 1;
                _memory.sp(new_sp);
                // move pc past the old instruction
                _memory.move_pc();

            }else{
                spdlog::critical("0x00?? bad lower bytes: 0x{:x}", opcode);
                spdlog::critical("rest: 0x{:x}", opcode & nn_mask);
            }
            break;
        }

        case 0x3:
        {

            // skip instruction(2 bytes) if vx == nn
            spdlog::info("3XNN");
            // v index
            const uint8_t x = (opcode & x_mask) >> 8;
            // v reg value
            const uint8_t vx = _memory.v_reg(x);
            // immediate value
            const uint8_t nn = opcode & nn_mask;
            // skip the next instruction if equal
            if(vx == nn){
                _memory.move_pc();
            }

            spdlog::info("vx: 0x{:x}", vx);
            spdlog::info("nn: 0x{:x}", nn);
            _memory.move_pc();

            break;
        }

        case 0x4:
        {

            // skip instruction(2 bytes) if vx != nn
            spdlog::info("4XNN");
            // v index
            const uint8_t x = (opcode & x_mask) >> 8;
            // v reg value
            const uint8_t vx = _memory.v_reg(x);
            // immediate value
            const uint8_t nn = opcode & nn_mask;
            // skip the next instruction if not equal
            if(vx != nn){
                _memory.move_pc();
            }

            spdlog::info("vx: 0x{:x}", vx);
            spdlog::info("nn: 0x{:x}", nn);
            _memory.move_pc();

            break;
        }

        case 0x5:
        {

            // skip if vx == vy
            spdlog::info("5XY0");
            // x index
            const uint8_t x = (opcode & x_mask) >> 8;
            // y index
            const uint8_t y = (opcode & y_mask) >> 4;
            // vx reg value
            const uint8_t vx = _memory.v_reg(x);
            // vy reg value
            const uint8_t vy = _memory.v_reg(y);


            if(vx == vy){
                _memory.move_pc();
            }

            spdlog::info("vx: 0x{:x}", vx);
            spdlog::info("vy: 0x{:x}", vy);
            _memory.move_pc();

            break;
        }

        case 0x6:
        {
            // set register to immediate value nn
            const uint8_t x = (opcode & x_mask) >> 8;
            const uint8_t nn = opcode & nn_mask;
            spdlog::info("6XNN");
            spdlog::info("x: 0x{:x}", x);
            spdlog::info("nn: 0x{:x}", nn);
            _memory.v_reg(x, nn);
            _memory.move_pc();
            break;
        }

        case 0x7:
        {
            // add immediate value nn to register vx
            const uint8_t x = (opcode & x_mask) >> 8;
            const uint8_t nn = opcode & nn_mask;
            spdlog::info("7XNN");
            spdlog::info("x: 0x{:x}", x);
            spdlog::info("nn: 0x{:x}", nn);

            const uint16_t vx = _memory.v_reg(x);
            spdlog::info("current reg val: 0x{:x}", vx);
            _memory.v_reg(x, (vx + nn) % 256);
            _memory.move_pc();
            break;
        }

        case 0x8:
        {

            uint8_t derivative = opcode & n_mask;
            // x index
            const uint8_t x = (opcode & x_mask) >> 8;
            // y index
            const uint8_t y = (opcode & y_mask) >> 4;
            // vx reg value
            const uint8_t vx = _memory.v_reg(x);
            // vy reg value
            const uint8_t vy = _memory.v_reg(y);

            if(derivative == 0){
                spdlog::info("0x8XY version 0");
                _memory.v_reg(x, vy);
            }else if(derivative == 0x1){
                spdlog::info("0x8XY version 1");
                _memory.v_reg(x, vx | vy);
            }else if(derivative == 0x2){
                spdlog::info("0x8XY version 2");
                _memory.v_reg(x, vx & vy);
            }else if(derivative == 0x3){
                spdlog::info("0x8XY version 3");
                _memory.v_reg(x, vx ^ vy);
            }else if(derivative == 0x4){
                spdlog::info("0x8XY version 4");
                if(vx + vy > 255){
                    // vf carry enable
                    _memory.v_reg(0xF, 1);
                }else{
                    // vf no carry
                    _memory.v_reg(0xF, 0);
                }

                // only keep lower byte, if overflowed
                _memory.v_reg(x, (uint8_t) (vx + vy));

            }else if(derivative == 0x5){
                spdlog::info("0x8XY version 5");
                if(vx > vy){
                    _memory.v_reg(0xF, 1);
                }else{
                    _memory.v_reg(0xF, 0);
                }
                // only keep lower byte, if underflowed
                _memory.v_reg(x, (uint8_t)(vx - vy));

            }else if(derivative == 0x6){
                spdlog::info("0x8XY version 6");

                // vy value to x index
                _memory.v_reg(x, vy);
                // shift vx right 1 bit
                const uint8_t bit_mask = 0b00000001;
                const uint8_t last_bit = vx & bit_mask;

                // shift vx right 1 bit, update index x
                _memory.v_reg(x, vx >> 1);
                // carry set, depending on value of bit shifted out
                _memory.v_reg(0xF, last_bit);

            }else if(derivative == 0x7){
                spdlog::info("0x8XY version 7");
                if(vy > vx){
                    _memory.v_reg(0xF, 1);
                }else{
                    _memory.v_reg(0xF, 0);
                }
                // only keep lower byte, if underflowed
                _memory.v_reg(x, (uint8_t)(vy - vx));

            }else if(derivative == 0xE){
                spdlog::info("0x8XY version E");

                // vy value to x index
                _memory.v_reg(x, vy);
                // shift vx right 1 bit
                const uint8_t bit_mask = 0b10000000;
                const uint8_t last_bit = (vx & bit_mask) >> 7;

                //spdlog::info("vx: {:x}", vx);
                uint8_t vx_left_shift = vx << 1;
                //spdlog::info("LEFT: {:x}", vx_left_shift);

                // shift vx left 1 bit, update index x
                _memory.v_reg(x, vx_left_shift);
                // carry set, depending on value of bit shifted out
                _memory.v_reg(0xF, last_bit);

            }else{
                spdlog::critical("bad 0x8XY? derivative");
            }

            _memory.move_pc();
            break;
        }

        case 0x9:
        {
            // skip if vx != vy
            spdlog::info("9XY0");
            // x index
            const uint8_t x = (opcode & x_mask) >> 8;
            // y index
            const uint8_t y = (opcode & y_mask) >> 4;

            // vx reg value
            const uint8_t vx = _memory.v_reg(x);
            // vy reg value
            const uint8_t vy = _memory.v_reg(y);

            if(vx != vy){
                _memory.move_pc();
            }

            spdlog::info("x: 0x{:x}", x);
            spdlog::info("y: 0x{:x}", y);

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
