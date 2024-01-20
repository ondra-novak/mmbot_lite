#pragma once

namespace mmbot {

struct MarketInfo;

struct StrategyMarketState {

    const MarketInfo *minfo;
    double bid = 0;
    double ask = 0;
    double last = 0;
    double position = 0;
    double prev_position = 0;
    double last_exec_price = 0;
    double prev_exec_price = 0;
    double cost = 0;            ///<cost of position
    double pnl = 0;             ///<equity change,  fees are substracted
    bool execution = false;
    bool alert = false;
};



struct StrategyOrder {
    static constexpr double no_order = 0;
    static constexpr double alert = -1;

    double price = 0;   //if price == 0, market order
    double size = 0;    //if size == 0, order is disabled (even if price == 0)
};


struct StrategyResult {
    StrategyOrder buy = {};
    StrategyOrder sell = {};
    double allocate = 0.0;
    double neutral_price = 0.0;
};


}
