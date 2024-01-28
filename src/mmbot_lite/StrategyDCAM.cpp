#include "StrategyDCAM.h"
#include "numerics.h"
#include "market.h"

#include <cmath>
namespace mmbot {

//smallest value - default count of steps of find_root is 32, so smallest unit is aprx 1e-10
constexpr double epsilon = 1.0/std::exp(32);

template<typename BaseFn>
StrategyDCAM<BaseFn> *StrategyDCAM<BaseFn>::clone() const {
    return new StrategyDCAM<BaseFn>(*this);
}

template<typename BaseFn>
StrategyDCAM<BaseFn>::StrategyDCAM(const Config &cfg)
:_cfg(cfg)
{
}



template<typename BaseFn>
void StrategyDCAM<BaseFn>::start(StrategyState &state) {
    _k = find_k_from_pos(_cfg, state.last_exec_price, state.position);
    _val = calc_value(_cfg, _k, state.last_exec_price);
    _p = state.last_exec_price;
    _pos = state.position;
}


template<typename BaseFn>
StrategyDCAM<BaseFn>::RuleResult StrategyDCAM<BaseFn>::find_k_rule(double new_price, bool alert) const {
    double aprx_pnl = _pos * (new_price - _p);
    double new_val = _val + aprx_pnl;
    double new_k = _k;
    double yield = calc_value(_cfg, _p,new_price);
    if ((_p - _k) * (new_price - _k) < 0) {
            new_k = new_price;
    } else {
        if (aprx_pnl < 0 || alert) {
            new_k = find_k(_cfg, new_price, new_val, _pos);
        } else if (new_val < 0 || !_pos) {
                double y = _pos?_cfg.yield_mult:_cfg.initial_yield_mult;
                double extra_val = yield * y;
                new_val += extra_val;
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

template<typename BaseFn>
void StrategyDCAM<BaseFn>::event(StrategyState &state) {
    bool max_lev = false;
    if (state.execution) {
        RuleResult r = find_k_rule(state.last_exec_price);
        double new_price = find_price_from_pos(_cfg, r.k, state.position);
        r = find_k_rule(state.last_exec_price);
        _val = r.val;
        _k = r.k;
        _p = new_price;
        _pos = state.position;
    } else if (state.alert) {
        _k = find_k(_cfg,state.last_exec_price, _val, _pos);
        _p = state.last_exec_price;

    } else {
/*        double pnl = (state.current_price - state.last_exec_price) * _pos;
        double apos = std::abs(_pos);
        double loss = _val +  pnl;
        double lev = (-loss + apos * _p) / _cfg.initial_budget;
        if (pnl < 0 && lev > _cfg.max_leverage) {
            if (_pos < 0) _p = std::max(_p, state.current_price);
            else _p = std::min(_p, state.current_price);
            _k = find_k(_cfg, _p, _val, _pos);
            state.equilibrium = _p;
            max_lev = true;
        }*/
    }
    if (std::abs(_pos) <  state.min_size) {
        _pos = 0;
    }

    create_orders(state);
    if (max_lev) {
        if (_pos>0) state.buy.reset();
        else state.sell.reset();
    }
}

template<typename BaseFn>
void StrategyDCAM<BaseFn>::store(PersistentStorage &storage) const {
}

template<typename BaseFn>
void StrategyDCAM<BaseFn>::restore(const PersistentStorage &storage) {
}

template<typename BaseFn>
std::optional<double> StrategyDCAM<BaseFn>::calc_order(double price, double side) const {
    RuleResult r = find_k_rule(price);
    double apos = r.pos * side;
    double lev = (apos * price - r.val)/_cfg.initial_budget;
    if (lev > _cfg.max_leverage) {
        return {};
    }
    double diff = apos - _pos * side;
    return diff;

}

template<typename BaseFn>
void StrategyDCAM<BaseFn>::create_orders(StrategyState &st) {

    if (st.buy.has_value()) {
        auto sz = calc_order(st.buy->price,1);
        if (!sz.has_value()) {
            st.buy->size = 0;
        } else if (*sz < st.min_size) {
            if (_pos) {
                st.buy.reset();
            } else {
                st.buy->size = st.min_size;
            }
        }
        else {
            st.buy->size = *sz;
        }
    }
    if (st.sell.has_value()) {
        auto sz = calc_order(st.sell->price,-1);
        if (!sz.has_value()) {
            st.sell->size = 0;
        } else if (*sz < st.min_size) {
            if (_pos) {
                st.sell.reset();
            } else {
                st.sell->size = st.min_size;
            }
        } else {
            st.sell->size = *sz;
        }
    }

    st.allocation = _cfg.initial_budget + _val + _pos*(st.current_price - _p);
    st.equilibrium = _p;
    st.neutral_price = _k;

}




static double calc_factor(double p,double w, double c) {
    return p/(std::cosh(w*(1.0 - c))-1.0);
}

double FunctionSinH::fnx(double p, double w, double k, double c, double x) {
    return (w/k) * std::sinh(w * (1 - x/k)) * p * c/(w*w);
}

double FunctionSinH::integral_fnx(double p, double w, double k, double c, double x) {
    return  (1 - std::cosh(w*(1 - x/k))) * p * c/(w*w);
}

template<typename BaseFn>
double StrategyDCAM<BaseFn>::find_k(double w, double c, double p, double price, double val, double pos) const {
        if (val >= epsilon) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return _baseFn.integral_fnx(p, w, k, c, price) - val;
        }),price);
}

double FunctionSinH::invert_fnx(double p, double w, double k, double c, double x) {
    return k - (k * asinh((k * x)/(p * c/w )))/w;
}

template<typename BaseFn>
double StrategyDCAM<BaseFn>::find_k_from_pos(double w, double c, double p, double price, double pos) const {
        if (std::abs(pos) < epsilon) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return _baseFn.integral_fnx(p, w, k, c, price) - pos;
        }),price);
}
template class StrategyDCAM<FunctionSinH>;

template class StrategyDCAM<FunctionPowerN>;

double FunctionPowerN::fnx(double p, double w, double k, double c,double x) {
    double xk = x/k;
    return (w*p*c)/(2*k*w*w)*(std::pow(xk,-w)-std::pow(xk,w));
}

double FunctionPowerN::integral_fnx(double p, double w, double k, double c,double x) {
    double xk = x/k;
    return -(c*p*(-2*k*w+(1+w)*x*std::pow(xk,-w)+(w-1)*x*std::pow(xk,w))/(2*k*(w-1)*w*(w+1)));

}

double FunctionPowerN::invert_fnx(double p, double w, double k,double c, double x) {
    return k*std::pow((std::sqrt(c*c*p*p+k*k*x*x*w*w) -  k*x*w)/(c*p),1.0/w);
}

}
