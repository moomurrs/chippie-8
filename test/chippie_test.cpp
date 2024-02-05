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

    // manually erase all memory
    chippie._get_memory().erase_memory(true);
    // verify ram is zeroed by checking 1st and ram bytes are 0
    REQUIRE(*(chippie._get_memory().get_ram_rom_start_location()) == 0);
    REQUIRE(*(chippie._get_memory().get_ram_rom_start_location() + 3583) == 0);
    // verify pc and sp are zeroed
    REQUIRE(chippie._get_memory()._get_program_counter() == 0);
    REQUIRE(chippie._get_memory()._get_stack_pointer() == 0);
    // verify timers are reset
    REQUIRE(chippie._get_memory()._get_delay_timer() == 255);
    REQUIRE(chippie._get_memory()._get_sound_timer() == 255);

    // verify pc sets properly
    chippie._get_memory()._set_program_counter(5);
    REQUIRE(chippie._get_memory()._get_program_counter() == 5);

    // verify sp sets properly
    chippie._get_memory()._set_stack_pointer(15);
    REQUIRE(chippie._get_memory()._get_stack_pointer() == 15);

    // verify pc cannot be set outside valid range
    REQUIRE_THROWS(chippie._get_memory()._set_program_counter(70000));
    REQUIRE_THROWS(chippie._get_memory()._set_program_counter(-1));

    // verify sp cannot be set outside valid range
    REQUIRE_THROWS(chippie._get_memory()._set_stack_pointer(256));
    REQUIRE_THROWS(chippie._get_memory()._set_stack_pointer(-1));
    // verify setting and getting vreg
    chippie._get_memory()._set_vregister(0, 100);
    REQUIRE(chippie._get_memory()._get_vregister(0) == 100);
    // error out on bad vreg index
    REQUIRE_THROWS(chippie._get_memory()._get_vregister(-1));
    REQUIRE_THROWS(chippie._get_memory()._get_vregister(16));

    REQUIRE_THROWS(chippie._get_memory()._set_vregister(-1, 100));
    REQUIRE_THROWS(chippie._get_memory()._set_vregister(16, 100));
    REQUIRE_THROWS(chippie._get_memory()._set_vregister(0, 256));
    REQUIRE_THROWS(chippie._get_memory()._set_vregister(0, -1));


}
