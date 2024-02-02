#include <fstream>
#include <filesystem>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "memory.h"
#include "display.h"

class Chippie {
public:

    void execute_opcode(uint8_t first, uint8_t second){

    }

    void load_rom_to_ram(const char* file_path){
        spdlog::info("Loading into ram from rom.");
        // zero out registers for fresh start
        memory.erase_memory(false); // false since it'll be overwritten anyway
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
        rom.read((char*)memory.get_ram_rom_start_location(), rom_size);
        // close rom file
        rom.close();
    }

    void render(){
        display.render();
    }

    // for testing
    Memory& _get_memory(){
        return memory;
    }

private:
    Display display{1.0}; // graphics helpers
    Memory memory{};      // internal chip8 memory
};
