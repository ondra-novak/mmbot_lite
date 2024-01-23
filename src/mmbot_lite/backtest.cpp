#include "market_simulator_read_file.h"
#include "trader.h"

#include "reporting_csv.h"

#include <fstream>


int main() {
    using namespace mmbot;

    MarketInfo nfo{
       1,
       0.01,
       0.00001,
       100,
       0.00001,
       0.001,
       1,
       false
    };

    std::fstream f("backtest.csv");
    auto market = std::make_unique<MarketSimulatorReadFile>(nfo, f, 10000);
    auto report = std::make_unique<ReportingCSV>("report.csv");
    Trader::Config cfg = {
            {
                   8,0.93,10000,0.1,10000
            },
            {
                    10*60,
                    10*60,
                    {1,3,5},
                    true,
            }
    };

    try {

        Trader trd(cfg, std::move(market), std::move(report));
        trd.start();
        do {
            trd.step();
        } while(true);


    } catch (const MarketSimulatorReadFile::EndOfDataException &) {
        //empty
    }

}
