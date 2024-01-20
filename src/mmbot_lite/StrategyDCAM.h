#pragma once

#include "strategy.h"
#include "spread.h"

namespace mmbot {

class StrategyDCAM {
public:


    struct Config {
        double exponent; //w
        double range_p;     //max range up (+150% = 1.5)
        double range_m;     //max range down (-80% = 0.8)
        double initial_budget; //p
        double target;      //percent of budget to achieve from zero position
        double max_leverage;


        double select_range(double pos) const {
            if (pos < 0) return 1.0+range_p;
            else return 1.0-range_m;
        }
    };


    struct State {
        bool zero = true;
        double val = 0;     //current value/loss
        double k = 0;       //previous k value;
        double p = 0;       //reference price
    };

    StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread);
    StrategyResult step(const StrategyMarketState &state);
    void start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state);

protected:

    Config _cfg;
    State _st;
    clone_ptr<ISpreadGen> _spread;
    clone_ptr<ISpreadGen::State> _spread_state;

//    double get_mc() const;
    static int get_side(double pos);

    static constexpr double calc_position(double p, double w, double k, double z, double x);
    static constexpr double calc_value(double p, double w, double k, double z, double x);

    static constexpr double find_k(double w, double z, double p, double price, double val);
    static constexpr double find_k_from_pos(double w, double z, double p, double price, double pos);

    struct RuleResult {
        double k = 0.0;
        double val = 0.0;
        double pos = 0.0;
    };

    RuleResult  find_k_rule(double pos, double pnl, double open_price, double close_price);


    double calc_order(double price, double cur_pos, double new_price, double feepct);

    double calc_target(double cur_price, double target_price) const;
};


}
