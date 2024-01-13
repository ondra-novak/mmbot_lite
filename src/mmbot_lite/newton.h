
#include <limits>

namespace mmbot {


template <typename Fn, typename DerivativeFn, int iterations = 1000>
constexpr double newtonMethod_positive(Fn fn, DerivativeFn dfn, double guess) {
    double x = guess;

    for (int i = 0; i < iterations; ++i) {
        double f_x = fn(x);
        double df_x = dfn(x);
        double new_x = x - f_x / df_x;
        if (new_x <= 0 ) new_x = x/2;     //prevent going to negative
        double err = std::abs(new_x - x)/(new_x + x);
        if (err < 1e-7) return new_x;
        x = new_x;
    }
    return x;
}

}
