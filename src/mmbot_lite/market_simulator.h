#pragma once
#include "market.h"


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
    virtual void restore_state(std::string_view state) override;
    virtual std::string save_state() const override;
    virtual const MarketInfo &get_info() override;
    virtual SourceData fetch_source() = 0;

protected:
    MarketInfo nfo;
    MarketState state = {};
    ACB acb;
    std::uint64_t id_counter;
    std::string get_id();
    Fills fills;

    template<Side side>
    void add_fill(PendingOrder &ord, const Time &tm);





};


}
