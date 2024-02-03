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

void AbstractMarketSimulator::restore(const PersistentStorage &st) {
    if (st.has_count(Field::_count)) {
        state.position = st[Field::position].as<Lot>();
        acb = ACB(st[Field::open_price].as<double>(), state.position, st[Field::balance].as<double>());
        id_counter = st[Field::id_counter].as<std::uint64_t>();
    }



}
void AbstractMarketSimulator::store(PersistentStorage &st) const {
    st.set_count(Field::_count);
    st[Field::position] = state.position;
    st[Field::balance] = acb.getRPnL();
    st[Field::open_price] = acb.getOpen();
    st[Field::id_counter] = id_counter;
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
