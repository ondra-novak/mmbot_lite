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
}


StrategyDCAM::RuleResult StrategyDCAM::find_k_rule(double new_price, bool alert) const {
    double aprx_pnl = _pos * (new_price - _p);
    double new_val = _val + aprx_pnl;
    double new_k = _k;
    double spread = std::abs(1.0-new_price/_p);
    double shift_v = _cfg.initial_budget * spread * _cfg.shift;
    if ((_p - _k) * (new_price - _k) < 0) {
            new_k = new_price;
    } else {
        if (aprx_pnl < 0 || alert) {
            new_val += shift_v;
            new_k = find_k(_cfg, new_price, new_val, _pos);
        } else if (new_val < 0 || !_pos) {
                double y = _pos?_cfg.spread_yield_f:_cfg.initial_spread_yield_f;
                double extra_val = std::min( _cfg.max_spread_yield_f,y *  spread) * _cfg.initial_budget;
                new_val -= extra_val;
                new_k = find_k(_cfg, new_price, new_val, _pos?_pos:(_p - new_price));
        }
/*        if ((new_k - _k) * (new_price - _k)< 0) {
            new_k = _k;
        }*/
    }
    return {
        new_k,
        calc_value(_cfg, new_k, new_price),
        calc_position(_cfg,new_k, new_price)
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
        _pos = state.position;
    } else if (state.alert) {
        RuleResult r = find_k_rule(state.last_exec_price, true);
        _val = r.val;
        _k = r.k;
        _p = state.last_exec_change;
        _pos = r.pos;
    }
    if (std::abs(_pos) <  state.min_size) {
        _pos = 0;
    }

    create_orders(state);
}

void StrategyDCAM::store(PersistentStorage &storage) const {
}

void StrategyDCAM::restore(const PersistentStorage &storage) {
}

double StrategyDCAM::calc_order(double price) const {
    RuleResult r = find_k_rule(price);
    return r.pos - _pos;
}

void StrategyDCAM::create_orders(StrategyState &st) const {

    if (st.buy.has_value()) {
        double sz = calc_order(st.buy->price);
        if (sz < st.min_size) {
            if (_pos) {
                st.buy.reset();
            } else {
                st.buy->size = st.min_size;
            }
        }
        else st.buy->size = sz;
    }
    if (st.sell.has_value()) {
        double sz = -calc_order(st.sell->price);
        if (sz < st.min_size) {
            if (_pos) {
                st.sell.reset();
            } else {
                st.sell->size = st.min_size;
            }

        }
        else st.sell->size = sz;
    }

    st.allocation = _cfg.initial_budget + _val + _pos*(st.current_price - _p);
    st.equilibrium = _p;
    st.neutral_price = _k;

}




static double calc_factor(double p,double w, double c) {
    return p/(std::cosh(w*(1.0 - c))-1.0);
}

double StrategyDCAM::calc_position(double p, double w, double k, double c, double x) {
    return (w/k) * std::sinh(w * (1 - x/k)) * calc_factor(p, w, c);
}

double StrategyDCAM::calc_value(double p, double w, double k, double c, double x) {
    return  (1 - std::cosh(w*(1 - x/k))) * calc_factor(p, w, c);
}

double StrategyDCAM::find_k(double w, double c, double p, double price, double val, double pos) {
        if (val >= epsilon) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_value(p, w, k, c, price) - val;
        }),price);
}

double StrategyDCAM::find_price_from_pos(double p, double w, double k, double c, double x) {
    return (k * (w - std::asinh((-x * k + x * k * cosh((1 - c) * w))/(p * w))))/w;
}

double StrategyDCAM::find_k_from_pos(double w, double c, double p, double price, double pos) {
        if (std::abs(pos < epsilon)) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_position(p, w, k, c, price) - pos;
        }),price);
}

}
