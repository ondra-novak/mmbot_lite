#include "trader.h"

#include "bollinger_spread.h"
namespace mmbot {

Trader::Trader(Config cfg, PMarket market, PReport rpt)
            :strategy(std::move(cfg.strategy))
            ,spread(std::move(cfg.spread))
            ,market(std::move(market))
            ,rpt(std::move(rpt))
{

}

void Trader::start() {
    minfo = market->get_info();
    rpt->rpt(minfo);
    const MarketState &st = market->get_state();
    rpt->rpt(st);
    auto &ss = process_state(st);
    ss.last_exec_price = ss.current_price;
    spread->start(ss.current_price);
    strategy->start(ss);
}

void Trader::step() {
    int att = 1;
    while (true) {
        const MarketState &st = market->get_state();
        if (st.market_rev != minfo.rev) {
            minfo = market->get_info();
            if (minfo.rev != st.market_rev) {
                if (++att>10) break;
                continue;
            }
        }
        auto &ss = process_state(st);
        strategy->event(ss);
        execute_state(ss, st);
        rpt->rpt(st);
        rpt->rpt(ss);
        rpt->rpt(rpt_data);
        return;
    }
    throw std::runtime_error("Failed to update market information consistently (revision mistmatch)");
}

StrategyState &Trader::process_state(const MarketState &state) {
    StrategyState &ss = strategy_state;
    ss.current_time = state.tp;
    ss.current_price = minfo.tick2price((state.ask+state.bid)/2);
    ss.min_size = std::max(minfo.lot2amount(minfo.min_size), minfo.min_volume/ss.current_price);
    ss.leverage = minfo.leverage;


    if (!state.fills.empty()) {
        ACB total_fill;
        for (const Fill &f: state.fills) {
            double price = minfo.tick2price(f.price);
            double size = minfo.lot2amount(f.size) * static_cast<double>(f.side);

            total_fill = total_fill.execution(price, size, f.fee);
            rpt_data.pnl = rpt_data.pnl.execution(price, size, f.fee);
        }
        double fill_price = total_fill.getOpen();
        double fees = total_fill.getRPnL();
        double fill_size = total_fill.getPos();
        double feepct = fees/(fill_price*fill_size);
        double adj_price = fill_price - fill_price * feepct;

        ss.last_exec_price = adj_price;
        ss.last_exec_change = fill_size;
        ss.position += fill_size;
        ss.execution = true;
    } else {
        ss.execution = false;
        if (alert_sell && state.bid > alert_sell) {
            double aprice = minfo.tick2price(alert_sell);
            ss.last_exec_price = aprice;
            ss.alert = true;
            alert_sell = 0;
        } else if (state.ask < alert_buy) {
            double aprice = minfo.tick2price(alert_buy);
            ss.last_exec_price = aprice;
            ss.alert = true;
            alert_buy = 0;
        } else {
            ss.alert = false;
        }

    }
    ss.buy.reset();
    ss.sell.reset();
    if (ss.execution || ss.alert) {
        spread->execution(ss.last_exec_price);
        spread->point(ss.current_price);
    } else {
        double eq = ss.equilibrium?ss.equilibrium:ss.last_exec_price;
        spread->point(ss.current_price);
        auto sres = spread->get_result(eq);
        if (sres.buy.has_value()) {
            ss.buy.emplace(StrategyState::Order{*sres.buy*(1+minfo.pct_fee)});
        }
        if (sres.sell.has_value()) {
            ss.sell.emplace(StrategyState::Order{*sres.sell*(1-minfo.pct_fee)});
        }
        ss.buy_flag = sres.buy_flag;
        ss.sell_flag = sres.sell_flag;
        if (ss.buy.has_value() && ss.sell.has_value() && ss.buy->price >= ss.sell->price) {
            ss.buy.reset();
            ss.sell.reset();
        }
    }
    return ss;
}

void Trader::execute_state(const StrategyState &st, const MarketState &state) {
    MarketCommand cmd;
    alert_buy = alert_sell = 0;

    if (st.buy.has_value()) {
        Tick tick =  minfo.price2tick(st.buy->price / (1+minfo.pct_fee));
        Lot lot = minfo.amount2lot(st.buy->size);
        if (tick >= state.ask) tick = state.ask-1;
        if (lot) {
            cmd.buy.emplace(MarketCommand::Order{tick,lot,true});
        } else {
            alert_buy = tick;
        }
    }
    if (st.sell.has_value()) {
        Tick tick =  minfo.price2tick(st.sell->price / (1-minfo.pct_fee));
        Lot lot = minfo.amount2lot(st.sell->size);
        if (tick <= state.bid) tick = state.bid-1;
        if (lot) {
            cmd.sell.emplace(MarketCommand::Order{tick,lot,true});
        } else {
            alert_sell = tick;
        }
    }
    if (st.market_order > 0) {
        Lot lot = minfo.lot2amount(st.market_order);
        if (lot) {
            cmd.buy.emplace(MarketCommand::Order{0,lot,false});
        }
    }
    if (st.market_order < 0) {
        Lot lot = minfo.lot2amount(-st.market_order);
        if (lot) {
            cmd.sell.emplace(MarketCommand::Order{0,lot,false});
        }
    }
    cmd.allocate = st.allocation;
    market->execute(cmd);
}

}
