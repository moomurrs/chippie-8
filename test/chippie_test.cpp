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
    const uint8_t* byte{};
    byte = chippie.memory()._get_rom_offset(1);

    REQUIRE(*byte == 0xE0);

    // check if the last byte in ram matches last rom byte
    byte = chippie.memory()._get_rom_offset(259);
    REQUIRE(*byte == 0x70);

    // manually erase all memory
    chippie.memory().erase_memory(true);
    // verify ram is zeroed by checking 1st and ram bytes are 0
    REQUIRE(*(chippie.memory().get_ram_rom_start_location()) == 0);
    REQUIRE(*(chippie.memory().get_ram_rom_start_location() + 3583) == 0);
    // verify pc and sp are zeroed
    REQUIRE(chippie.memory()._get_program_counter() == 0);
    REQUIRE(chippie.memory()._get_stack_pointer() == 0);
    // verify timers are reset
    REQUIRE(chippie.memory()._get_delay_timer() == 255);
    REQUIRE(chippie.memory()._get_sound_timer() == 255);

    // verify pc sets properly
    chippie.memory()._set_program_counter(5);
    REQUIRE(chippie.memory()._get_program_counter() == 5);

    // verify sp sets properly
    chippie.memory()._set_stack_pointer(15);
    REQUIRE(chippie.memory()._get_stack_pointer() == 15);

    // verify index reg sets properly
    chippie.memory()._set_index_register(1001);
    REQUIRE(chippie.memory()._get_index_register() == 1001);

    // verify pc cannot be set outside valid range
    REQUIRE_THROWS(chippie.memory()._set_program_counter(70000));
    REQUIRE_THROWS(chippie.memory()._set_program_counter(-1));

    // verify sp cannot be set outside valid range
    REQUIRE_THROWS(chippie.memory()._set_stack_pointer(256));
    REQUIRE_THROWS(chippie.memory()._set_stack_pointer(-1));
    // verify setting and getting vreg
    chippie.memory()._set_vregister(0, 100);
    REQUIRE(chippie.memory()._get_vregister(0) == 100);
    // error out on bad vreg index
    REQUIRE_THROWS(chippie.memory()._get_vregister(-1));
    REQUIRE_THROWS(chippie.memory()._get_vregister(16));

    REQUIRE_THROWS(chippie.memory()._set_vregister(-1, 100));
    REQUIRE_THROWS(chippie.memory()._set_vregister(16, 100));
    REQUIRE_THROWS(chippie.memory()._set_vregister(0, 256));
    REQUIRE_THROWS(chippie.memory()._set_vregister(0, -1));

    chippie.memory()._set_stack(0, 200);
    REQUIRE(chippie.memory()._get_stack(0) == 200);

    REQUIRE_THROWS(chippie.memory()._get_stack(-1));
    REQUIRE_THROWS(chippie.memory()._get_stack(16));

    REQUIRE_THROWS(chippie.memory()._set_stack(-1, 300));
    REQUIRE_THROWS(chippie.memory()._set_stack(16, 300));
    REQUIRE_THROWS(chippie.memory()._set_stack(0, -1));
    REQUIRE_THROWS(chippie.memory()._set_stack(0, 70000));

    REQUIRE_THROWS(chippie.memory()._get_ram(-1));
    REQUIRE_THROWS(chippie.memory()._get_ram(4096));

    REQUIRE_THROWS(chippie.memory()._set_index_register(-1));
    REQUIRE_THROWS(chippie.memory()._set_index_register(4096));


}

TEST_CASE("font Test"){
    Chippie chippie{};

    // probe a couple font data
    REQUIRE(chippie.memory().get_font_byte(0) == 0xF0); // first font byte
    REQUIRE(chippie.memory().get_font_byte(79) == 0x80); // last font byte

    REQUIRE_THROWS(chippie.memory().get_font_byte(-1));
    REQUIRE_THROWS(chippie.memory().get_font_byte(80));

}
