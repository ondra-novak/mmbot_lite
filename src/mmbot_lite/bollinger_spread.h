#pragma once
#include "math/emstdev.h"
#include "spread.h"

#include <vector>
#include <fstream>

namespace mmbot {



class BBSpread : public ISpread {
public:

    struct Config {
        unsigned int mean_points;
        unsigned int stdev_points;
        std::vector<double> levels;
        bool zero_level;

        static Config fromHours(double mean_points, double stdev_points, std::vector<double> levels, bool zero_level = true) {
            return {
                static_cast<unsigned int>(mean_points * 60),
                static_cast<unsigned int>(stdev_points * 60),
                std::move(levels),
                zero_level
            };
        }
    };

    BBSpread(Config cfg);
    virtual void point(double y) override;
    virtual Result get_result(double equilibrium) const override;
    virtual void execution(double y) override;
    virtual unsigned int get_min_point_count() const override;
    virtual void start(double y) override;

protected:

    using Iter = const double *;

    std::vector<double> _curves;
    EMStDev _stdev;
    unsigned int _min_period;
    Iter _disabled_curve;
    Iter _sell_curve;
    Iter _buy_curve;

    static BBSpread init(const Config &cfg);

    BBSpread(std::vector<double> c, EMStDev stdev, unsigned int min_period):_curves(c), _stdev(stdev),_min_period(min_period) {
        start(1.0);
    }
    virtual BBSpread* clone() const override;

    Iter next_buy(Iter x) const;
    Iter next_sell(Iter x) const;
    bool below(Iter x) const;
    bool above(Iter x) const;



};


}
