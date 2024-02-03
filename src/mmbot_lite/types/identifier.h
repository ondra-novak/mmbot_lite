#pragma once

#include <string>
#include <string_view>
#include <type_traits>

namespace mmbot {


class Identifier {
public:

    constexpr Identifier() = default;
    constexpr Identifier(std::uint64_t val):val(val) {}
    constexpr Identifier(std::string_view name):val(0) {
        for (auto c: name) {
            int v;
            if (c >= '0' && c <= '9') {
                v = c - '0';
            } else if (c >= 'A' && c <= 'Z') {
                v = c - 'A' + 10;
            } else if (c >= 'a' && c <= 'z') {
                v = c - 'a' + 10;
            } else if (c == '-') {
                v = 36;
            } else {
                v = 37;
            }
            if (std::is_constant_evaluated() && val > 485440633518672410ULL) {
                throw "Overflow!";
            }
            val = (val * 38) + v;
        }
    }

    constexpr int operator <=> (const Identifier &ident) const {
        return val < ident.val?-1:val > ident.val?1:0;
    }


    template<typename Iter>
    constexpr Iter to_string(Iter out) const {
        char buff[20];
        auto c = std::end(buff);
        auto v = val;
        while (v) {
            --c;
            auto p = v % 38;
            v = v / 38;
            if (p < 10) *c = p+ '0';
            else if (p < 36) *c = p + 'a' - 10;
            else if (p == 36) *c = '-';
            else *c = '_';
        }
        return std::copy(c, std::end(buff), out);
    }

    std::string to_string() const {
        std::string out;
        to_string(std::back_inserter(out));
        return out;
    }

    std::uint64_t to_number() const {
        return val;
    }

protected:
    std::uint64_t val;
};

}
