#pragma once
#include "market.h"
#include "market_account.h"

#include <cstdint>

namespace mmbot {

class AbstractMarketSimulator: public IMarket {
public:

    struct SourceData {
        Time tm;
        Tick bid;
        Tick ask;
    };


    AbstractMarketSimulator(const MarketInfo &nfo, double equity);

    virtual const MarketState &get_state() override;
    virtual void execute(const MarketCommand &command) override;
    virtual void restore(const PersistentStorage &state) override;
    virtual void store(PersistentStorage &state) const override;
    virtual const MarketInfo &get_info() override;
    virtual SourceData fetch_source() = 0;

    virtual std::shared_ptr<IMarketAccount> get_account() const {return nullptr;}

protected:
    MarketInfo nfo;
    MarketState state = {};
    ACB acb;
    std::uint64_t id_counter;
    std::string get_id();
    Fills fills;

    template<Side side>
    void add_fill(PendingOrder &ord, const Time &tm);


    enum class Field {
        position,
        balance,
        open_price,
        id_counter,
    _count};



};


}
