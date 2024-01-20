#pragma once
#include "reporting.h"

#include <fstream>
namespace mmbot {

class ReportingCSV: public IReport {
public:

    ReportingCSV(std::string ofile);
    ~ReportingCSV();
    virtual void rpt(const MarketInfo &minfo) override;
    virtual void rpt(const StrategyResult &strategy_result) override;
    virtual void rpt(const MarketCommand &mcmd) override;
    virtual void rpt(const MarketState &state) override;
    virtual void rpt(const TraderReport &trpt) override;

protected:
    MarketInfo nfo;
    StrategyResult sr;
    MarketState state;
    TraderReport trpt;

    std::ofstream output;

    bool strategy_output = false;
    bool state_output = false;
    bool trader_output = false;

    void activate_section(volatile bool &what);
    void add_to_report();

};

}
