#include "trader.h"

#include "bollinger_spread.h"
namespace mmbot {

Trader::Trader(const Config &cfg, PMarket market, PReport rpt)
           :market(std::move(market))
           ,rpt(std::move(rpt))
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
    rpt->rpt(minfo);
    const MarketState &st = market->get_state();
    rpt->rpt(st);
    last = (st.ask + st.bid) / 2;
    last_fill = minfo.tick2price(last);
    if (std::isfinite(st.open_price)) {
        last_fill = minfo.price2tick(st.open_price);
    }
    position = st.position;
    strategy->start(process_state(st), std::move(spreadState));
    auto cmd = step(st);
    rpt->rpt(cmd);
    market->execute(cmd);

}

void Trader::step() {
    const MarketState &st = market->get_state();
    rpt->rpt(st);
    auto cmd = step(st);
    rpt->rpt(cmd);
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

    StrategyResult mcmd = strategy->step(process_state(state));

    rpt->rpt(mcmd);

    MarketCommand ret;
    ret.allocate = mcmd.allocate;

    alert_buy = 0;
    alert_sell = 0;

    setOrder<-1>(minfo, mcmd.buy, alert_buy, ret.buy, state.ask);
    setOrder<1>(minfo, mcmd.sell, alert_sell, ret.sell, state.bid);

    rpt->rpt(TraderReport{
        pnl.getRPnL(), minfo.acb_get_upnl(pnl, last),
        minfo.acb_get_position(pnl),
        minfo.acb_get_open(pnl),
        std::abs(pnl.getSuma())

    });

    return ret;



}

StrategyMarketState Trader::process_state(const MarketState &state) {
    StrategyMarketState ret = {};
    last = std::max(state.bid, std::min(state.ask, last));
    ret.minfo = &minfo;
    ret.ask = minfo.tick2price(state.ask);
    ret.bid = minfo.tick2price(state.bid);
    ret.last = minfo.tick2price(last);
    ret.prev_exec_price = last_fill;
    ret.prev_position =  position;

    if (!state.fills.empty()) {
        ACB total_fill;
        for (const Fill &f: state.fills) {
            total_fill = minfo.update_acb(total_fill, f.price, f.size, f.side, f.fee);
            pnl = minfo.update_acb(pnl, f.price, f.size, f.side, f.fee);
        }
        double fill_price = minfo.acb_get_open(total_fill);
        double fees = -total_fill.getRPnL();
        double pnl = minfo.calc_pnl(last_fill, fill_price, position);
        last_fill = fill_price;
        ret.execution = true;
        ret.pnl = pnl - fees;
        position = this->pnl.getPos();
    } else {

        if (alert_sell && state.bid > alert_sell) {
            double aprice = minfo.tick2price(alert_sell);
            ret.pnl =  minfo.calc_pnl(last_fill, aprice, position);
            ret.alert = true;
            last_fill = aprice;
            ret.position = position;
            alert_sell = 0;
        } else if (state.ask < alert_buy) {
            double aprice = minfo.tick2price(alert_buy);
            ret.pnl =  minfo.calc_pnl(last_fill, aprice, position);
            ret.alert = true;
            last_fill = aprice;
            ret.position = position;
            alert_sell = 0;
        } else {
            ret.pnl = minfo.calc_pnl(last_fill, minfo.tick2price(last), position);
        }
        ret.pnl -= (minfo.inverted?std::abs(position):last_fill*std::abs(position))*minfo.pct_fee;
    }
    ret.last_exec_price = last_fill;
    ret.position = position;
    ret.cost = std::abs(pnl.getSuma());
    return ret;
}

}
