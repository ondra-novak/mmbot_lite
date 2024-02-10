#include "database.h"
#include <docdb/timepoint.h>

namespace mmbot {

DatabaseCntr::DatabaseCntr(docdb::PDatabase db)
        :_db(db)
        ,_tickers(db, "tickers")
        ,_fills(db, "fills")
        ,_trader_state(db, "trader_state")

        {
}

std::vector<IStorage::Ticker> DatabaseCntr::read_history(MarketID market,
        SymbolID symbol, MinuteID start, std::size_t count_to_history) const {

    std::vector<IStorage::Ticker> r;
    auto rc = _tickers.select_from({market, symbol, start}, docdb::Direction::backward);
    auto iter = rc.begin();
    auto end = rc.end();
    for (std::size_t i = 0; iter != end && i < count_to_history; ++i, ++iter) {
        auto k = iter->key.get<TickerKey>();
        auto v = iter->value.get<TickerValue>();
        r.push_back({
            Time(std::chrono::duration_cast<Time::duration>(std::chrono::minutes(std::get<MinuteID>(k)))),
            std::get<0>(v),
            std::get<1>(v)
        });

    }
    std::reverse(r.begin(),r.end());
    return r;

}

void DatabaseCntr::store_ticker(MarketID market, SymbolID symbol,
        const IStorage::Ticker &ticker) {

    MinuteID minid = std::chrono::duration_cast<std::chrono::minutes>(ticker.tm.time_since_epoch()).count();

    _tickers.put(TickerKey(market, symbol, minid), TickerValue(ticker.bid, ticker.ask));

}

void DatabaseCntr::store_fill(TraderID id, const Fill &fill) {
    _fills.put(FillKey(id, fill.tm), fill);
}

DatabaseCntr::Fills DatabaseCntr::read_fills(TraderID id) const {
    Fills out;
    for (const auto &rc: _fills.select({id})) {
        out.push_back(rc.value);
    }
    return out;
}

JsonValue DatabaseCntr::restore_trader_state(TraderID id) const {
    auto res = _trader_state.find({id});
    if (res.has_value()) {
        return *res;
    } else {
        return {};
    }
}
void DatabaseCntr::store_trader_state(TraderID id, const JsonValue &state) {
    _trader_state.put({id}, state);
}


class Storage : public IStorage{
public:
    DatabaseCntr &cntr;
    TraderID id;
    std::string market;
    std::string symbol;

    virtual void store_fill(const Fill &fill) override;
    virtual std::vector<IStorage::Ticker,
            std::allocator<IStorage::Ticker> > read_history(
            std::size_t count) const override;
    virtual Fills read_fills() const override;
    virtual void store_ticker(const IStorage::Ticker &ticker) override;
    virtual void store_trader_state(const JsonValue &state)  override;
    virtual JsonValue restore_trader_state() const override;

    Storage(DatabaseCntr &cntr, TraderID id, MarketID market, SymbolID symbol)
        :cntr(cntr),id(id),market(market),symbol(symbol) {}

};

std::unique_ptr<IStorage> DatabaseCntr::create_storage(TraderID id, MarketID market, SymbolID symbol) {
    return std::make_unique<Storage>(*this, id, market, symbol);
}

void Storage::store_fill(const Fill &fill) {
    cntr.store_fill(id, fill);
}

std::vector<IStorage::Ticker,
        std::allocator<IStorage::Ticker> > Storage::read_history(
        std::size_t count) const {
    auto now = std::chrono::system_clock::now();
    auto start = std::chrono::duration_cast<std::chrono::minutes>(now.time_since_epoch()).count();
    return cntr.read_history(market, symbol, start, count);
}

Storage::Fills Storage::read_fills() const {
    return cntr.read_fills(id);
}

void Storage::store_ticker(
        const IStorage::Ticker &ticker) {
    cntr.store_ticker(market, symbol, ticker);
}

void Storage::store_trader_state(const JsonValue &state) {
    cntr.store_trader_state(id, state);
}
JsonValue Storage::restore_trader_state() const {
    return cntr.restore_trader_state(id);
}

}
