#pragma once

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


struct PersistentValue {


    enum Type {
        empty,
        boolean,
        integer,
        floating,
        datetime,
    };

    Type type;
    union {
        bool b_val;
        int i_val;
        double f_val;
        std::int64_t t_val;
    };

    PersistentValue():type(empty) {}
    PersistentValue(bool b):type(boolean),b_val(b) {}
    PersistentValue(int v):type(integer),i_val(v) {}
    PersistentValue(double v):type(floating),f_val(v) {}
    PersistentValue(std::chrono::system_clock::time_point v):type(datetime),t_val(v.time_since_epoch().count()) {}

    template<typename Fn>
    auto visit(Fn &&fn) const {
        switch (type) {
            default:
            case empty: return fn(nullptr);break;
            case boolean: return fn(b_val);break;
            case integer: return fn(i_val);break;
            case floating: return fn(f_val);break;
            case datetime: return fn(std::chrono::system_clock::time_point(
                    std::chrono::system_clock::duration(t_val)));break;
        }
    }

    template<typename T>
    T as() const {
        return visit([&](auto x) -> T {
            if constexpr(std::is_constructible_v<T, decltype(x)>) {
                return T(x);
            } else {
                return {};
            }
        });
    }
};

template<typename T>
concept Enumerator = std::is_enum_v<T>;

class PersistentStorage : public std::vector<PersistentValue> {
public:

    template<Enumerator T>
    PersistentValue &operator[](T v) {
        return std::vector<PersistentValue>::operator [](static_cast<unsigned int>(v));
    }
    template<Enumerator T>
    const PersistentValue &operator[](T v) const {
        return std::vector<PersistentValue>::operator [](static_cast<unsigned int>(v));
    }

    template<typename T, Enumerator ... Args>
    auto as(Args ... items) const {
        return std::make_tuple(std::vector<PersistentValue>::operator [](static_cast<unsigned int>(items)).as<T>()...);
    }

    template<Enumerator T>
    void set_count(T count_value) {
        std::vector<PersistentValue>::resize(static_cast<unsigned int>(count_value));
    }
    template<Enumerator T>
    bool has_count(T count_value) const {
        return std::vector<PersistentValue>::size() >= static_cast<unsigned int>(count_value);
    }

};



///UserStateField is state field, which is presented to the user
/**
 * The strategy can convert values to be more informative for user
 */

struct UserStateField{
    enum Transform {
        ///no transformation
        normal,
        ///enumeration
        enumeration,
        ///value is price (transform can be applied for inverse market)
        price,
        ///value is position (transform can be applied for inverse market)
        position,

    };
    Transform transform;
    PersistentValue value;
    std::string_view name;
    std::string_view values;
};

using UserState = std::vector<UserStateField>;

class IStrategy {
public:

    virtual ~IStrategy() = default;

    ///strategy is initialized, this is called when there is not persistent storage yet
    virtual void start(StrategyState &state) = 0;
    ///called every cycle (1 minute)
    virtual void event(StrategyState &state) = 0;
    ///store state of the strategy into persistent storage
    virtual void store(PersistentStorage &storage) const = 0;
    ///restore state of the strategy - it is called instead start()
    virtual bool restore(const PersistentStorage &storage) = 0;

    virtual IStrategy *clone() const = 0;

};



}
