#pragma once

#include "types/json.h"
#include "spread.h"
#include <chrono>
#include <optional>
#include <string_view>
#include <vector>

namespace mmbot {

struct MarketInfo;

struct StrategyMarketState {

    const MarketInfo *minfo;
    double bid = 0;
    double ask = 0;
    double last = 0;
    double position = 0;
    double prev_position = 0;
    double last_exec_price = 0;
    double prev_exec_price = 0;
    double cost = 0;            ///<cost of position
    double pnl = 0;             ///<equity change,  fees are substracted
    bool execution = false;
    bool alert = false;
};



struct StrategyOrder {
    static constexpr double no_order = 0;
    static constexpr double alert = -1;

    double price = 0;   //if price == 0, market order
    double size = 0;    //if size == 0, order is disabled (even if price == 0)
};


struct StrategyResult {
    StrategyOrder buy = {};
    StrategyOrder sell = {};
    double allocate = 0.0;
    double neutral_price = 0.0;
    double loss = 0.0;
};


///Base class for strategy state.
/** The strategy must create own version. This state is persistent during
 * Trader's lifetime, so strategy can store various variables within the state
 */
class StrategyState {
public:
    struct Order {
        double price = 0.0;
        double size = 0.0;
    };


    ///market leverage (0 for spot)
    double leverage;
    ///minimal order size (smaller orders are removed)
    double min_size;
    ///current trading time (can be real time or simulation time)
    std::chrono::system_clock::time_point current_time;
    ///current position
    double position = 0;
    ///last position change
    double last_exec_change = 0;
    ///last execution price
    double last_exec_price = 0;
    ///current average price
    double current_price = 0;
    ///is set to true, if execution happened from last call
    bool execution = false;
    ///is set to true, if allet happened from last call
    bool alert = false;
    ///order, the Trader prefills price, strategy must fill size (can adjust price)
    std::optional<Order> buy;
    ///order, the Trader prefills price, strategy must fill size (can adjust price)
    std::optional<Order> sell;

    ISpread::Flag buy_flag;

    ISpread::Flag sell_flag;

    ///fills this value to perform market order
    double market_order = 0.0;
    ///strategy fills: neutral price
    double neutral_price = 0.0;
    ///strategy fills: allocated balance
    double allocation = 0;
    ///strategy fills: price where generated size is exactly zero. If zero, last_exec_price is used.
    double equilibrium = 0;
    ///strategy fills: minimal tradable price
    double min_tradable_price = 0;
    ///strategy fills: maximal tradable price
    double max_tradable_price = 0;


};


class IStrategy {
public:

    virtual ~IStrategy() = default;

    ///strategy is initialized, this is called when there is not persistent storage yet
    virtual void start(StrategyState &state) = 0;
    ///called every cycle (1 minute)
    virtual void event(StrategyState &state) = 0;
    ///store state of the strategy into persistent storage
    virtual JsonValue store_state() const = 0;
    ///restore state of the strategy - it is called instead start()
    virtual bool restore_state(const JsonValue &state) = 0;

    virtual IStrategy *clone() const = 0;

};



}
