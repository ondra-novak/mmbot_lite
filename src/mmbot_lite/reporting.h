#pragma once
#include "market.h"
#include "strategy.h"

#include "acb.h"
namespace mmbot {


struct TraderReport {
    double rpnl;
    double upnl;
    double pos;
    double open;
    double suma;

};


class IReport {
public:

    virtual void rpt(const MarketInfo &minfo) = 0;
    virtual void rpt(const MarketState &state) = 0;
    virtual void rpt(const StrategyResult &strategy_result) = 0;
    virtual void rpt(const MarketCommand &mcmd) = 0;
    virtual void rpt(const TraderReport &trpt) = 0;
    virtual ~IReport() = default;
};




}
