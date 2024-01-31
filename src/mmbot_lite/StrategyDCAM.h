#pragma once

#include "strategy.h"
#include "spread.h"

namespace mmbot {

template<typename BaseFn>
class StrategyDCAM: public IStrategy {
public:
    struct Config {
        double power; //w
        double multiplier; //c
        double initial_budget; //p
        double initial_yield_mult;
        double yield_mult;
        double max_leverage; //currently not used
    };

    StrategyDCAM(const Config &cfg);
    virtual void start(StrategyState &state) override ;
    virtual void event(StrategyState &state) override ;
    virtual void store(PersistentStorage &storage) const override ;
    virtual bool restore(const PersistentStorage &storage) override ;
    virtual StrategyDCAM *clone() const override ;

protected:

    Config _cfg;
    mutable BaseFn _baseFn;
    double _val = 0;     //current value/loss
    double _k = 0;       //previous k value;
    double _p = 0;       //reference price
    double _pos = 0;

    enum class Fld {
        val, k, p, pos,

    _count};


//    double get_mc() const;
    static int get_side(double pos);

    double find_k(double w, double c, double p, double price, double val, double pos) const;
    double find_k_from_pos(double w, double c, double p, double price, double pos) const;

    double calc_position(const Config &cfg, double k, double x) const {
        return _baseFn.fnx(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }
    double calc_value(const Config &cfg, double k, double x) const {
        return _baseFn.integral_fnx(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }
    double find_price_from_pos(const Config &cfg, double k, double x) const {
        return _baseFn.invert_fnx(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }

    double find_k(const Config &cfg, double price, double val, double pos) const{
        return find_k(cfg.power, cfg.multiplier, cfg.initial_budget, price, val, pos);
    }
    double find_k_from_pos(const Config &cfg, double price, double pos) const{
        return find_k_from_pos(cfg.power, cfg.multiplier, cfg.initial_budget, price, pos);
    }


    struct RuleResult {
        double k = 0.0;
        double val = 0.0;
        double pos = 0.0;
    };

    RuleResult  find_k_rule(double new_price, bool alert = false) const;

    void create_orders(StrategyState &st) ;

    std::optional<double> calc_order(double price, double side) const;
};

struct FunctionSinH {
    static double fnx(double p, double w, double k, double c, double x);
    static double integral_fnx(double p, double w, double k, double c, double x);
    static double invert_fnx(double p, double w, double k, double c, double x);
};
struct FunctionPowerN {
    static double fnx(double p, double w, double k, double c, double x);
    static double integral_fnx(double p, double w, double k, double c, double x);
    static double invert_fnx(double p, double w, double k, double c, double x);
};

using StrategyDCAM_SinH = StrategyDCAM<FunctionSinH>;
using StrategyDCAM_PowerN = StrategyDCAM<FunctionPowerN>;


}
