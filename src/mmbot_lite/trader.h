#pragma once
#include "market.h"

#include "spread.h"

#include "strategy.h"
#include "reporting.h"

#include "acb.h"
namespace mmbot {


class Trader {
public:

    using PMarket = std::unique_ptr<IMarket>;
    using PSpread = clone_ptr<ISpread>;
    using PReport = std::unique_ptr<IReport>;
    using PStrategy = clone_ptr<IStrategy>;


    struct SpreadConfig {
        unsigned int mean_points;
        unsigned int stdev_points;
        std::vector<double> curves;
        bool zero_line;
    };

    struct Config {
        PStrategy strategy;
        PSpread spread;
    };



    Trader(Config cfg, PMarket market, PReport rpt);

    void start();
    void step();

protected:


    MarketInfo minfo;
    PStrategy strategy;
    StrategyState strategy_state;
    PSpread spread;
    PMarket market;
    PReport rpt;


    Tick alert_buy = 0;
    Tick alert_sell = 0;

    TraderReport rpt_data;

    MarketCommand step(const MarketState &state);

    StrategyState &process_state(const MarketState &state);
    void execute_state(const StrategyState &st, const MarketState &state);

};


}
