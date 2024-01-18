#include "market_simulator_read_file.h"
#include "trader.h"
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
    Trader::Config cfg = {
            {
                    50,0.2,0.2,10000,100
            },
            {
                    15*60,
                    15*60,
                    {1,3,5},
                    true,
            }
    };

    try {

        Trader trd(cfg, std::move(market));
        trd.start();
        do {
            trd.step();
        } while(true);


    } catch (const MarketSimulatorReadFile::EndOfDataException &) {
        //empty
    }

}
