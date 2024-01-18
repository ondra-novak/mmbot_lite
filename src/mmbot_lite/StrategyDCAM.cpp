#include "StrategyDCAM.h"
#include "numerics.h"

#include <cmath>
namespace mmbot {

StrategyDCAM::StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread)
:_cfg(cfg),_spread(std::move(spread))
{
}

StrategyMarketCommand StrategyDCAM::step(const StrategyMarketState &state) {
    if (state.execution || state.alert) {
        _spread->point(_spread_state, state.last_exec_price, true);

        double k;
        double r= _cfg.select_range(state.position);
        double refpos = 0;
        bool zero_pos = std::abs(state.position) < state.min_size;

        if (_st.zero) {

            if (zero_pos) {
                _st.val = 0;
                k = state.position;
            } else {
                _st.zero = false;
                k = find_k(_cfg.exponent, r, _cfg.initial_budget,
                           state.last_exec_price, -_cfg.target+state.pnl,
                           get_side(state.position) );
                _st.val = calc_budget(state.last_exec_price, k, _cfg.exponent,
                           calc_mc(_cfg.initial_budget,k,_cfg.exponent, r));
            }
        } else  {
            _st.val += state.pnl;
            k = find_k(_cfg.exponent, r, _cfg.initial_budget,
                       state.last_exec_price, _st.val, get_side(state.position) );

        }
        refpos = calc_position(state.last_exec_price, k, _cfg.exponent, calc_mc(_cfg.initial_budget, k, _cfg.exponent, r));
        _st.zero = zero_pos;
    } else {
        _spread->point(_spread_state, state.last, false);
    }


    StrategyMarketCommand cmd;

    auto spread_state = _spread->get_result(_spread_state, state.last_exec_price);
    if (spread_state.buy) {
        cmd.buy.price = std::min(*spread_state.buy, state.ask);
        cmd.buy.size = std::max(calc_order(state.last_exec_price, state.position, cmd.buy.price, 1.0),0.0);
    } else {
        cmd.buy = {};
    }

    if (spread_state.sell) {
        cmd.sell.price = std::max(*spread_state.sell, state.bid);
        cmd.sell.size = std::max(calc_order(state.last_exec_price,  state.position, cmd.sell.price, -1.0), 0.0);
    }

    cmd.allocate = _cfg.initial_budget + _st.val + state.pnl;

    return cmd;

}

void StrategyDCAM::start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state) {
    _spread_state = std::move(state);
    double k = find_k_from_pos(_cfg.exponent, _cfg.select_range(ms.position), _cfg.initial_budget, ms.last_exec_price, ms.position);
    _st.zero = ms.position == 0;
    _st.val = calc_budget(ms.last_exec_price, k, _cfg.initial_budget, calc_mc(_cfg.initial_budget, k, _cfg.exponent, _cfg.select_range(ms.position)));
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


double StrategyDCAM::find_k(double w, double z, double p, double price, double loss, int side) {
    if (side == 0 || loss >= 0) return price;
    double gprice = price * z;
    return not_nan(Numerics<>::find_root_pos(price, price-gprice,[&](double x){
        return calc_search_k_fn(price, x, w, z, p) - loss;
    }),price);

}

double StrategyDCAM::calc_price_from_pos(double k, double w,  double z, double p, double pos) {
    double mc = calc_mc(p,k,w,z);
    return not_nan(Numerics<>::find_root_pos(k,-pos,[&](double x){
        return calc_position(x, k, w, mc) - pos;
    }), k);

}

int StrategyDCAM::get_side(double pos) {
    return pos < 0?-1:pos > 0?1:0;
}

double StrategyDCAM::find_k_from_pos(double p, double w, double z, double price, double pos) {
    return not_nan(Numerics<>::find_root_pos(price, pos, [&](double x){
        return calc_position_fn_k(price, x, w, z, p) - pos;
    }), price);


}

double StrategyDCAM::calc_position_fn_k(double v, double x, double w, double z, double p) {
    double vx = v/x;
    return (-(p * (std::pow(vx,-w) - std::pow(vx,w)))/((-1/(1 - w) + 1/(1 + w) + std::pow(z,(1 - w))/(1 - w) - std::pow(z,(1 + w))/(1 + w)) * x));
}

double StrategyDCAM::calc_order(double price, double cur_pos, double new_price, double side) {
    double k;
    double r;
    double new_pnl = cur_pos * (new_price - price);
    if (_st.zero) {
        r = _cfg.select_range(side);
        k = find_k(_cfg.exponent, r, _cfg.initial_budget, new_price, -_cfg.target, get_side(side) );
    } else  {
        r = _cfg.select_range(cur_pos);
        k = find_k(_cfg.exponent,r, _cfg.initial_budget, new_price, _st.val+new_pnl, get_side(cur_pos) );
    }

    double new_pos = calc_position(new_price, k, _cfg.exponent, calc_mc(_cfg.initial_budget, k, _cfg.exponent, r));
    if (!_st.zero && cur_pos * new_pos < 0) {
        new_pos = 0;
    }
    return (new_pos - cur_pos)*side;

}

}
