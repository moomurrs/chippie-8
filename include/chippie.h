#include <fstream>
#include <filesystem>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "memory.h"
#include "display.h"
#include "input.h"

class Chippie {
public:
    Chippie(){
        spdlog::info("chip8: initializing...");
    }

    void tick(){
        // halt until cycle timer is done
        while(!cycle_timer.is_timer_done());
        cycle_timer.reset_timer();

        // update delay + sound timers at 60Hz
        if(timer_60Hz.is_timer_done()){
            uint8_t previous_delay = _memory.delay_timer();
            uint8_t previous_sound = _memory.sound_timer();
            // decrement delay on each tick
            if(previous_delay > 0){
                _memory.delay_timer(previous_delay - 1);
            }
            // decrement sound on each tick
            if(previous_sound > 0){
                _memory.sound_timer(previous_sound - 1);
                // play sound
                _memory.play_sound();
            }

            // reset timer
            timer_60Hz.reset_timer();
        }
    }

    void fetch(){
        const uint16_t pc = _memory.pc();
        first_half = *(_memory.ram_offset(pc));
        second_half = *(_memory.ram_offset(pc + 1));
        spdlog::info("-----------------------------");
        spdlog::info("first: 0x{:x}, second: 0x{:x}", first_half, second_half);
        instruction_set = (first_half << 8) | second_half;

    }

    void decode_execute_opcode(){

        //spdlog::info("-----------------------------");
        //spdlog::info("first: 0x{:x}, second: 0x{:x}", first, second);

        const uint16_t opcode = instruction_set;

        constexpr uint16_t op_mask = 0xF000;
        constexpr uint16_t x_mask = 0x0F00; // 2nd nibble
        constexpr uint16_t y_mask = 0x00F0; // 3rd nibble
        constexpr uint16_t n_mask = 0x000F; // 4th nibble

        // 8-bit immediate value
        constexpr uint16_t nn_mask = 0x00FF; // 3rd & 4th nibble
        // 12-bit immediate value
        constexpr uint16_t nnn_mask = 0x0FFF; // 2nd, 3rd, & 4th nibble

        const uint8_t command_nibble = (opcode & op_mask) >> 12;

        switch(command_nibble){
        case 0xD:
            {
                // manipulate pixel buffer (which gets drawn from)
                spdlog::info("DXYN");
                const uint16_t x_nibble = (opcode & x_mask) >> 8;
                const uint16_t y_nibble = (opcode & y_mask) >> 4;
                const uint16_t n_rows = (opcode & n_mask);

                // wrap starting position
                const uint16_t x_pos_corner = (_memory.v_reg(x_nibble)) % 64;
                const uint16_t y_pos_corner = (_memory.v_reg(y_nibble)) % 32;

                // set vf to 0
                _memory.v_reg(15, 0);

                uint8_t bit_mask = 0b10000000;
                // height of sprite
                for(size_t i = 0; i < n_rows; i++){
                    // clip sprite height if too long
                    if((y_pos_corner + i) > 31){
                        //spdlog::critical("too long! y start: {:d}, i: {:d}", y_pos_corner, i);
                        break;
                    }
                    const uint16_t index = _memory.i_reg() + i;
                    //spdlog::info("index: {:d}", index);
                    const uint8_t sprite = *_memory.ram_offset(index);

                    // width of sprite
                    for(size_t j = 0; j < 8; j++){
                        // clip sprite width if too wide
                        if((x_pos_corner + j) > 63){
                            //spdlog::critical("too wide! x start: {:d}, j: {:d}", x_pos_corner, j);
                            break;
                        }

                        const bool pixel_request = sprite & bit_mask;
                        const bool pixel_status = _display.pixels()[y_pos_corner + i][x_pos_corner + j];

                        // modify pixel buffer
                        if(pixel_request){ // pixel ON requested
                            if(pixel_status){ // display pixel current on
                                // on requested, screen pixel already on - do nothing, set vf flag
                                _display.pixels()[y_pos_corner + i][x_pos_corner + j] = false;
                                _memory.v_reg(0xF, 1);
                            }else{
                                _display.pixels()[y_pos_corner + i][x_pos_corner + j] = true;
                            }
                        }
                        // read next sprite bit
                        bit_mask >>= 1;
                    }
                    // reset bitmask
                    bit_mask = 0b10000000;
                }

                _memory.move_pc();
                break;
            }

        case 0xA:
            {
                // set index register i
                spdlog::info("ANNN");
                const uint16_t nnn = opcode & nnn_mask;
                _memory.i_reg(nnn);
                _memory.move_pc();
                break;
            }

        case 0xB:
            {
                // jump to address nnn + v0
                spdlog::info("BNNN");
                const uint8_t v0 = _memory.v_reg(0);
                const uint16_t  nnn = opcode & nnn_mask;
                //const uint8_t x = (opcode & x_mask) >> 8;
                // v reg value
                //const uint8_t vx = _memory.v_reg(x);

                _memory.pc(nnn + v0);
                //_memory.move_pc();
                break;
            }

        case 0xC:
            {

                // AND random num with nn, put in vx
                spdlog::info("CXNN");
                // v index
                const uint8_t x = (opcode & x_mask) >> 8;
                // immediate value
                const uint8_t nn = opcode & nn_mask;

                const uint8_t random = _memory.random_number();

                _memory.v_reg(x, (random & nn));

                //spdlog::info("\tvx: 0x{:x}", vx);
                //spdlog::info("\tnn: 0x{:x}", nn);
                _memory.move_pc();
                break;
            }

        case 0x1:
            {
                // jump to immediate value nnn
                spdlog::info("1NNN (jump)");
                const uint16_t val = opcode & nnn_mask;
                _memory.pc(val);
                //spdlog::info("1NNN: to location {:x}", val);
                // dont increment pc
                break;
            }

        case 0x2:
            {
                // run subroutine at nnn, cache current pc on stack
                spdlog::info("2NNN (subroutine)");
                const uint16_t  sub_routine = opcode & nnn_mask;

                const uint16_t current_sp = _memory.sp();
                const uint16_t current_pc = _memory.pc();

                spdlog::info("\tstack index: {:d} saving pc: {:d}", current_sp,
                             current_pc);

                // push program counter to stack
                _memory.stack_layer(current_sp, current_pc);
                // increment sp
                _memory.sp(current_sp + 1);
                // fast-forward program counter to subroutine
                // will be executed on next cycle, do not move pc
                _memory.pc(sub_routine);

                spdlog::info("\tnew stack: {:d}, new pc: {:d}", _memory.sp(),
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
                    spdlog::info("\tprevious stack: {:d}, returning to pc: {:d}",
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

                spdlog::info("\tvx: 0x{:x}", vx);
                spdlog::info("\tnn: 0x{:x}", nn);
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
                    spdlog::info("\tskipped");
                    _memory.move_pc();
                }else{
                    spdlog::info("\tnot skipped");
                }

                spdlog::info("\tv{:x}: 0x{:x}", x, vx);
                spdlog::info("\tnn: 0x{:x}", nn);
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

                spdlog::info("\tvx: 0x{:x}", vx);
                spdlog::info("\tvy: 0x{:x}", vy);
                _memory.move_pc();

                break;
            }

        case 0x6:
            {
                // set register to immediate value nn
                const uint8_t x = (opcode & x_mask) >> 8;
                const uint8_t nn = opcode & nn_mask;
                spdlog::info("6XNN");
                spdlog::info("\tx: 0x{:x}", x);
                spdlog::info("\tnn: 0x{:x}", nn);
                _memory.v_reg(x, nn);
                spdlog::info("\tv{:x} set to {:d}", x, nn);
                _memory.move_pc();
                break;
            }

        case 0x7:
            {
                // add immediate value nn to register vx
                const uint8_t x = (opcode & x_mask) >> 8;
                const uint8_t nn = opcode & nn_mask;
                spdlog::info("7XNN");
                spdlog::info("\tx: 0x{:x}", x);
                spdlog::info("\tnn: 0x{:x}", nn);

                const uint16_t vx = _memory.v_reg(x);
                spdlog::info("\tcurrent reg val: 0x{:x}", vx);
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
                    _memory.v_reg(0xF, 0);
                }else if(derivative == 0x2){
                    spdlog::info("0x8XY version 2");
                    _memory.v_reg(x, vx & vy);
                    _memory.v_reg(0xF, 0);
                }else if(derivative == 0x3){
                    spdlog::info("0x8XY version 3");
                    _memory.v_reg(x, vx ^ vy);
                    _memory.v_reg(0xF, 0);
                }else if(derivative == 0x4){
                    spdlog::info("0x8XY version 4");
                    // only keep lower byte, if overflowed
                    _memory.v_reg(x, (uint8_t) (vx + vy));
                    if(vx + vy > 255){
                        // vf carry enable
                        _memory.v_reg(0xF, 1);
                    }else{
                        // vf no carry
                        _memory.v_reg(0xF, 0);
                    }

                }else if(derivative == 0x5){
                    spdlog::info("0x8XY version 5 (subtract)");
                    // only keep lower byte, if underflowed

                    const uint8_t result = (uint8_t) (vx - vy);
                    _memory.v_reg(x, (uint8_t)(vx - vy));

                    spdlog::info("\tsubtract: {:d} - {:d} = {:d}", vy, vy, _memory.v_reg(x));

                    if(vx - vy < 0){
                        _memory.v_reg(0xF, 0);
                    }else{
                        _memory.v_reg(0xF, 1);
                    }

                }else if(derivative == 0x6){
                    spdlog::info("0x8XY version 6");

                    // vy value to x index
                    _memory.v_reg(x, vy);
                    // shift vx right 1 bit
                    const uint8_t bit_mask = 0b00000001;
                    const uint8_t last_bit = vx & bit_mask;

                    // grab new vx again
                    const uint8_t new_vx = _memory.v_reg(x);

                    // shift vx right 1 bit, update index x
                    _memory.v_reg(x, new_vx >> 1);
                    // carry set, depending on value of bit shifted out
                    _memory.v_reg(0xF, last_bit);

                }else if(derivative == 0x7){
                    spdlog::info("0x8XY version 7");
                    // only keep lower byte, if underflowed
                    _memory.v_reg(x, (uint8_t)(vy - vx));
                    if(vy - vx < 0){
                        _memory.v_reg(0xF, 0);
                    }else{
                        _memory.v_reg(0xF, 1);
                    }

                }else if(derivative == 0xE){
                    spdlog::info("0x8XY version E");

                    // vy value to x index
                    _memory.v_reg(x, vy);
                    // grab new vx again
                    const uint8_t new_vx = _memory.v_reg(x);
                    // shift vx right 1 bit
                    const uint8_t bit_mask = 0b10000000;
                    const uint8_t last_bit = (new_vx & bit_mask) >> 7;

                    //spdlog::info("vx: {:x}", vx);
                    uint8_t vx_left_shift = new_vx << 1;
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

                spdlog::info("\tx: 0x{:x}", x);
                spdlog::info("\ty: 0x{:x}", y);

                spdlog::info("\tvx: 0x{:x}", vx);
                spdlog::info("\tvy: 0x{:x}", vy);
                _memory.move_pc();
                break;
            }

        case 0xE:{

            const uint8_t x = (opcode & x_mask) >> 8;
            const uint8_t nn = opcode & nn_mask;
            // vx reg value
            const uint8_t vx = _memory.v_reg(x);

            if(nn == 0x9E){
                spdlog::info("0xEX 9E");
                spdlog::info("\tkey press check: {:x}", vx);

                bool key_status = _input.is_pressing((Input::Keys)vx);


                if(key_status){
                    // pressed, skip next instruction
                    _memory.move_pc();
                    spdlog::info("\tpressed");
                }else{
                    spdlog::info("\tnot pressed");

                }

            }else if(nn == 0xA1){
                spdlog::info("0xEX A1");
                spdlog::info("key press check: {:x}", vx);

                bool key_status = _input.is_pressing((Input::Keys)vx);

                if(!key_status){
                    // not pressed, skip next instruction
                    _memory.move_pc();
                }

            }else{
                std::string err{"ERROR: bad 0xEX?? instruction"};
                spdlog::error(err);
                throw std::runtime_error{err};
            }

            _memory.move_pc();
            break;
        }

        case 0xF:
            {
                const uint8_t x = (opcode & x_mask) >> 8;
                const uint8_t nn = opcode & nn_mask;
                const uint16_t start = _memory.i_reg();

                if(nn == 0x65){
                    spdlog::info("0xFX 65");
                    // copy contiguous ram starting at i to registers [0, x]

                    for(size_t i = 0; i <= x; i++){
                        const uint8_t& location = _memory.ram(start + i);
                        _memory.v_reg(i, location);
                        //_memory.i_reg(start + i);
                    }
                    // TODO: make this quark optional
                    _memory.i_reg(start + 1 + x);


                }else if(nn == 0x55){
                    spdlog::info("0xFX 55");
                    // copy register values [0, x] to ram contiguous, starting at i

                    for(size_t i = 0; i <= x; i++){
                        uint8_t& location = _memory.ram(start + i);
                        location = _memory.v_reg(i);
                        //_memory.i_reg(start + i);
                    }
                    // TODO: make this quark optional
                    _memory.i_reg(start + 1 + x);


                }else if(nn == 0x33){
                    // binary to decimal conversion
                    spdlog::info("0xFX 33");
                    const uint8_t vx = _memory.v_reg(x);

                    // get digit of vx
                    const uint8_t digit[3] = {(uint8_t) (vx / 100),
                                              (uint8_t)((vx % 100) / 10),
                                              (uint8_t) (vx % 10)};
                    // assign each digit to i contiguous
                    for(size_t i = 0; i < 3; i++){
                        uint8_t& location = _memory.ram(start + i);
                        location = digit[i];
                    }


                }else if(nn == 0x29){
                    // load font
                    spdlog::info("0xFX 29");

                    const uint8_t font_offset = _memory.v_reg(x);
                    const uint16_t font_location = 0x050 + (font_offset * 5);

                    /*spdlog::critical("index: {:d}, font: {:d}, timer: {:d}",
                                     font_offset,
                                     font_location,
                                     _memory.delay_timer());*/

                    _memory.i_reg(font_location);

                }else if(nn == 0x1E){
                    spdlog::info("0xFX 1E");

                    const uint8_t vx = _memory.v_reg(x);
                    const uint16_t i = _memory.i_reg();

                    const uint16_t sum = vx + i;

                    // WARNING: original COSMAC Chip didn't do this
                    // but some chip8 games rely on this
                    if(sum > 0xFFF){
                        // outside the normal addressing range
                        _memory.v_reg(0xF, 1);
                    }

                    // add vx to current it
                    _memory.i_reg(sum);

                }else if(nn == 0x07){
                    // set vx to delay timer value
                    spdlog::info("0xFX 07");
                    const uint8_t dt = _memory.delay_timer();
                    _memory.v_reg(x, dt);

                }else if(nn == 0x15){
                    // set delay timer to vx
                    spdlog::info("0xFX 15");
                    // vx reg value
                    const uint8_t vx = _memory.v_reg(x);
                    _memory.delay_timer(vx);

                }else if(nn == 0x18){
                    spdlog::info("0xFX 18");

                    // vx reg value
                    const uint8_t vx = _memory.v_reg(x);
                    _memory.sound_timer(vx);

                }else if(nn == 0x0A){
                    //spdlog::info("0xFX 0A");

                    Input::Keys pressed_key = _input.get_pressed_key();

                    if(_input.is_not_pressed() ||  !_input.is_released()){
                        // no input or not released this cycle, repeat instruction
                        //spdlog::info("\tnot ready to increment");
                        _memory.move_back_pc();
                    }else{
                        // input released, assign to vx

                        Input::Keys released_key = _input.released_key_value();
                        //spdlog::info("\treleased key: {:x}", (uint8_t) released_key);
                        _memory.v_reg(x, (uint8_t) released_key);
                        _input.reset_key_stage();
                        //spdlog::info("\tpressed + released");
                    }

                }else{
                    std::string err{"ERROR: bad 0xF??? instruction "};
                    spdlog::error(err);
                    throw std::runtime_error{err};
                }

                _memory.move_pc();
                break;
            }

        default:
            spdlog::critical("bad instruction or data: 0x{:x}", opcode);
        }

    }

    void load_rom_to_ram(const char* file_path){
        spdlog::info("Loading into ram from rom.");
        // clear pixel buffer
        _display.clear();
        // zero out registers for fresh start
        _memory.erase_memory(false); // false since it'll be overwritten anyway
        // load rom in binary
        std::ifstream rom{file_path, std::ios::binary};
        if(!rom.is_open()){
            // error
            std::string err{"\tERROR: failed to open file "};
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

    Input& input(){
        return _input;
    }


    uint16_t get_instruction(){
        return instruction_set;
    }


    uint8_t get_first_half(){
        return first_half;
    }


    uint8_t get_second_half(){
        return second_half;
    }

    bool is_draw_instruction(){
        const uint8_t command_nibble = (instruction_set & 0xF000) >> 12;
        return instruction_set == 0x00E0 || command_nibble == 0xD;
    }


private:
    Display _display{1.0, BLACK, RED}; // graphics helpers
    Memory _memory{};      // internal chip8 memory
    Input _input{};
    uint16_t instruction_set;
    uint8_t first_half;
    uint8_t second_half;
    Timer timer_60Hz{60.0};
    Timer cycle_timer{500.0};

};
