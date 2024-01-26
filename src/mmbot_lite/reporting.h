#pragma once
#include "market.h"
#include "strategy.h"

#include "acb.h"
namespace mmbot {


struct TraderReport {
    ACB pnl;

};


class IReport {
public:

    virtual void rpt(const MarketInfo &minfo) = 0;
    virtual void rpt(const MarketState &state) = 0;
    virtual void rpt(const StrategyState &strategy_result) = 0;
    virtual void rpt(const MarketCommand &mcmd) = 0;
    virtual void rpt(const TraderReport &trpt) = 0;
    virtual ~IReport() = default;
};




}
