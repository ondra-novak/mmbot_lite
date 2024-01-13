#pragma once

#include "market_simulator.h"
#include <iostream>

#include <exception>
namespace mmbot {

class MarketSimulatorReadFile : public AbstractMarketSimulator {
public:

    class EndOfDataException: public std::exception {
    public:
        const char *what() const noexcept override {return "End of data";}
    };

    MarketSimulatorReadFile (const MarketInfo &info, std::istream &stream, double equity);
    virtual AbstractMarketSimulator::SourceData fetch_source() override;

protected:
    std::istream &stream;
    std::chrono::system_clock::time_point tp;

};

}
