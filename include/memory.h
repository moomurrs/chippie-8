#include <array>
#include <cstdint>

class Memory {
public:
    int a;
private:
    /* ram address space is 0x000 - 0xFFF
       0x000 - 0x1FF: not used
       0x050 - 0x0A0: storage space for built-in characters 0-F
       0x200 - 0xFFF: instructions from ROM, after ROM is free to use */

    std::array<uint8_t, 4096> ram{};

    /* opcodes are 2 bytes long but we can only address
       1 bytes at a time from memory. So, to make a complete opcode,
       grab a byte from PC, then PC+1 to make a full opcode.
       Make sure to increment the PC by 2.
*/


    uint16_t program_counter = 0; // points to the next instruction to be executed
    uint16_t index_register = 0;  // store memory address for in-use operations
    std::array<uint16_t, 16> stack{}; // stack is 16 layers deep, keeping track of new and old PC
    uint8_t delay_timer = 255;    // decremented 60 times per sec until 0
    uint8_t sound_timer = 255;    //

    /* general purpose variable registers */
    std::array<uint8_t, 16> vregisters{};


};
