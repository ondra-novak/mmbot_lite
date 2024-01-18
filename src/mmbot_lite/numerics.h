#pragma once

#include <limits>

namespace mmbot {


template<unsigned int iterations = 30>
class Numerics {
public:

    ///find root of an function between two intervals
    /**
     * @param from first interval. Point on this interval must be defined or infinity
     * @param to second interval. Point on this interval don't need to be defined
     * @param fn function
     * @return returns found root or NaN if not found
     */

    template<typename Fn>
    static constexpr double find_root(double from, double to, Fn &&fn) {
        double rv = fn(from);
        //test whether we are close to root on from - in this case, return the from directly
        if (rv < 2*std::numeric_limits<double>::min() && rv > 2-std::numeric_limits<double>::min()) return from;
        bool found = false;
        for (unsigned int i = 0; i < iterations; ++i) {
            double mid = (from + to)*0.5;
            double v = fn(mid);
            if (v * rv < 0) {
                found = true;
                to = mid;
            } else {
                from = mid;
            }
        }
        return found?(from + to)*0.5:std::numeric_limits<double>::signaling_NaN();
    }

    ///find root of an function between specified point and +infinity
    /**
     * tries to find root between given point and infinity on positive side of axis
     *
     * @param from where start to search, must be positive and must be defined or infinity
     * @param fn function to search
     * @return found root or NaN
     */
    template<typename Fn>
    static constexpr double find_root_to_inf(double from, Fn &&fn) {
        auto trnfn = [&](double x) {return fn(1.0/x);};
        double res = find_root(1.0/from, 0.0, trnfn);
        if (res == std::numeric_limits<double>::signaling_NaN()) return res;
        else return 1.0/res;
    }

    ///find root of an function between specified point and zero
    /**
     * tries to find root between given point and zero on positive side of axis
     *
     * @param from where start to search, must be positive and must be defined or infinity
     * @param fn function to search
     * @return found root or NaN
     */
    template<typename Fn>
    static constexpr double find_root_to_zero(double from, Fn &&fn) {
        return find_root(from, 0.0, fn);
    }


    ///find root in given direction
    /**
     * @param from starting point
     * @param dir if direction is positive, it searches for root towards infinite. If dir is negative,
     * it searches for root towards zero. If dir is zero it returns NaN
     * @param fn function
     * @return
     */
    template<typename Fn>
    static constexpr double find_root_pos(double from, double dir, Fn &&fn) {
        if (dir > 0) {
            return find_root_to_inf(from, fn);
        } else if (dir < 0) {
            return find_root_to_zero(from, fn);
        } else return std::numeric_limits<double>::signaling_NaN();
    }
};

static constexpr bool isFinite(double x) {
    return (-std::numeric_limits<double>::infinity() < x && x < std::numeric_limits<double>::infinity());
}

static constexpr double not_nan(double x, double val = 0.0) {
    return isFinite(x)?x:val;
}

}
