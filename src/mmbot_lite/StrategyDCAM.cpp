#include "StrategyDCAM.h"
#include "numerics.h"
#include "market.h"

#include <cmath>
namespace mmbot {

//smallest value - default count of steps of find_root is 32, so smallest unit is aprx 1e-10
constexpr double epsilon = 1.0/std::exp(32);

StrategyDCAM *StrategyDCAM::clone() const {
    return new StrategyDCAM(*this);
}

StrategyDCAM::StrategyDCAM(const Config &cfg)
:_cfg(cfg)
{
}



void StrategyDCAM::start(StrategyState &state) {
    _k = find_k_from_pos(_cfg, state.last_exec_price, state.position);
    _val = calc_value(_cfg, _k, state.last_exec_price);
    _p = state.last_exec_price;
    _pos = state.position;

    create_orders(state);
}


StrategyDCAM::RuleResult StrategyDCAM::find_k_rule(double new_price, bool alert) const {
    double aprx_pnl = _pos * (new_price - _p);
    double new_val = _val + aprx_pnl;
    double new_k = _k;
    if (aprx_pnl < 0 || alert) {
        new_k = find_k(_cfg, new_price, new_val, _pos);
    } else if (new_val < 0 || !_pos) {
            double spread = std::abs(1.0-new_price/_p);
            if (spread > _cfg.yield_max_spread) spread = _cfg.yield_max_spread;
            double extra_val = calc_value(_cfg, new_price, new_price*(1.0-spread))*_cfg.yield_multipler;
            new_val += extra_val;
            new_k = find_k(_cfg, new_price, new_val, _pos);
    }
    if ((new_k - _k) * (new_price - _k)< 0) new_k = _k;
    return {
        _k,
        calc_value(_cfg, _k, new_price),
        calc_position(_cfg,_k, new_price)
    };
}

void StrategyDCAM::event(StrategyState &state) {
    if (state.execution) {
        RuleResult r = find_k_rule(state.last_exec_price);
        double new_price = find_price_from_pos(_cfg, r.k, state.position);
        r = find_k_rule(state.last_exec_price);
        _val = r.val;
        _k = r.k;
        _p = new_price;
        _pos = r.pos;
    } else if (state.alert) {
        RuleResult r = find_k_rule(state.last_exec_price, true);
        _val = r.val;
        _k = r.k;
        _p = state.last_exec_change;
        _pos = r.pos;
    }

    create_orders(state);
}

double StrategyDCAM::calc_order(double price) const {
    RuleResult r = find_k_rule(price);
    return r.pos - _pos;
}

void StrategyDCAM::create_orders(StrategyState &st) const {

    if (st.buy.has_value()) {
        double sz = calc_order(st.buy->price);
        if (sz < st.min_size) st.buy.reset();
    }
    if (st.sell.has_value()) {
        double sz = -calc_order(st.sell->price);
        if (sz < st.min_size) st.buy.reset();
    }

    st.allocation = _cfg.initial_budget + _val;
    st.equilibrium = _p;

}




constexpr static double calc_factor(double p,double w, double c) {
    return p/std::cosh(w*(1.0 - c))-1.0;
}

constexpr double StrategyDCAM::calc_position(double p, double w, double k, double c, double x) {
    return (w/k) * std::sinh(w * (1 - x/k)) * calc_factor(p, w, c);
}

constexpr double StrategyDCAM::calc_value(double p, double w, double k, double c, double x) {
    return  (1 - std::cosh(w*(1 - x/k))) * calc_factor(p, w, c);
}

constexpr double StrategyDCAM::find_k(double w, double c, double p, double price, double val, double pos) {
        if (val >= epsilon) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_value(p, w, k, c, price) - val;
        }),price);
}

constexpr double StrategyDCAM::find_price_from_pos(double p, double w, double k, double c, double x) {
    return (k * (w - std::asinh((-x * k + x * k * cosh((1 - c) * w))/(p * w))))/w;
}

constexpr double StrategyDCAM::find_k_from_pos(double w, double c, double p, double price, double pos) {
        if (std::abs(pos < epsilon)) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_position(p, w, k, c, price) - pos;
        }),price);
}

}
