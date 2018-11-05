#pragma once
#include <boost/assert.hpp>
#include <random>
#include <vector>

namespace nibashared {

// TODO figure out how to let js have the same random result by passing seed
// returning double might not be a good idea, so lets return integers..
class rng_server {
public:
    rng_server() : mt_(rd_()) {}
    int operator()(int lower, int upper) {
        std::uniform_int_distribution<> dis(lower, upper);
        auto result = dis(mt_);
        generated.push_back(result);
        return result;
    }

    std::vector<int> generated;

private:
    std::random_device rd_;
    std::mt19937 mt_;
};

//
class rng_client {
public:
    rng_client(std::vector<int> &&precomputed) : precomputed_(precomputed){};
    int operator()(int lower, int upper) {
        (void)lower;
        (void)upper;
        BOOST_ASSERT(internal_idx_ < precomputed_.size());
        return precomputed_[internal_idx_++];
    }

private:
    std::size_t internal_idx_ = 0;
    std::vector<int> precomputed_;
};

} // namespace nibashared
