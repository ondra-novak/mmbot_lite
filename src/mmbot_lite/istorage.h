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
    };

    virtual ~IStorage() = default;

    virtual void store_fill(const Fill &fill) = 0;
    virtual void store_ticker(const Ticker &ticker) = 0;
    virtual void store_strategy_state(const PersistentStorage &strategy_state) = 0;

    virtual std::vector<Ticker> read_history(std::size_t count) const = 0;
    virtual Fills read_fills() const = 0;
    virtual PersistentStorage restore_strategy_state() const = 0;



};


}
