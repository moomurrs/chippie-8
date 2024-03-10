#include <raylib.h>
#include <array>
#include <cstdint>
#include <algorithm>
#include <spdlog/spdlog.h>
#include "timer.h"

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

    // render screen pixels, according to pixel buffer data
    void render(){
        //ClearBackground(DARKGRAY);
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

    void render_pixel(int x, int y, bool is_on){
        if(x < 0 || x > 64){
            std::string err{"ERROR: pixel x out of range "};
            spdlog::error(err);
            throw std::runtime_error{err};
        }
        if(y < 0 || y > 32){
            std::string err{"ERROR: pixel y out of range "};
            spdlog::error(err);
            throw std::runtime_error{err};
        }

        Color color = is_on ? pixel_color : background_color;

        //spdlog::info("rendering pixel: ({:d},{:d}), color: {:d}", x, y, a);

        DrawRectangle(width_offset + (pixel_width * x) + (padding * x),
                      height_offset + (pixel_height * y) + (padding * y),
                      pixel_width, pixel_height,
                      color);
    }

    // clear entire screen
    void clear(){
        //if(!timer.is_timer_done()) return
        std::fill(std::begin(pixel_buffer.at(0)), std::end(pixel_buffer.at(31)), 0);
        render_all(background_color);
        /*
        DrawRectangle(width_offset,
                      height_offset,
                      (64 * pixel_width) + (padding * 64 - 1),
                      (32 * pixel_height) + (padding * 32 - 1),
                      background_color);*/
        //timer.start_timer();

    }

    std::array<std::array<bool, 64>, 32>& pixels(){
        return pixel_buffer;
    }

    // get bottom of pixel of screen
    int bottom_pixel(int offset){
        return height_offset + (pixel_height * 32) + (padding * 32) + offset;
    }

    // get left most pixel side of screen
    int left_pixel(int offset){
        return offset;
    }


private:
    float magnifier = 1.0f; // scaling

    const int padding = 0; // spacing between pixel blocks

    const int height_offset = 20;
    const int width_offset = 20;

    const float pixel_height = 10 * magnifier;
    const float pixel_width = 10 * magnifier;

    std::array<std::array<bool, 64>, 32> pixel_buffer{}; // 32 rows, 64 cols

    const Color background_color;
    const Color pixel_color;

};
