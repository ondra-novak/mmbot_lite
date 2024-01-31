
#include <limits>

namespace mmbot {

constexpr bool isfinite(double x) {
    return x > -std::numeric_limits<double>::infinity() && x < std::numeric_limits<double>::infinity();
}

constexpr bool is_not_large(double x) {
    return x > -1e30 && x < 1e30;
}

template<typename Fn>
constexpr double calculate_delta(Fn &&fn, double x, double step = 1e-4) {
    double xr = x+x*step;
    double y = fn(x);
    double yr = fn(xr);
    if (!is_not_large(yr)) {
        xr = x - x*step;
        yr = fn(xr);
    }
    return (yr - y)/(xr - x);
}




template<typename Fn, unsigned int steps = 10>
constexpr std::pair<double,double> find_definition_edge(Fn &&fn, double from, double to) {
    for (unsigned int i = 0; i < steps; i++) {
        double mid = (from + to)*0.5;
        double v = fn(mid);
        if (is_not_large(v)) {
            from = mid;
        } else {
            to = mid;
        }
    }
    double mid2 = (from + to)*0.5;
    double v2 = fn(mid2);
    if (!is_not_large(v2)) {
        return {from, std::abs((from-to)*0.5)/from};
    } else {
        return {mid2, std::abs((from-to)*0.5)/mid2};
    }
}



template <typename Fn, int iterations = 1000>
constexpr double newtonMethod_positive(Fn &&fn, double guess) {
    double x = guess;

    double err = 2e-4;
    double prev_x = x;
    for (int i = 0; i < iterations; ++i) {
        double f_x = fn(x);
        if (!is_not_large(f_x)) {
            auto r = find_definition_edge(fn, prev_x, x);
            x = r.first;
            f_x = fn(x);
            err = r.second;
        }
        prev_x = x;
        double df_x = calculate_delta(fn, x, std::min(err/2, 1e-4));
        double new_x = x - f_x / df_x;
        if (new_x <= 0 ) new_x = x/2;     //prevent going to negative
        err = std::abs(new_x - x)/(new_x + x);
        if (err < 1e-7) return new_x;
        x = new_x;
    }
    return x;
}




template<int dir, typename Fn,int iterations = 30>
constexpr double findRoot(Fn &&fn, double start) {
    bool found;
    static_assert(dir <0 || dir > 0);
    auto transf = [](auto x){
        if constexpr(dir > 0) {
            return 1.0/x;
        } else {
            return x;
        }
    };
    double from = transf(start);
    double to = 0;
    double rf = fn(transf(from));
    for (unsigned int i = 0; i < iterations; ++i) {
        double mid = (from + to)*0.5;
        double v = fn(transf(mid));
        if (v * rf < 0) {
            found = true;
            to = mid;
        } else {
            from = mid;
        }
    }
    if (!found) return std::numeric_limits<double>::signaling_NaN();
    return transf((from+to)*0.5);
}






}


