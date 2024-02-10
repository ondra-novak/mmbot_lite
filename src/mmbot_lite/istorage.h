#pragma once


#include "market.h"

#include "strategy.h"
namespace mmbot {

class IStorage {
public:

    struct Ticker {
        Time tm;
        double bid;
        double ask;
        double price() const {return (bid+ask)*0.5;}
    };

    struct Fill {
        Time tm;
        double avg_price;
        double size;
        double fee;
    };

    using Fills = std::vector<Fill>;
    using History = std::vector<Ticker>;

    virtual ~IStorage() = default;

    virtual void store_fill(const Fill &fill) = 0;
    virtual void store_ticker(const Ticker &ticker) = 0;
    virtual void store_trader_state(const JsonValue &state) = 0;

    virtual History read_history(std::size_t count) const = 0;
    virtual Fills read_fills() const = 0;
    virtual JsonValue restore_trader_state() const = 0;



};


}
