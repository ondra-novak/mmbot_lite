#pragma once
#include "market.h"

#include "spread.h"

#include "StrategyDCAM.h"
namespace mmbot {


class Trader {
public:


    struct SpreadConfig {
        unsigned int mean_points;
        unsigned int stdev_points;
        std::vector<double> curves;
        bool zero_line;
    };

    struct Config {
        StrategyDCAM::Config strategy;
        SpreadConfig spread;
    };

    using PMarket = std::unique_ptr<IMarket>;
    using PSpread = clone_ptr<ISpreadGen>;
    using PSpreadState = clone_ptr<ISpreadGen::State>;


    Trader(const Config &cfg, PMarket market);

    void start();
    void step();

protected:



    std::optional<StrategyDCAM> strategy;
    PSpreadState spreadState;
    PMarket market;
    MarketInfo minfo;

    Tick last = 0;
    Tick position = 0;
    Tick last_fill = 0;


    MarketCommand step(const MarketState &state);

    StrategyMarketState process_state(const MarketState &state);

};


}
