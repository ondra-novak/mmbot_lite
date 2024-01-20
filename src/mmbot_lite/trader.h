#pragma once
#include "market.h"

#include "spread.h"

#include "StrategyDCAM.h"
#include "reporting.h"

#include "acb.h"
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
    using PReport = std::unique_ptr<IReport>;


    Trader(const Config &cfg, PMarket market, PReport rpt);

    void start();
    void step();

protected:



    std::optional<StrategyDCAM> strategy;
    PSpreadState spreadState;
    PMarket market;
    PReport rpt;
    MarketInfo minfo;

    Tick last = 0;
    double position = 0;
    double last_fill = 0;

    Tick alert_buy = 0;
    Tick alert_sell = 0;

    ACB pnl;


    MarketCommand step(const MarketState &state);

    StrategyMarketState process_state(const MarketState &state);

};


}
