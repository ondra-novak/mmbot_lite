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
       0.002,
       1,
       false
    };

    std::fstream f("backtest.csv");
    auto market = std::make_unique<MarketSimulatorReadFile>(nfo, f, 10000);
    auto report = std::make_unique<ReportingCSV>("report.csv");
    Trader::Config cfg = {
            {
                   30,0.2,0.2,10000,10,10
            },
            {
                    21*60,
                    21*60,
                    {0.8,2.4,4.8},
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
