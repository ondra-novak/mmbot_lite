#include "StrategyDCAM.h"
#include "numerics.h"
#include "market.h"

#include <cmath>
namespace mmbot {

//smallest value - default count of steps of find_root is 32, so smallest unit is aprx 1e-10
constexpr double epsilon = 1.0/std::exp(32);

StrategyDCAM::StrategyDCAM(const Config &cfg, clone_ptr<ISpreadGen> spread)
:_cfg(cfg),_spread(std::move(spread))
{
}

void StrategyDCAM::start(const StrategyMarketState &ms, clone_ptr<ISpreadGen::State> state) {
    _spread_state = std::move(state);
    double k = find_k_from_pos(_cfg.exponent, _cfg.select_range(ms.position), _cfg.initial_budget, ms.last_exec_price, ms.position);
    _st.zero = ms.position == 0;
    _st.val = calc_value(_cfg.initial_budget, _cfg.exponent, k, _cfg.select_range(ms.position), ms.last_exec_price);
    _st.k = k;
    _st.p = ms.last;
}



StrategyDCAM::RuleResult StrategyDCAM::find_k_rule(double pos, double pnl, double open_price, double close_price) {
    double target = calc_target(open_price, close_price);
    int side = pos < -epsilon?-1:pos > epsilon?1:0;
    double k;
    if (side == 0) {
        pnl -= target;
        side = close_price<open_price?1:close_price>open_price?-1:0;
        if (side == 0) return {close_price};
    }
    double z = _cfg.select_range(side);


    if (pnl > 0) {
        if (_st.val+pnl < 0) {
            pnl -= target;
            k = find_k(_cfg.exponent, z, _cfg.initial_budget, close_price, _st.val+pnl);
        } else {
            k = close_price;
        }
        /*
        if (_st.val+pnl < -target) {
            pnl -= target;
            k = find_k(_cfg.exponent, z, _cfg.initial_budget, close_price, _st.val+pnl);
        } else if (_st.val + pnl < 0){
            k = _st.k;
        } else {
            k = close_price;
        }*/
    } else {
        k = find_k(_cfg.exponent, z, _cfg.initial_budget, close_price, _st.val+pnl);
    }
    double val = calc_value(_cfg.initial_budget, _cfg.exponent, k, z, close_price);
    double rpos = calc_position(_cfg.initial_budget, _cfg.exponent, k, z, close_price);
    return {k, val, rpos};
}

StrategyResult StrategyDCAM::step(const StrategyMarketState &state) {
    double min_size = state.minfo->get_min_size(state.last_exec_price);
    StrategyResult cmd = {};

    bool zero = std::abs(state.position) < min_size;
    bool enable_buy = true;
    bool enable_sell = true;


    if (state.execution || state.alert) {
        double prev_pos = std::abs(state.prev_position)<min_size?0.0:state.prev_position;
        _spread->point(_spread_state, state.last_exec_price, true);

        RuleResult rr = find_k_rule(prev_pos, state.pnl, state.prev_exec_price, state.last_exec_price);
        _st.val = rr.val;
        cmd.allocate = _st.val+_cfg.initial_budget;
        cmd.neutral_price = rr.k;
        _st.p = state.last_exec_price;
        _st.k = rr.k;
    } else {
        _spread->point(_spread_state, state.last, false);
        cmd.allocate = _st.val + state.pnl+_cfg.initial_budget;
        cmd.neutral_price = _st.k;
        double lev = state.cost / cmd.allocate;
        if (lev > _cfg.max_leverage) {
            if (state.position<0) {
                enable_sell = false;
                _st.p = std::max(_st.p, state.last);
            }
            else if (state.position>0) {
                enable_buy = false;;
                _st.p = std::min(_st.p, state.last);
            }
        }
    }

    _st.zero =  zero;

    auto spread_state = _spread->get_result(_spread_state, _st.p);
    if (spread_state.buy && enable_buy) {
        cmd.buy.price = std::min(*spread_state.buy, state.ask);
        cmd.buy.size = std::max(calc_order(_st.p, state.position, cmd.buy.price, state.minfo->pct_fee),0.0);
//        if (cmd.buy.size == 0 && !enable_sell) cmd.buy.size = std::max(0.0,-state.position*0.5);
    }

    if (spread_state.sell && enable_sell) {
        cmd.sell.price = std::max(*spread_state.sell, state.bid);
        cmd.sell.size = std::max(calc_order(_st.p,  state.position, cmd.sell.price, state.minfo->pct_fee), 0.0);
//        if (cmd.sell.size == 0 && !enable_buy) cmd.sell.size = std::max(0.0,state.position*0.5);
    }



    return cmd;

}

constexpr double StrategyDCAM::find_k(double w, double z, double p, double price, double val) {
        if (val >= epsilon) return price;
        double gprice = price * z;
        return not_nan(Numerics<>::find_root_pos(price, price-gprice,[&](double k){
            return calc_value(p, w, k, z, price) - val;
        }),price);
}

constexpr double StrategyDCAM::find_k_from_pos(double w, double z, double p, double price, double pos) {
        if (std::abs(pos < epsilon)) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_position(p, w, k, z, price) - pos;
        }),price);
}


int StrategyDCAM::get_side(double pos) {
    return pos < 0?-1:pos > 0?1:0;
}



double StrategyDCAM::calc_order(double price, double cur_pos, double new_price, double feepct) {
    double new_pnl = cur_pos * (new_price - price);
    double side = get_side(price - new_price);
    RuleResult rr = find_k_rule(cur_pos, new_pnl, price, new_price);
    double max_pos = 0.9*_cfg.max_leverage * (_st.val+_cfg.initial_budget)/new_price;
    double pos = rr.pos;
    if (pos * side > cur_pos * side && max_pos < pos * side) {
        pos = max_pos * side;
    }
    double diff = (pos - cur_pos)*side;
    double newfee = diff * new_price * feepct;
    rr = find_k_rule(cur_pos, new_pnl-newfee, price, new_price);
    diff = (pos - cur_pos)*side;
    return diff;

}

double StrategyDCAM::calc_target(double cur_price, double target_price) const {
    return (cur_price < target_price? (1.0 - cur_price/target_price):(1.0 - target_price/cur_price))*_cfg.target;
}

constexpr double StrategyDCAM::calc_position(double p, double w, double k, double z, double x) {
    return (p*w*std::sinh(w * (1 - x/k))) /
       /*---------------------------------------*/
           (k * (std::cosh(w * (1 - z))- 1));
}

constexpr double StrategyDCAM::calc_value(double p, double w, double k, double z, double x) {
    return p * (1 - std::cosh(w * (1 - x/k))) /
       /*---------------------------------------*/
                (std::cosh(w*(1 - z)) - 1);
}

}
