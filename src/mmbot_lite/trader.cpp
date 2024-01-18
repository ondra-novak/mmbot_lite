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

template<int dir>
static void setOrder(const MarketInfo &minfo,
        const StrategyOrder &srcOrder,
        Tick &alert,
        std::optional<MarketCommand::Order> &trgOrder,
        Tick askbid) {

    if (srcOrder.size == srcOrder.alert) {
        if (srcOrder.price) {
            alert = minfo.price2tick(srcOrder.price);
        }
    } else if (srcOrder.size > 0) {
        Lot amount = minfo.amount2lot(srcOrder.size);
        if (amount > 0) {
            if (srcOrder.price) {
                Tick t = minfo.price2tick(srcOrder.price);
                if constexpr(dir < 0) {
                    t = std::min(t, askbid-1);
                } else {
                    t = std::max(t, askbid+1);
                }
                trgOrder.emplace(MarketCommand::Order{
                    t,
                    amount,
                    true
                });
            } else {
                trgOrder.emplace(MarketCommand::Order{
                    askbid,
                    amount,
                    false
                });
            }
        }
    }

}

MarketCommand Trader::step(const MarketState &state) {

    StrategyMarketCommand mcmd = strategy->step(process_state(state));

    MarketCommand ret;
    ret.allocate = mcmd.allocate;

    alert_buy = 0;
    alert_sell = 0;

    setOrder<-1>(minfo, mcmd.buy, alert_buy, ret.buy, state.ask);
    setOrder<1>(minfo, mcmd.sell, alert_sell, ret.sell, state.bid);
    return ret;



}

StrategyMarketState Trader::process_state(const MarketState &state) {
    StrategyMarketState ret = {};
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
        ret.execution = true;
        ret.pnl = pnl - fees;
    } else {

        if (alert_sell && state.bid > alert_sell) {
            ret.pnl =  minfo.calc_pnl(last_fill, alert_sell, position);
            ret.alert = true;
            ret.last_exec_price = alert_sell;
            ret.position = position;
            alert_sell = 0;
        } else if (state.ask < alert_buy) {
            ret.pnl =  minfo.calc_pnl(last_fill, alert_buy, position);
            ret.alert = true;
            ret.last_exec_price = alert_buy;
            ret.position = position;
            alert_sell = 0;
        } else {
            ret.pnl = minfo.calc_pnl(last_fill, last, position);
        }
        ret.pnl -= minfo.calc_fee(last_fill, position);
    }
    ret.min_size = minfo.lot2amount(minfo.min_lot(last));
    ret.last_exec_price = minfo.tick2price(last_fill);
    ret.position = minfo.position(position);
    return ret;
}

}
