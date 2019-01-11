#include <chrono>

namespace nibautil {

class stopwatch {
    
public:
    using clock = std::chrono::high_resolution_clock;
    stopwatch() : start_point_(clock::now()) {}

    double elapsed_ms() const {
        auto now = clock::now();
        std::chrono::duration<double, std::milli> fp_ms = now - start_point_;
        return fp_ms.count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_point_;
};

} // namespace nibautil