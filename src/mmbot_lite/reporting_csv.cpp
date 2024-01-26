#include "reporting_csv.h"
#include <numeric>

#include "acb.h"

#include <iomanip>
namespace mmbot {

ReportingCSV::ReportingCSV(std::string ofile) {
    output.open(ofile);
    if (!output) throw std::runtime_error("Failed to open report for writing");
    output << "time,bid,ask,equity,open_price,position,fill_price,fill_size,fill_fee,"
             "buy_price,buy_size,sell_price,sell_size,"
            "allocated,neutral,equi,rpnl" << std::endl;
}



void ReportingCSV::rpt(const MarketInfo &minfo) {
    nfo = minfo;
}

void ReportingCSV::rpt(const StrategyState &strategy_result) {
    activate_section(strategy_output);
    sr = strategy_result;
}

void ReportingCSV::rpt(const MarketCommand &) {
    //empty

}

void ReportingCSV::rpt(const MarketState &state) {
    activate_section(state_output);
    this->state = state;
}

void ReportingCSV::rpt(const TraderReport &trpt) {
    activate_section(trader_output);
    this->trpt = trpt;
}


void ReportingCSV::activate_section(volatile bool &what) {
    if (what) {
        add_to_report();
        strategy_output = false;
        state_output = false;
        trader_output = false;
    }
    what = true;
}


ReportingCSV::~ReportingCSV() {
    add_to_report();
}

void ReportingCSV::add_to_report() {
    if (state_output) {
        auto t = std::chrono::system_clock::to_time_t(state.tp-std::chrono::days(400));
        output << std::put_time(gmtime(&t), "%F %T") << ","
               << nfo.tick2price(state.bid) << ","
               << nfo.tick2price(state.ask) << ","
               << state.equity << ","
               << state.open_price << ","
               << nfo.lot2amount(state.position);
        if (!state.fills.empty()) {
            ACB fillsum = std::accumulate(state.fills.begin(), state.fills.end(), ACB(), [&](const ACB &x, const Fill &f){
                return x.execution(nfo.tick2price(f.price), nfo.lot2amount(f.size)*static_cast<double>(f.side), f.fee);
            });
            output << "," << fillsum.getOpen() << "," << fillsum.getPos() << "," << fillsum.getRPnL();
        } else {
            output << ",,,";
        }
        if (state.pending_orders.buy) {
            output << ',' << nfo.tick2price(state.pending_orders.buy->price);
            output << ',' << nfo.lot2amount(state.pending_orders.buy->size);
        } else {
            output << ",,";
        }
        if (state.pending_orders.sell) {
            output << ',' << nfo.tick2price(state.pending_orders.sell->price);
            output << ',' << nfo.lot2amount(state.pending_orders.sell->size);
        } else {
            output << ",,";
        }
    } else {
        output << ",,,,,,,,,,,,";
    }
    if (strategy_output) {
        output << "," << sr.allocation ;
        output << "," << sr.neutral_price;
        output << "," << sr.equilibrium;
    } else {
        output << ",,,";
    }
    if (trader_output) {
        output << "," << trpt.pnl.getEquity(sr.current_price);
    } else {
        output << ",";
    }
    output << std::endl;
}

}
