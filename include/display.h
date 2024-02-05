#include <raylib.h>
#include <array>

class Display {
public:

    Display() = delete;
    Display(float mag) : magnifier(mag) {}

    constexpr static auto background_color = DARKGRAY;
    constexpr static auto pixel_color = BLACK;

    // turn on all pixels
    void render(){
        for(std::size_t i = 0; i < 32; i++){
            for(std::size_t j = 0; j < 64; j++){
                DrawRectangle(width_offset + pixel_width * j + (padding * j),
                              height_offset + pixel_height * i + (padding * i),
                              pixel_width, pixel_height,
                              pixel_color);
            }
        }
    }

    // clear entire screen
    void clear(){
        DrawRectangle(width_offset,
                      height_offset,
                      (64 * pixel_width) + (padding * 64 - 1),
                      (32 * pixel_height) + (padding * 32 - 1),
                      background_color);
    }

private:
    float magnifier = 1.0f; // scaling

    const int padding = 1; // spacing between pixel blocks

    const int height_offset = 20;
    const int width_offset = 20;

    const float pixel_height = 10 * magnifier;
    const float pixel_width = 10 * magnifier;

    std::array<std::array<bool, 64>, 32> pixels{}; // 32 rows, 64 cols

};
