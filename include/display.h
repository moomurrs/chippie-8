#include <raylib.h>
#include <array>
#include <cstdint>
#include <algorithm>
#include <spdlog/spdlog.h>

class Display {
public:

    Display() = delete;
    Display(float mag, Color background, Color pixel) :
        magnifier(mag),
        background_color(background),
        pixel_color(pixel)
        {
            // draw the blank pixels on startup
            spdlog::info("display: initializing...");
        }

    // turn on all pixels
    void render_all(Color color){
        for(std::size_t i = 0; i < 32; i++){
            for(std::size_t j = 0; j < 64; j++){
                DrawRectangle(width_offset + pixel_width * j + (padding * j),
                              height_offset + pixel_height * i + (padding * i),
                              pixel_width, pixel_height,
                              color);
            }
        }
    }

    void render(){
        for(std::size_t i = 0; i < 32; i++){
            for(std::size_t j = 0; j < 64; j++){
                if(pixel_buffer[i][j]){
                    DrawRectangle(width_offset + pixel_width * j + (padding * j),
                                  height_offset + pixel_height * i + (padding * i),
                                  pixel_width, pixel_height,
                                  pixel_color);
                }else{
                    DrawRectangle(width_offset + pixel_width * j + (padding * j),
                                  height_offset + pixel_height * i + (padding * i),
                                  pixel_width, pixel_height,
                                  background_color);
                }
            }
        }
    }

    // clear entire screen
    void clear(){
        std::fill(std::begin(pixel_buffer.at(0)), std::end(pixel_buffer.at(31)), 0);
        render_all(background_color);
        /*
        DrawRectangle(width_offset,
                      height_offset,
                      (64 * pixel_width) + (padding * 64 - 1),
                      (32 * pixel_height) + (padding * 32 - 1),
                      background_color);*/
    }

    std::array<std::array<bool, 64>, 32>& pixels(){
        return pixel_buffer;
    }


private:
    float magnifier = 1.0f; // scaling

    const int padding = 1; // spacing between pixel blocks

    const int height_offset = 20;
    const int width_offset = 20;

    const float pixel_height = 10 * magnifier;
    const float pixel_width = 10 * magnifier;

    std::array<std::array<bool, 64>, 32> pixel_buffer{}; // 32 rows, 64 cols

    const Color background_color;
    const Color pixel_color;

};
