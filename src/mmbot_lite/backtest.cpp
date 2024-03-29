#include "market_simulator_read_file.h"
#include "trader.h"

#include "reporting_csv.h"

#include "bollinger_spread.h"
#include "broker_config.h"

#include "StrategyDCAM.h"
#include "math/random.h"

#include "strategy_config.h"
#include <fstream>
#include <json20/serializer.h>



int main() {
    using namespace mmbot;

    auto s = BrokerRepository::instance.to_json().to_json();


    std::cout << s << std::endl;

    MarketInfo nfo{
       1,
       0.01,
       0.00001,
       100,
       0.00001,
       0.001,
       1    };

    std::fstream f("backtest.csv");
    auto market = std::make_unique<MarketSimulatorReadFile>(nfo, f, 10000);
    auto report = std::make_unique<ReportingCSV>("report.csv");
    Trader::Config cfg;
    cfg.spread = Trader::PSpread(new BBSpread(BBSpread::Config::fromHours(10,10, {1,3,5})));
    cfg.strategy = Trader::PStrategy(new StrategyDCAM_SinH({15,1,10000,2,1,10}));
    try {

        Trader trd(cfg, std::move(market), nullptr, std::move(report));
        trd.start();
        do {
            trd.step();
        } while(true);


    } catch (const MarketSimulatorReadFile::EndOfDataException &) {
        //empty
    }

}
