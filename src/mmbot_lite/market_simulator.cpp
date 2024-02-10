#include "market_simulator.h"

#include <sstream>

namespace mmbot {

AbstractMarketSimulator::AbstractMarketSimulator(const MarketInfo &nfo, double equity)
:nfo(nfo),acb(0, 0, equity) {}


template<Side side>
void AbstractMarketSimulator::add_fill(PendingOrder &ord, const Time &tm) {
    constexpr double sd = static_cast<double>(side);
    state.position+=ord.size*static_cast<Lot>(side);
    double price = nfo.tick2price(ord.price);
    double size = nfo.lot2amount(ord.size);
    float fees = nfo.pct_fee*price*size;
    acb = acb.execution(price, size*sd, fees);
    state.pending_orders.buy.reset();
    fills.push_back(Fill{get_id(),tm,ord.price, ord.size,side, fees, nfo.tick_size, nfo.lot_size});
}

void AbstractMarketSimulator::restore_state(const JsonValue &st) {
    state.position = st["pos"].get();
    acb = ACB(st["open"].get(), state.position, st["balance"].get());
    id_counter = st["id"].get();
}
JsonValue AbstractMarketSimulator::store_state() const {
    return {
        {"pos", state.position},
        {"open", acb.getOpen()},
        {"balance",acb.getRPnL()},
        {"is", id_counter}
    };

}

const MarketState& AbstractMarketSimulator::get_state()  {

    SourceData src = fetch_source();

    if (state.pending_orders.buy) {
        PendingOrder &ord = *state.pending_orders.buy;
        if (ord.price == 0) {
            ord.price = src.ask;
        }
        if (ord.price >= src.ask) {
            add_fill<Side::buy>(ord, src.tm);
            state.pending_orders.buy.reset();
        }
    }
    if (state.pending_orders.sell) {
        PendingOrder &ord = *state.pending_orders.sell;
        if (ord.price == 0) {
            ord.price = src.bid;
        }
        if (ord.price <= src.bid) {
            add_fill<Side::sell>(ord, src.tm);
            state.pending_orders.sell.reset();
        }
    }

    state.ask = src.ask;
    state.bid = src.bid;
    state.tp = src.tm;
    state.market_rev = nfo.rev;
    state.equity = acb.getEquity(nfo.tick2price((src.ask+src.bid)>>1));
    state.open_price = acb.getOpen();
    state.fills = fills;
    fills.clear();
    return state;
}

void AbstractMarketSimulator::execute(const MarketCommand &command) {
    if (command.buy) {
        state.pending_orders.buy.emplace(PendingOrder{
            get_id(), command.buy->price, command.buy->size
        });
    } else {
        state.pending_orders.buy.reset();
    }
    if (command.sell) {
        state.pending_orders.sell.emplace(PendingOrder{
            get_id(), command.sell->price, command.sell->size
        });
    } else {
        state.pending_orders.sell.reset();
    }
}

const MarketInfo& AbstractMarketSimulator::get_info() {
    return nfo;
}

std::string AbstractMarketSimulator::get_id() {
    return std::to_string(id_counter++);
}

}
