#pragma once

#include "types/json.h"
#include "math/acb.h"



#include <string>
#include <chrono>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>


#include <tuple>
namespace mmbot {

using Tick = long;
using Lot = long;
using Revision = unsigned long;

using OrderID = std::string;

struct PendingOrder {
    OrderID id;
    Tick price;
    Lot size;
};

using FillID = std::string;
using Time = std::chrono::system_clock::time_point;

enum class Side: int {
    buy = 1,
    sell = -1
};

struct Fill {
    FillID id;
    Time tp;
    Tick price;
    Lot size;
    Side side;
    float fee;
    float tick_size;    //tick size in time of execution to reconstruct price
    float lot_size;     //lot size in time of execution to reconstruct size

    template<typename Me, typename Arch>
    static auto serialize(Me &me, Arch &arch) {
        return arch(me.id, me.tp, me.price, me.size, me.side, me.fee, me.tick_size, me.lot_size);
    }
};

struct MarketInfo {
    //revision of this information. If revision changes, you know, that you need to update info
    Revision rev;
    float tick_size;
    float lot_size;
    float leverage;
    float min_volume;
    float pct_fee;
    Lot min_size;
    //name of symbol
    std::string_view symbol_name = {};
    //name of position (currency of position)
    std::string_view position_currency_name = {};
    //equity currency name (currency of equity)
    std::string_view equity_currency_name = {};
    //quote currency name
    std::string_view quote_currency_name = {};


    constexpr Tick price2tick(double price) const {
        double adj = price>0?0.5:-0.5;
        return static_cast<Tick>((price+tick_size*adj)/tick_size);
    }

    template<unsigned int n>
    auto price2tick(const double (&arr)[n]) const {
        return [&]<std::size_t... idx>(std::index_sequence<idx...>) {
            return std::make_tuple(price2tick(arr[idx]) ...);
        }(std::make_index_sequence<n>());
    }

    constexpr double tick2price(Tick tick) const {
        return tick * tick_size;
    }

    template<unsigned int n>
    auto tick2price(const Tick (&arr)[n]) const {
        return [&]<std::size_t... idx>(std::index_sequence<idx...>) {
            return std::make_tuple(tick2price(arr[idx]) ...);
        }(std::make_index_sequence<n>());
    }

    constexpr Lot amount2lot(double amount) const {
        double adj = amount>0?0.5:-0.5;
        return static_cast<Lot>((amount+lot_size*adj)/lot_size);
    }

    template<unsigned int n>
    auto amount2lot(const double (&arr)[n]) const {
        return [&]<std::size_t... idx>(std::index_sequence<idx...>) {
            return std::make_tuple(amount2lot(arr[idx]) ...);
        }(std::make_index_sequence<n>());
    }


    constexpr double lot2amount(Lot lot) const {
        return lot * lot_size;
    }

    template<unsigned int n>
    auto lot2amount(const Lot (&arr)[n]) const {
        return [&]<std::size_t... idx>(std::index_sequence<idx...>) {
            return std::make_tuple(lot2amount(arr[idx]) ...);
        }(std::make_index_sequence<n>());
    }

};

using Fills = std::vector<Fill>;

struct PendingOrders {
    std::optional<PendingOrder> buy;
    std::optional<PendingOrder> sell;
};


struct MarketState {
    //time of last update
    Time tp = {};
    ///contains revision of the market info
    Revision market_rev = 0;
    PendingOrders pending_orders = {};
    Fills fills = {};
    Lot position = 0;
    Tick bid = 0;
    Tick ask= 0;
    double equity = 0;
    double open_price = 0;
};

using TraderID = std::uint64_t;

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


class IMarketAccount;

class IMarket {
public:

    virtual ~IMarket() = default;


    virtual void restore_state(const JsonValue &state) = 0;
    virtual JsonValue store_state() const = 0;

    ///get market info
    virtual const MarketInfo &get_info() = 0;
    ///get market state
    virtual const MarketState &get_state()= 0;
    virtual void execute(const MarketCommand &command) = 0;

    virtual std::shared_ptr<IMarketAccount> get_account() const = 0;
};



}
