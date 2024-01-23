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
    double k = find_k_from_pos(_cfg.power, _cfg.multiplier, _cfg.initial_budget, ms.last_exec_price, ms.position);
    _zero = ms.position == 0;
    _val = calc_value(_cfg.initial_budget, _cfg.power, k, _cfg.multiplier, ms.last_exec_price);
    _k = k;
    _p = ms.last;
}



StrategyDCAM::RuleResult StrategyDCAM::find_k_rule(double pos, double pnl, double open_price, double close_price) {
    double  target = calc_target(open_price, close_price);
    int side = pos < -epsilon?-1:pos > epsilon?1:0;
    double new_val = _val + pnl;
    double k;
    if (side == 0) {
        side = close_price<open_price?1:close_price>open_price?-1:0;
        if (side == 0) return {close_price};
        new_val -= target;
    }



    k = find_k(_cfg.power, _cfg.multiplier, _cfg.initial_budget, close_price, new_val, side);


    double val = calc_value(_cfg.initial_budget, _cfg.power, k, _cfg.multiplier, close_price);
    double rpos = calc_position(_cfg.initial_budget, _cfg.power, k, _cfg.multiplier, close_price);
    return {k, val, rpos};
}

StrategyResult StrategyDCAM::step(const StrategyMarketState &state) {
    double min_size = state.minfo->get_min_size(state.last_exec_price);
    StrategyResult cmd = {};

    bool zero = std::abs(state.position) < min_size;
    double pnl = state.pnl;


    if (state.execution || state.alert) {

            double prev_pos = std::abs(state.prev_position)<min_size?0.0:state.prev_position;
            _spread->point(_spread_state, state.last_exec_price, true);

            RuleResult rr = find_k_rule(prev_pos, pnl, state.prev_exec_price, state.last_exec_price);
            _val = rr.val;
            cmd.allocate = _val+_cfg.initial_budget;
            cmd.neutral_price = rr.k;
            _p = find_price_from_pos(_cfg.initial_budget, _cfg.power, rr.k, _cfg.multiplier, state.position);
            _k = rr.k;
    } else {
        cmd.allocate = _val + pnl+_cfg.initial_budget;
        if (state.cost/cmd.allocate > _cfg.max_leverage) {
            _val = 0;
            _p = state.last;
        }
        cmd.neutral_price = _k;
    }
    _zero = zero;

    _spread->point(_spread_state, state.last, false);

    auto spread_state = _spread->get_result(_spread_state, _p);
    if (spread_state.buy ) {
        cmd.buy.price = std::min(*spread_state.buy, state.ask);
        cmd.buy.size = std::max(calc_order(state.last_exec_price, state.position, cmd.buy.price, state.minfo->pct_fee),0.0);
    }

    if (spread_state.sell) {
        cmd.sell.price = std::max(*spread_state.sell, state.bid);
        cmd.sell.size = std::max(calc_order(state.last_exec_price,  state.position, cmd.sell.price, state.minfo->pct_fee), 0.0);
    }

    return cmd;

}

constexpr double StrategyDCAM::find_k(double w, double c, double p, double price, double val, double pos) {
        if (val >= epsilon) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_value(p, w, k, c, price) - val;
        }),price);
}

constexpr double StrategyDCAM::find_k_from_pos(double w, double c, double p, double price, double pos) {
        if (std::abs(pos < epsilon)) return price;
        return not_nan(Numerics<>::find_root_pos(price, pos,[&](double k){
            return calc_position(p, w, k, c, price) - pos;
        }),price);
}


int StrategyDCAM::get_side(double pos) {
    return pos < 0?-1:pos > 0?1:0;
}



double StrategyDCAM::calc_order(double price, double cur_pos, double new_price, double feepct) {
    double new_pnl = cur_pos * (new_price - price);
    double side = get_side(price - new_price);
    RuleResult rr = find_k_rule(cur_pos, new_pnl, price, new_price);
    double max_pos = 0.9*_cfg.max_leverage * (_val+_cfg.initial_budget)/new_price;
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

constexpr double StrategyDCAM::find_price_from_pos(double p, double w, double k, double c, double x) {
    return (k * (w - std::asinh((-x * k + x * k * cosh((1 - c) * w))/(p * w))))/w;
}

double StrategyDCAM::calc_target(double cur_price, double target_price) const {
    return (cur_price < target_price? (1.0 - cur_price/target_price):(1.0 - target_price/cur_price))*_cfg.target;
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

}
