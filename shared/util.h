#pragma once

#include <algorithm>
#include <chrono>

namespace nibautil {

template<typename Iterable, typename F>
F for_each(Iterable &iterable, F f) {
    return std::for_each(std::begin(iterable), std::end(iterable), f);
}

// code copied straight from LLVM STLExtras.h
template <typename R, typename UnaryPredicate>
auto find_if(R &&Range, UnaryPredicate P) -> decltype(std::begin(Range)) {
  return std::find_if(std::begin(Range), std::end(Range), P);
}

// idiom to delete from vector
template<typename Vector, typename RemoveCondF>
void vector_remove(Vector& v, RemoveCondF f) {
    v.erase(std::remove_if(v.begin(), v.end(), f), v.end());
}

class stopwatch {

public:
    using clock = std::chrono::high_resolution_clock;
    stopwatch();
    double elapsed_ms() const;

private:
    std::chrono::high_resolution_clock::time_point start_point_;
};

} // namespace nibautil
