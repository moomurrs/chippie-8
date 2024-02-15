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
        F
    };

    // return raylib key that was pressed
    /*
    int get_input(){
        for(size_t i = 0; i < 16; i++){
            if(input.at(i)){
                return key_map.at(i);
            }
        }
        // input buffer false, no input
        return KEY_NULL;
        }*/

    bool is_pressing(Keys key){
        if((int)key < 0 || (int) key > 15){
            std::string err{"ERROR: key enum out of range (are you using key raylib values?)"};
            spdlog::error(err);
            throw std::runtime_error{err};
        }
        return input.at((int)key);
    }

    // for testing: print values from input buffer
    void print_keys(){
        update_input();
        for(size_t i = 0; i < 16; i++){
            if(input.at(i)){
                //spdlog::info("{:d}: 1", i);
                std::cout << i << ": " << "y" << " ";
            }else{
                //spdlog::info("{:d}:", i);
                std::cout << i << ": " << " " << " ";
            }
        }
        std::cout << "\n";
    }

    // query button press and update input buffer
    void update_input(){
        // for each key, check if it was pressed or released
        for(size_t i = 0; i < 16; i++){
            if(IsKeyPressed(key_map.at(i))){
                spdlog::info("input {:d} or 0x{:x}: yes", i, i);
                input.at(i) = true;
            }else if(IsKeyReleased(key_map.at(i))){
                //spdlog::info("input {:d} or 0x{:x}:  no", i, i);
                input.at(i) = false;
            }
        }
    }



private:
    // input buffer to track button presses
    std::array<bool, 16> input{};
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
        {15, KEY_F}
    };

};
