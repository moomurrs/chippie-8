#include <raylib.h>
#include <spdlog/spdlog.h>
#include <array>
#include <map>
#include <iostream>

class Input{
public:
    // key enum (must remap to raylib)
    enum class Keys{
        ZERO,
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        A,
        B,
        C,
        D,
        E,
        F,
        NONE
    };

    // return the status of the supplied keypad
    // used by EX?? input instructions
    bool is_pressing(Keys key){
        if((int)key < 0 || (int) key > 16){
            std::string err{"ERROR: key enum out of range (are you using key raylib values?)"};
            spdlog::error(err);
            throw std::runtime_error{err};
        }
        return IsKeyDown(key_map.at((int)key));
    }

    Keys get_pressed_key(){
        // consumes pressed key from queue
        // subsequent calls will be blank (even if pressing)
        int raylib_key = GetKeyPressed();

        Keys key = Keys::NONE;

        for(const auto& [index_key, value_raylib] : key_map){
            if(value_raylib == raylib_key){
                // raylib -> Keys mapping match found
                key = (Input::Keys)index_key;

                if(key == Keys::NONE){
                    // key is either not pressed at all, or pressed but not released
                    if(key_stage == 1){
                        // key is held
                        if(IsKeyUp(key_map.at((int)released_key))){
                            // key is released, move to last stage
                            key_stage = 2;
                        }
                    }
                }else{
                    // key is pressed
                    key_stage = 1;
                    released_key = key;
                }
            }
        }
        return key;
    }

    int get_key_stage(){
        return key_stage;
    }

    Keys released_key_value(){
        return released_key;
    }

    bool is_not_pressed(){
        return key_stage == 0;
    }

    bool is_released(){
        return key_stage == 2;
    }

    void reset_key_stage(){
        key_stage = 0;
        released_key = Keys::NONE;
    }

private:
    // remap 0-15 to raylib key values
    // remap the second value for each entry to remap as needed
    std::map<int, int> key_map= {
        {(int)Keys::ONE, KEY_ONE}, {(int)Keys::TWO, KEY_TWO}, {(int)Keys::THREE, KEY_THREE}, {(int)Keys::C, KEY_FOUR},
        {(int)Keys::FOUR, KEY_Q}, {(int)Keys::FIVE, KEY_W}, {(int)Keys::SIX, KEY_E}, {(int)Keys::D, KEY_R},
        {(int)Keys::SEVEN, KEY_A}, {(int)Keys::EIGHT, KEY_S}, {(int)Keys::NINE, KEY_D}, {(int)Keys::E, KEY_F},
        {(int)Keys::A, KEY_Z}, {(int)Keys::ZERO, KEY_X}, {(int)Keys::B, KEY_C}, {(int)Keys::F, KEY_V},
        {16, KEY_NULL} // no press
    };

    Keys released_key = Keys::NONE;
    // 0: not pressed
    // 1: pressing
    // 2: released
    int key_stage = 0;
};
