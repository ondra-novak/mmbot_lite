#pragma once

#include <random>


namespace mmbot {

using RandomID = std::uint64_t;




inline RandomID generate_random_id() {
    static thread_local std::default_random_engine rnd(std::random_device().operator ()());
    std::uniform_int_distribution<int> dist(0,65536);
    RandomID out = 0;
    for (unsigned int i = 0; i < 4; ++i) {
        out = (out << 16) | dist(rnd);
    }
    return out;
}

template<typename Iter>
constexpr inline Iter randomIdToText(RandomID id, Iter iter) {
    if (id == 0) {
        *iter='0';
        ++iter;
    } else {
        while (id) {
            auto p = id % 36;
            id = id / 36;
            if (p < 10) *iter=p+'0';
            else *iter=p+'A'-10;
            ++iter;
        }
    }
    return iter;
}


template<typename Iter>
constexpr inline RandomID textToRandomID(Iter at, Iter end) {
    RandomID sum = 0;
    RandomID mult = 1;
    while (at != end) {
        char c = *at;
        int v;
        if (c >= '0' && c <='9') {
            v = static_cast<int>(c-'0');
        } else if (c >= 'A' && c <= 'Z') {
            v = static_cast<int>(c-'A')+10;
        } else if (c >= 'a' && c <= 'z') {
            v = static_cast<int>(c-'a')+10;
        }
        ++at;
        sum += v * mult;
        mult *= 36;
    }
    return sum;

}







}
