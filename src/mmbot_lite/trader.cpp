#include "trader.h"

#include "bollinger_spread.h"
namespace mmbot {

Trader::Trader(const Config &cfg, PMarket market)
           :market(std::move(market))
{

    PSpread spread (new BollingerSpread(
            cfg.spread.mean_points,
            cfg.spread.stdev_points,
            cfg.spread.curves,
            cfg.spread.zero_line
    ));
    spreadState = spread->start();
    strategy.emplace(cfg.strategy, std::move(spread));
}

void Trader::start() {
    minfo = market->get_info();
    const MarketState &st = market->get_state();
    last = (st.ask + st.bid) / 2;
    last_fill = last;
    if (std::isfinite(st.open_price)) {
        last_fill = minfo.price2tick(st.open_price);
    }
    position = st.position;
    strategy->start(process_state(st), std::move(spreadState));
    auto cmd = step(st);
    market->execute(cmd);

}

void Trader::step() {
    const MarketState &st = market->get_state();
    auto cmd = step(st);
    market->execute(cmd);
}

MarketCommand Trader::step(const MarketState &state) {
    StrategyMarketCommand mcmd = strategy->step(process_state(state));
    //TODO




}

StrategyMarketState Trader::process_state(const MarketState &state) {
    StrategyMarketState ret;
    last = std::max(state.bid, std::min(state.ask, last));
    ret.ask = minfo.tick2price(state.ask);
    ret.bid = minfo.tick2price(state.bid);
    ret.last = minfo.tick2price(last);

    if (!state.fills.empty()) {
        double fees =0;
        double pnl = 0;
        for (const Fill &f: state.fills) {
            double p = minfo.calc_pnl(last_fill, f.price, position);
            pnl += p;
            fees += f.fee;
            if (f.side == Side::buy) position += f.size; else position -= f.size;
            last_fill = f.price;

        }
        ret.last_exec_price = minfo.tick2price(last_fill);
        ret.execution = true;
        ret.pnl = pnl - fees;
    } else {
        double p = minfo.calc_pnl(last_fill, last, position);
        ret.pnl = p - p * minfo.pct_fee;
    }
    ret.min_size = minfo.lot2amount(minfo.min_lot(last));
    return ret;
}

}
