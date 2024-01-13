#include "market_simulator_read_file.h"

namespace mmbot {

MarketSimulatorReadFile::MarketSimulatorReadFile(const MarketInfo &info,
        std::istream &stream, double equity)
                :AbstractMarketSimulator(info, equity)
                 ,stream(stream)
                 ,tp(std::chrono::system_clock::now()){}


AbstractMarketSimulator::SourceData MarketSimulatorReadFile::fetch_source() {
    double v = 0;
    stream >> v;
    if (v == 0 || stream.eof()) throw EndOfDataException();
    Tick t = nfo.price2tick(v);
    auto ntp = tp;
    tp = tp + std::chrono::minutes(1);
    return SourceData{ntp, t-1, t};
}

}
