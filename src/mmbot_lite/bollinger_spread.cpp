#include "bollinger_spread.h"

namespace mmbot {

BBSpread::BBSpread(Config cfg):BBSpread(init(cfg)) {
}

BBSpread BBSpread::init(const Config &cfg) {
    std::vector<double> c;
    if (cfg.levels.empty()) {
        c.push_back(1.0);
        c.push_back(-1.0);
    } else {
        for (double d : cfg.levels) {
            if (d > 0) {
                c.push_back(d);
                c.push_back(-d);
            }
        }
    }

    if (cfg.zero_level) c.push_back(0.0);
    std::sort(c.begin(), c.end());
    return BBSpread(std::move(c), EMStDev(cfg.mean_points, cfg.stdev_points), std::max(cfg.mean_points, cfg.stdev_points));

}

void BBSpread::point(double y) {
    _stdev += y;
    auto nxb = next_buy(_buy_curve);
    if (_stdev(*nxb) < y) _buy_curve = nxb;

    auto nxs = next_sell(_sell_curve);
    if (_stdev(*nxs) > y) _sell_curve = nxs;

}

ISpread::Result BBSpread::get_result(double equilibrium) const {
    Result r;
    {
        auto iter = _buy_curve;
        while (!below(iter)) {
            auto b = _stdev(*iter);
            if (b < equilibrium) {
                r.buy = b;
                break;
            }
            --iter;
        }
    }

    {
        auto iter = _sell_curve;
        while (!above(iter)) {
            auto s = _stdev(*iter);
            if (s > equilibrium) {
                r.sell = s;
                break;
            }
            ++iter;
        }
    }
    return r;
}


void BBSpread::execution(double y) {
    auto near = _disabled_curve;
    double best = std::numeric_limits<double>::max();
    for (const auto &c: _curves) {
        if (&c == _disabled_curve) continue;
        double p = _stdev(c);
        double dist = std::abs(p - y);
        if (dist < best) {
            best = dist;
            near = &c;
        }
    }

    _disabled_curve = &(*near);
    _buy_curve = &(*near)-1;
    _sell_curve = &(*near)+1;
}

unsigned int BBSpread::get_min_point_count() const {
    return _min_period;
}

void BBSpread::start(double y) {
    _stdev.set_initial(y, 0.01*y);
    auto init = std::lower_bound(_curves.begin(), _curves.end(), 0.0);
    if (init == _curves.end()) {
        _disabled_curve = &_curves.back();
    } else {
        _disabled_curve = &(*init);
    }
    _buy_curve = &_curves.front();
    _sell_curve = &_curves.back();
}




}
