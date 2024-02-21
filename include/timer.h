#include <raylib.h>

class Timer {

public:
    Timer() = delete;
    Timer(double frequency) : delta(1.0 / frequency) {
        reset_timer();
    }
    void reset_timer(){
        start_time = GetTime();
    }
    bool is_timer_done(){
        return GetTime() - start_time >= delta;
    }
    double get_elapsed(){
        return GetTime() - start_time;
    }
private:
    double start_time = 0.0;
    const double delta;
};
