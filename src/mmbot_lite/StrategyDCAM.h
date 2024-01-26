#pragma once

#include "strategy.h"
#include "spread.h"

#include "acb.h"
namespace mmbot {

class StrategyDCAM: public IStrategy {
public:
    struct Config {
        double power; //w
        double multiplier; //c
        double initial_budget; //p
        double initial_spread_yield_f;
        double spread_yield_f;
        double max_spread_yield_f;
        double shift;
        double max_leverage; //currently not used
    };

    StrategyDCAM(const Config &cfg);
    virtual void start(StrategyState &state) override ;
    virtual void event(StrategyState &state) override ;
    virtual void store(PersistentStorage &storage) const override ;
    virtual void restore(const PersistentStorage &storage) override ;
    virtual StrategyDCAM *clone() const override ;

protected:

    Config _cfg;
    bool _zero = true;
    double _val = 0;     //current value/loss
    double _k = 0;       //previous k value;
    double _p = 0;       //reference price
    double _pos = 0;

//    double get_mc() const;
    static int get_side(double pos);

    static double calc_position(double p, double w, double k, double c, double x);
    static double calc_value(double p, double w, double k, double c, double x);
    static double find_price_from_pos(double p, double w, double k, double c, double x);

    static double find_k(double w, double c, double p, double price, double val, double pos);
    static double find_k_from_pos(double w, double c, double p, double price, double pos);

    static double calc_position(const Config &cfg, double k, double x) {
        return calc_position(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }
    static double calc_value(const Config &cfg, double k, double x) {
        return calc_value(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }
    static double find_price_from_pos(const Config &cfg, double k, double x) {
        return find_price_from_pos(cfg.initial_budget, cfg.power, k, cfg.multiplier, x);
    }

    static double find_k(const Config &cfg, double price, double val, double pos) {
        return find_k(cfg.power, cfg.multiplier, cfg.initial_budget, price, val, pos);
    }
    static double find_k_from_pos(const Config &cfg, double price, double pos) {
        return find_k_from_pos(cfg.power, cfg.multiplier, cfg.initial_budget, price, pos);
    }


    struct RuleResult {
        double k = 0.0;
        double val = 0.0;
        double pos = 0.0;
    };

    RuleResult  find_k_rule(double new_price, bool alert = false) const;

    void create_orders(StrategyState &st) const;

    double calc_order(double price) const;


};


}
