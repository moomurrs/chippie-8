#include <raylib.h>

class Timer {

public:
    void start_timer(){
        start_time = GetTime();
    }
    bool is_timer_done(){
        return GetTime() - start_time >= duration;
    }
    double get_elapsed(){
        return GetTime() - start_time;
    }
private:
    double start_time = 0.0;
    const double duration = 0.01667;
};
