#pragma once

#include "spread.h"

namespace mmbot {


struct StrategyMarketState {
    double bid = 0;
    double ask = 0;
    double last = 0;
    double position = 0;
    double last_exec_price = 0;
    double min_size = 0;        ///< minimum order size
    double pnl = 0;             ///<equity change,  fees are substracted
    bool execution = false;
    bool alert = false;
};

struct StrategyOrder {
    static constexpr double no_order = 0;
    static constexpr double alert = -1;

    double price = 0;   //if price == 0, market order
    double size = 0;    //if size == 0, order is disabled (even if price == 0)
};

struct StrategyMarketCommand {
    StrategyOrder buy;
    StrategyOrder sell;
    double allocate;
};

class StrategyDCAM {
public:


    struct Config {
        double exponent; //w
        double range_p;     //max range up (+150% = 1.5)
        double range_m;     //max range down (-80% = 0.8)
        double initial_budget; //p
        double target;      //percent of budget to achieve from zero position


        double select_range(double pos) const {
            if (pos < 0) return 1.0+range_p;
            else return 1.0-range_m;
        }
    };


    struct State {
        bool zero = true;
        double val = 0;     //current value/loss
    };

    StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread);
    StrategyMarketCommand step(const StrategyMarketState &state);
    void start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state);

protected:

    Config _cfg;
    State _st;
    clone_ptr<ISpreadGen> _spread;
    clone_ptr<ISpreadGen::State> _spread_state;

//    double get_mc() const;
    static int get_side(double pos);

    static double calc_mc(double p, double k, double w, double z);
    static double calc_position(double x, double k, double w, double mc);
//    static double calc_position_dfn(double x, double k, double w, double mc);
    static double calc_budget(double x, double k, double w, double mc);
    static double calc_search_k_fn(double v, double x, double w, double z, double p);
//    static double calc_search_k_dfn(double v, double x, double w, double z, double p);
    static double find_k(double w, double z, double p, double price, double loss, int side);
    static double calc_price_from_pos(double k, double w, double z, double p, double pos);
    static double calc_position_fn_k(double v, double x, double w, double z, double p);
///    static double calc_position_dfn_k(double v, double x, double w, double z, double p);
    static double find_k_from_pos(double p, double w, double z, double price, double pos);


    double calc_order(double price, double cur_pos, double new_price, double side);
};


}
