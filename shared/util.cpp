#include "util.h"

namespace nibautil {

stopwatch::stopwatch() : start_point_(clock::now()) {}

double stopwatch::elapsed_ms() const {
    auto now = clock::now();
    std::chrono::duration<double, std::milli> fp_ms = now - start_point_;
    return fp_ms.count();
}

void stopwatch::reset() {
    start_point_ = clock::now();
}

} // namespace nibautil