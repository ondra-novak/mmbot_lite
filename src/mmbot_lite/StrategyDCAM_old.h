#pragma once

#include "strategy.h"
#include "spread.h"

#include "acb.h"
namespace mmbot {

class StrategyDCAM {
public:


    struct Config {
        double power; //w
        double multiplier; //c
        double initial_budget; //p
        double target;      //percent of budget to achieve from zero position
        double max_leverage;

    };



    StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread);
    StrategyResult step(const StrategyMarketState &state);
    void start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state);

protected:

    Config _cfg;
    bool _zero = true;
    double _val = 0;     //current value/loss
    double _k = 0;       //previous k value;
    double _p = 0;       //reference price
    clone_ptr<ISpreadGen> _spread;
    clone_ptr<ISpreadGen::State> _spread_state;

//    double get_mc() const;
    static int get_side(double pos);

    static constexpr double calc_position(double p, double w, double k, double c, double x);
    static constexpr double calc_value(double p, double w, double k, double c, double x);
    static constexpr double find_price_from_pos(double p, double w, double k, double c, double x);

    static constexpr double find_k(double w, double c, double p, double price, double val, double pos);
    static constexpr double find_k_from_pos(double w, double c, double p, double price, double pos);

    struct RuleResult {
        double k = 0.0;
        double val = 0.0;
        double pos = 0.0;
    };

    RuleResult  find_k_rule(double pos, double pnl, double open_price, double close_price);


    double calc_order(double price, double new_price);

    double calc_target(double cur_price, double target_price) const;
};


}
