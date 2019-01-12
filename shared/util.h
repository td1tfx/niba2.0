#pragma once

#include <chrono>

namespace nibautil {

class stopwatch {
    
public:
    using clock = std::chrono::high_resolution_clock;
    stopwatch();
    double elapsed_ms() const;

private:
    std::chrono::high_resolution_clock::time_point start_point_;
};

} // namespace nibautil