#include "StrategyDCAM.h"
#include "newton.h"

#include <cmath>
namespace mmbot {

StrategyDCAM::StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread)
:_cfg(cfg),_spread(std::move(spread))
{
}

StrategyMarketCommand StrategyDCAM::step(const StrategyMarketState &state) {
    if (state.execution) {
        _spread->point(_spread_state, state.last_exec_price, true);

        double k;
        if (_st.zero) {
            _st.zero = state.last_exec_price == 0;
            k = find_k(_cfg.exponent, _cfg.range, _cfg.initial_budget, state.last_exec_price, _cfg.target, get_side(state.position) );
            _st.val = calc_budget(state.last_exec_price, k, _cfg.exponent, calc_mc(_cfg.initial_budget,k,_cfg.exponent, _cfg.range));
        } else  {
            _st.val += state.pnl;
            k = find_k(_cfg.exponent, _cfg.range, _cfg.initial_budget, state.last_exec_price, _st.val, get_side(state.position) );
            _st.zero = std::abs(state.position) < state.min_size;
        }

    } else {
        _spread->point(_spread_state, state.last, true);
    }


    StrategyMarketCommand cmd;

    auto spread_state = _spread->get_result(_spread_state, state.last_exec_price);
    if (spread_state.buy) {
        cmd.buy.price = std::min(*spread_state.buy, state.ask);
        cmd.buy.size = std::max(calc_order(cmd.buy.price, state.position),0.0);
    } else {
        cmd.buy = {};
    }

    if (spread_state.sell) {
        cmd.sell.price = std::max(*spread_state.sell, state.bid);
        cmd.sell.size = std::max(calc_order(cmd.sell.price, state.position), 0.0);
    }
    return cmd;

}

void StrategyDCAM::start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state) {
    _spread_state = std::move(state);
    double k = find_k_from_pos(_cfg.exponent, _cfg.range, _cfg.initial_budget, ms.last_exec_price, ms.position);
    _st.zero = ms.position == 0;
    _st.val = calc_budget(ms.last_exec_price, k, _cfg.initial_budget, calc_mc(_cfg.initial_budget, k, _cfg.exponent, _cfg.range));
}

double StrategyDCAM::calc_mc(double p, double k, double w, double z) {
    return -p
            / (-1.0/(1.0-w)+1.0/(1.0+w)+std::pow(z,1.0-w)/(1.0-w)-std::pow(z,1.0+w)/(1.0+w))
            / k;
}

double StrategyDCAM::calc_position(double x, double k, double w, double mc) {
    double xk = x/k;
    return mc * (std::pow(xk, -w) - std::pow(xk, w));
}

double StrategyDCAM::calc_budget(double x, double k, double w, double mc) {
    double xk = x/k;
    return k*mc*(1.0/(1.0+w)-1.0/(1.0-w) + std::pow(xk,1.0-w)/(1.0 - w) - std::pow(xk,1+w)/(1.0+w));
}

double StrategyDCAM::calc_search_k_fn(double v, double x, double w, double z, double p) {
    double vx = v/x;
    return (-(p * (-1/(1 - w) + 1/(1 + w) + std::pow(vx,(1 - w))/(1 - w) - std::pow(vx,(1 + w))/(1 + w)))/(-1/(1 - w) + 1/(1 + w) + std::pow(z,(1 - w))/(1 - w) - std::pow(z,(1 + w))/(1 + w)));
}

double StrategyDCAM::calc_search_k_dfn(double v, double x, double w, double z, double p) {
    double vx = v/x;
    return (p * v * (w - 1) * (1 + w) * std::pow(z,w) * (std::pow(vx,2* w) - 1) * std::pow(vx,-w)/(x*x* ((w - 1) * std::pow(z,(2* w + 1)) - 2 * w * std::pow(z,w) + (1 + w) * z)));
}

double StrategyDCAM::calc_position_dfn(double x, double k, double w, double mc) {
    double xk = x/k;
    return -(mc * w * std::pow(xk,-w) * (1 + std::pow(xk,2 * w)))/x;
}

double StrategyDCAM::find_k(double w, double z, double p, double price, double loss, int side) {
    if (side == 0 || loss >= 0) return price;
    double gprice = side > 0?price * z:price/z;
    return newtonMethod_positive([&](double x){
        return calc_search_k_fn(price, x, w, z, p) - loss;
    }, [&](double x){
        return calc_search_k_dfn(price, x, w, z, p);
    }, gprice);

}

double StrategyDCAM::calc_price_from_pos(double k, double w,  double z, double p, double pos) {
    double mc = calc_mc(p,k,w,z);
    double price_min = k/z;
    double price_max = k*z;
    double zpos_min = calc_position((price_max+k)/2.0, k, w, mc);
    double zpos_max = calc_position((price_min+k)/2.0, k, w, mc);
    double guess = pos > zpos_max?price_min:pos<zpos_min?price_max:k;
    return newtonMethod_positive([&](double x){
        return calc_position(x, k, w, mc) - pos;
    }, [&](double x){
        return calc_position_dfn(x,k,w,mc);
    }, guess);

}

int StrategyDCAM::get_side(double pos) {
    return pos < 0?-1:pos > 0?1:0;
}

double StrategyDCAM::find_k_from_pos(double p, double w, double z, double price, double pos) {
    double price_min = price/z;
    double price_max = price*z;
    double zmin = calc_position_fn_k((price_max+price)/2.0, price, w, z, p);
    double zmax = calc_position_fn_k((price_min+price)/2.0, price, w, z, p);
    double guess = pos > zmax?price_max:pos<zmin?price_min:price;
    return newtonMethod_positive([&](double x){
        return calc_position_fn_k(price, x, w, z, p) - pos;
    }, [&](double x){
        return calc_position_dfn_k(price,x,w,z,p);
    }, guess);


}

double StrategyDCAM::calc_position_fn_k(double v, double x, double w, double z, double p) {
    double vx = v/x;
    return (-(p * (std::pow(vx,-w) - std::pow(vx,w)))/((-1/(1 - w) + 1/(1 + w) + std::pow(z,(1 - w))/(1 - w) - std::pow(z,(1 + w))/(1 + w)) * x));
}
double StrategyDCAM::calc_position_dfn_k(double v, double x, double w, double z, double p) {
    double vx = v/x;
    return (p * (-1.0 + w) * (1.0 + w)
            * (-1.0 + w + std::pow(vx,2.0*w) + w * std::pow(vx,2 * w)) * std::pow(z,w))
            /(std::pow(vx,w) * x*x * ((1.0 + w) * z - 2 * w * std::pow(z,w) + (-1.0 + w) * std::pow(z,(1.0 + 2.0 * w))));

}

double StrategyDCAM::calc_order(double price, double cur_pos) {
    double k;
    if (_st.zero) {
        k = find_k(_cfg.exponent, _cfg.range, _cfg.initial_budget, price, _cfg.target, get_side(cur_pos) );
    } else  {
        k = find_k(_cfg.exponent, _cfg.range, _cfg.initial_budget, price, _st.val, get_side(cur_pos) );
    }

    double new_pos = calc_position(price, k, _cfg.exponent, calc_mc(_cfg.initial_budget, k, _cfg.exponent, _cfg.range));
    if (!_st.zero && cur_pos * new_pos < 0) {
        new_pos = 0;
    }
    return new_pos - cur_pos;

}

}
