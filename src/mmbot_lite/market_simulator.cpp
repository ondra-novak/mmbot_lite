#include "market_simulator.h"

#include <sstream>

namespace mmbot {

AbstractMarketSimulator::AbstractMarketSimulator(const MarketInfo &nfo, double equity)
:nfo(nfo),acb(0, 0, equity) {}


template<Side side>
void AbstractMarketSimulator::add_fill(PendingOrder &ord, const Time &tm) {
    constexpr double sd = side == Side::buy?1:-1;
    state.position+=ord.size;
    double price = nfo.tick2price(ord.price);
    double size = nfo.lot2amount(ord.size);
    acb = acb.execution(price, size*sd);
    state.pending_orders.buy.reset();
    fills.push_back(Fill{get_id(),tm,ord.price, ord.size,side,nfo.pct_fee*price*size});
}




std::string AbstractMarketSimulator::save_state() const {
    std::ostringstream s;
    s << state.position << " " << acb.getRPnL() << " " << acb.getPos() << " " << acb.getSuma() << " " << id_counter;
    return s.str();
}

void AbstractMarketSimulator::restore_state(std::string_view state) {
    std::istringstream s((std::string(state)));
    double rpnl;
    double pos;
    double sum;

    s >> this->state.position >> rpnl >> pos >> sum >> this->id_counter;
    if (pos) {
        acb = ACB(sum/pos, pos, rpnl);
    } else {
        acb = ACB(0,0,rpnl);
    }
    this->state.equity = acb.getRPnL();
}

const MarketState& AbstractMarketSimulator::get_state()  {

    SourceData src = fetch_source();

    if (state.pending_orders.buy) {
        PendingOrder &ord = *state.pending_orders.buy;
        if (ord.price >= src.ask) {
            add_fill<Side::buy>(ord, src.tm);
            state.pending_orders.buy.reset();
        }
    }
    if (state.pending_orders.sell) {
        PendingOrder &ord = *state.pending_orders.sell;
        if (ord.price <= src.bid) {
            add_fill<Side::sell>(ord, src.tm);
            state.pending_orders.sell.reset();
        }
    }

    state.ask = src.ask;
    state.bid = src.bid;
    state.tp = src.tm;
    state.market_rev = nfo.rev;
    state.equity = acb.getRPnL();
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
