#include <raylib.h>
#include <array>

class Display {
public:

    Display() = delete;
    Display(int mag) : magnifier(mag) {}

    void render(){
        for(std::size_t i = 0; i < 32; i++){
            for(std::size_t j = 0; j < 64; j++){
                DrawRectangle(width_offset + pixel_width * j, height_offset + pixel_height * i, pixel_width, pixel_height, BLACK);
            }
        }
    }

private:
    int magnifier = 1;

    const int height_offset = 10;
    const int width_offset = 10;

    const int pixel_height = 10;
    const int pixel_width = 10;

    std::array<std::array<bool, 64>, 32> pixels{}; // 32 rows, 64 cols

};
