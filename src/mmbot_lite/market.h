#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <vector>

namespace mmbot {

using Tick = long;
using Lot = long;
using Revision = unsigned long;


struct MarketInfo {
    //revision of this information. If revision changes, you know, that you need to update info
    Revision rev;
    double tick_size;
    double lot_size;
    double leverage;
    double min_volume;
    double pct_fee;
    Lot min_size;
    bool inverted;


    constexpr Tick price2tick(double price) const {
        double adj = price>0?0.5:-0.5;
        return static_cast<Tick>((price+tick_size*adj)/tick_size);
    }
    constexpr double tick2price(Tick tick) const {
        return tick * tick_size;
    }
    constexpr Lot amount2lot(double amount) const {
        double adj = amount>0?0.5:-0.5;
        return static_cast<Lot>((amount+lot_size*adj)/lot_size);
    }
    constexpr double lot2amount(Lot lot) const {
        return lot * lot_size;
    }
    constexpr Lot min_lot(Tick at_price) const {
        double price = tick2price(at_price);
        double minsz = min_volume/price;
        return std::max(min_size, static_cast<Lot>(minsz/lot_size)+1);
    }

    constexpr double calc_pnl(Tick open, Tick close, Lot amount) {
        if (inverted) {
            return -(1.0/tick2price(open)+1.0/tick2price(close))*lot2amount(amount);
        } else {
            return (tick2price(close)-tick2price(open))*lot2amount(amount);
        }
    }
};

using OrderID = std::string;

enum class Side: int {
    buy = 1,
    sell = -1
};

struct PendingOrder {
    OrderID id;
    Tick price;
    Lot size;
};

using FillID = std::string;
using Time = std::chrono::system_clock::time_point;

struct Fill {
    FillID id;
    Time tp;
    Tick price;
    Lot size;
    Side side;
    double fee;
};

using Fills = std::vector<Fill>;

struct PendingOrders {
    std::optional<PendingOrder> buy;
    std::optional<PendingOrder> sell;
};


struct MarketState {
    //time of last update
    Time tp = {};
    //contains revision of the market info
    Revision market_rev = 0;
    PendingOrders pending_orders = {};
    Fills fills = {};
    Lot position = 0;
    Tick bid = 0;
    Tick ask= 0;
    double equity = 0;
    double open_price = 0;
};

using TraderID = std::string;

struct MarketCommand {

    struct Order {
        Tick price;
        Lot size;
        bool post_only;
    };

    TraderID id;
    std::optional<Order> buy;
    std::optional<Order> sell;
    ///allocated equity for this trader
    double allocate;
};


class IMarket {
public:

    virtual ~IMarket() = default;

    ///restore saved state
    ///
    virtual void restore_state(std::string_view state) = 0;
    ///save state
    /** Allows to store state of this object inside of trader's state
     * This preserves state between restarts etcs
     * */
    virtual std::string save_state() const = 0;
    ///get market info
    virtual const MarketInfo &get_info() = 0;
    ///get market state
    virtual const MarketState &get_state()= 0;
    virtual void execute(const MarketCommand &command) = 0;
};


}
