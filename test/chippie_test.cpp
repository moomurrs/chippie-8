#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>
#include "../include/chippie.h"

void print_char_hex(uint8_t decimal){
    std::cout << "byte: 0x" << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (0xFF & decimal) << std::dec << "\n";
}

TEST_CASE("memory ram test"){
    Chippie chippie{};
    // make sure a non-existant file throws error
    REQUIRE_THROWS_AS(chippie.load_rom_to_ram("./dummy-test"),
                      std::runtime_error);
    chippie.load_rom_to_ram("./1-chip8-logo.ch8");

    // check if the 2nd byte in ram matches rom hex 0XE0
    uint8_t* byte{};
    byte = chippie._get_memory()._get_rom_offset(1);
    REQUIRE(*byte == 0xE0);

    // check if the last byte in ram matches last rom byte
    byte = chippie._get_memory()._get_rom_offset(259);
    REQUIRE(*byte == 0x70);
}

TEST_CASE("???"){

}
