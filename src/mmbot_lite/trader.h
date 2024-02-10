#pragma once
#include "market.h"

#include "spread.h"
#include "istorage.h"
#include "strategy.h"
#include "reporting.h"


namespace mmbot {


class Trader {
public:

    using PMarket = std::unique_ptr<IMarket>;
    using PSpread = clone_ptr<ISpread>;
    using PReport = std::unique_ptr<IReport>;
    using PStrategy = clone_ptr<IStrategy>;
    using PStorage = std::unique_ptr<IStorage>;


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



    Trader(Config cfg, PMarket market, PStorage storage, PReport rpt);

    void start();
    void step();

protected:


    MarketInfo minfo;
    PStrategy strategy;
    StrategyState strategy_state;
    PSpread spread;
    PMarket market;
    PStorage storage;
    PReport rpt;


    Tick alert_buy = 0;
    Tick alert_sell = 0;

    TraderReport rpt_data;

    MarketCommand step(const MarketState &state);

    StrategyState &process_state(const MarketState &state);
    void execute_state(const StrategyState &st, const MarketState &state);
    bool restore_trader_state(const IStorage::Fills &fills, const IStorage::History &history);
};


}
