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
    std::map<int, int> key_map= {
        {0, KEY_ZERO},
        {1, KEY_ONE},
        {2, KEY_TWO},
        {3, KEY_THREE},
        {4, KEY_FOUR},
        {5, KEY_FIVE},
        {6, KEY_SIX},
        {7, KEY_SEVEN},
        {8, KEY_EIGHT},
        {9, KEY_NINE},
        {10, KEY_A},
        {11, KEY_B},
        {12, KEY_C},
        {13, KEY_D},
        {14, KEY_E},
        {15, KEY_F},
        {16, KEY_NULL} // no press
    };

    Keys released_key = Keys::NONE;
    // 0: not pressed
    // 1: pressing
    // 2: released
    int key_stage = 0;
};
