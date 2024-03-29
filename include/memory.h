#include <array>
#include <cstdint>
#include <filesystem>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <random>
#include <raylib.h>

class Memory {
public:


    /* get byte value from rom with offset */
    const uint8_t* _get_rom_offset(uint32_t offset){
        return &ram_memory[512 + offset];
    }

    /* return the start address of where rom begins
     * which is 0x200 (or item index 512) */
    const uint8_t* get_ram_rom_start_location(){
        return &ram_memory[512];
    }

    const uint8_t* ram_offset(uint32_t offset){
        return &ram_memory[offset];
    }

    /* zero out all registers
       ram erased only if flagged  */
    void erase_memory(bool erase_ram){
        spdlog::info("erasing memory");
        program_counter = 0x200;
        index_register = 0;
        stack_pointer = 0;
        delay_timer_count = 0;
        sound_timer_count = 0;
        for(size_t i = 0; i < 16; i++){
            stack.at(i) = 0;
            vregisters.at(i) = 0;
        }
        // only manually erase ram when flagged
        if(erase_ram){
            spdlog::info("|___erasing memory");
            std::fill(std::begin(ram_memory), std::end(ram_memory), 0);
        }
    }

    // seed random generator on creation
    Memory() : random_generator(time(nullptr)) {
        InitAudioDevice();
        beep = LoadSound("../assets/beep.wav");
    }

    ~Memory(){
        UnloadSound(beep);
        CloseAudioDevice();
    }


    const uint16_t& pc(){
        return program_counter;
    }

    void pc(int32_t new_pc){
        if(new_pc < 0 || new_pc > 65535){
            std::string err{"ERROR: new pc outside range "};
            spdlog::critical(err + ": {:d}", new_pc);
            throw std::runtime_error{err};
            return;
        }
        //spdlog::info("SETTING: old pc: {:d}, new pc: {:d}", program_counter, new_pc);
        program_counter = new_pc;
    }

    const uint16_t& i_reg(){
        return index_register;
    }

    void i_reg(int32_t ireg){
        if(ireg < 0 || ireg > 4095){
            std::string err{"ERROR: new index reg outside range "};
            spdlog::critical(err + ": {:d}", ireg);
            throw std::runtime_error{err};
            return;
        }
        //spdlog::info("SETTING: old i: {:d}, new i: {:d}", index_register, ireg);
        index_register = ireg;
    }

    const uint8_t& sp(){
        return stack_pointer;
    }

    void sp(int16_t new_sp){
        if(new_sp < 0 || new_sp > 255){
            std::string err{"ERROR: new sp outside range "};
            spdlog::critical(err + ": {:d}", new_sp);
            throw std::runtime_error{err};
        }
        spdlog::info("\told sp: {:d}, new sp: {:d}", stack_pointer, new_sp);
        stack_pointer = new_sp;
    }

    const uint8_t& delay_timer(){
        return delay_timer_count;
    }

    void delay_timer(int new_time){
        if(new_time < 0 || new_time > 255){
            std::string err{"ERROR: new delay timer outside range "};
            spdlog::critical(err + ": {:d}", new_time);
            throw std::runtime_error{err};
        }
        delay_timer_count = new_time;
    }

    const uint8_t& sound_timer(){
        return sound_timer_count;
    }

    void sound_timer(int new_time){
        if(new_time < 0 || new_time > 255){
            std::string err{"ERROR: new sound timer outside range "};
            spdlog::critical(err + ": {:d}", new_time);
            throw std::runtime_error{err};
        }

        if(new_time > 0){
            // TODO: play beep
            spdlog::info("playing beep");
        }else{
            // stop beep
            spdlog::info("stopping beep");
        }

        sound_timer_count = new_time;
    }

    void play_sound(){
        if(!IsSoundPlaying(beep)){
            PlaySound(beep);
        }
    }

    void move_pc(){
        //spdlog::info("\tincrement: old pc: {:d}, new pc: {:d}", program_counter, program_counter + 2);
        program_counter += 2;
    }

    void move_back_pc(){
        //spdlog::info("\tincrement back: old pc: {:d}, new pc: {:d}", program_counter, program_counter - 2);
        program_counter -= 2;
    }

    const uint8_t& v_reg(int32_t index){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad vregister get index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        return vregisters.at(index);
    }

    void v_reg(int32_t index, int32_t value){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad vregister get index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        if(value < 0 ||  value > 255){
            std::string err{"ERROR: bad vregister set value "};
            spdlog::critical(err + "{:d}", value);
            throw std::runtime_error{err};
        }
        vregisters.at(index) = value;
    }

    const uint16_t& stack_layer(int32_t index){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad stack depth index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        return stack.at(index);
    }

    void stack_layer(int32_t index, int32_t value){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad stack depth index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        if(value < 0 ||  value > 65535){
            std::string err{"ERROR: bad stack set value "};
            spdlog::critical(err + "{:d}", value);
            throw std::runtime_error{err};
        }
        stack.at(index) = value;
    }

    uint8_t random_number(){
        return random_generator() % (255 + 1);
    }

    uint8_t& ram(int32_t index){
        if(index < 0 ||  index > 4095){
            std::string err{"ERROR: bad ram get index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        return ram_memory.at(index);
    }

    uint8_t& font_byte(int32_t index){
        uint16_t font_start = 0x050;
        if(index < 0 ||  index > 79){
            std::string err{"ERROR: bad font get index "};
            spdlog::critical(err + "{:d}", index);
            throw std::runtime_error{err};
        }
        return ram(font_start + index);
    }

private:
    /* ram address space is 0x000 - 0xFFF
       0x000 - 0x1FF: not used
       0x050 - 0x0A0: storage space for built-in characters 0-F
       0x200 - 0xFFF: instructions from ROM, after ROM is free to use */

    std::array<uint8_t, 4096> ram_memory{
 /*0x000*/  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /*-----*/
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* not */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* used*/
            0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,           /*-----*/
 /*0x050*/  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 /*-------*/
            0x20, 0x60, 0x20, 0x20, 0x70, // 1 /* sprite*/
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2 /* fonts */
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3 /*-------*/
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    /* opcodes are 2 bytes long but we can only address
       1 bytes at a time from memory. So, to make a complete opcode,
       grab a byte from PC, then PC+1 to make a full opcode.
       Make sure to increment the PC by 2. */

    uint16_t program_counter = 0x200; // points to the next instruction to be executed
    uint16_t index_register = 0;  // store memory address for in-use operations
    std::array<uint16_t, 16> stack{}; // stack is 16 layers deep, keeping track of new and old PC
    uint8_t stack_pointer = 0;
    uint8_t delay_timer_count = 0;    // decremented 60 times per sec until 0
    uint8_t sound_timer_count = 0;    //
    /* general purpose variable registers */
    std::array<uint8_t, 16> vregisters{};

    std::mt19937 random_generator;

    Sound beep;

};
