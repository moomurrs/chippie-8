#include <array>
#include <cstdint>
#include <filesystem>
#include <algorithm>
#include <spdlog/spdlog.h>

class Memory {
public:

    /* get byte value from rom with offset */
    uint8_t* _get_rom_offset(uint32_t offset){
        return get_ram_rom_start_location() + offset;
    }

    /* return the start address of where rom begins
     * which is 0x200 (or item index 512) */
    uint8_t* get_ram_rom_start_location(){
        return &ram[512];
    }

    /* zero out all registers
       ram erased only if flagged  */
    void erase_memory(bool erase_ram){
        spdlog::info("erasing memory");
        program_counter = 0;
        index_register = 0;
        stack_pointer = 0;
        delay_timer = 255;
        sound_timer = 255;
        for(size_t i = 0; i < 16; i++){
            stack.at(i) = 0;
            vregisters.at(i) = 0;
        }
        // only manually erase ram when flagged
        if(erase_ram){
            spdlog::info("|___erasing memory");
            std::fill(std::begin(ram), std::end(ram), 0);
        }
    }

    Memory(){
        /* load dummy bytes into ram to detect
           if rom load failed (for diagnostics only) */
        ram.at(512) = 0xFF;
        ram.at(513) = 0xAB;
    }

    const uint16_t& _get_program_counter(){
        return program_counter;
    }

    void _set_program_counter(int32_t pc){
        if(pc < 0 || pc > 65535){
            std::string err{"ERROR: new pc outside range. "};
            spdlog::critical(err);
            throw std::runtime_error{err};
            return;
        }
        spdlog::info("old pc: %u, new pc: %u", program_counter, pc);
        program_counter = pc;
    }

    const uint16_t& _get_index_register(){
        return index_register;
    }

    const uint8_t& _get_stack_pointer(){
        return stack_pointer;
    }

    void _set_stack_pointer(int16_t sp){
        if(sp < 0 || sp > 255){
            std::string err{"ERROR: new sp outside range. "};
            spdlog::critical(err);
            throw std::runtime_error{err};
            return;
        }
        stack_pointer = sp;
    }

    const uint8_t& _get_delay_timer(){
        return delay_timer;
    }

    void reset_delay_timer(){
        delay_timer = 255;
    }

    const uint8_t& _get_sound_timer(){
        return sound_timer;
    }

    void reset_sound_timer(){
        sound_timer = 255;
    }

    const uint8_t& _get_vregister(int32_t index){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad vregister get index "};
            spdlog::critical(err);
            throw std::runtime_error{err};
        }
        return vregisters.at(index);
    }

    void _set_vregister(int32_t index, int32_t value){
        if(index < 0 ||  index > 16){
            std::string err{"ERROR: bad vregister get index "};
            spdlog::critical(err);
            throw std::runtime_error{err};
        }
        if(value < 0 ||  value > 255){
            std::string err{"ERROR: bad vregister set value "};
            spdlog::critical(err);
            throw std::runtime_error{err};
        }
        vregisters.at(index) = value;
    }

private:
    /* ram address space is 0x000 - 0xFFF
       0x000 - 0x1FF: not used
       0x050 - 0x0A0: storage space for built-in characters 0-F
       0x200 - 0xFFF: instructions from ROM, after ROM is free to use */

    std::array<uint8_t, 4096> ram{};

    /* opcodes are 2 bytes long but we can only address
       1 bytes at a time from memory. So, to make a complete opcode,
       grab a byte from PC, then PC+1 to make a full opcode.
       Make sure to increment the PC by 2. */

    uint16_t program_counter = 0; // points to the next instruction to be executed
    uint16_t index_register = 0;  // store memory address for in-use operations
    std::array<uint16_t, 16> stack{}; // stack is 16 layers deep, keeping track of new and old PC
    uint8_t stack_pointer = 0;
    uint8_t delay_timer = 255;    // decremented 60 times per sec until 0
    uint8_t sound_timer = 255;    //
    /* general purpose variable registers */
    std::array<uint8_t, 16> vregisters{};
};