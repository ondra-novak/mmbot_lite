#pragma once
#include "types/json_document.h"
#include "market.h"
#include "istorage.h"

#include "dbtypes.h"

#include <docdb/database.h>
#include <docdb/map.h>



namespace mmbot {


static_assert(docdb::IsSerializableMethod<Fill , docdb::ToBinarySerializer<char *> >);

class DatabaseCntr {
public:

    using Fill = IStorage::Fill;
    using Fills = std::vector<Fill>;



    DatabaseCntr(docdb::PDatabase db);


    std::vector<IStorage::Ticker> read_history(
            MarketID market,
            SymbolID symbol,
            MinuteID start,
            std::size_t count_to_history) const;
    void store_ticker(MarketID market,
            SymbolID symbol,
            const IStorage::Ticker &ticker);

    void store_fill(TraderID id, const Fill &fill);

    Fills read_fills(TraderID id) const;

    void store_trader_state(TraderID id, const JsonValue &state) ;
    JsonValue restore_trader_state(TraderID id) const ;


    std::unique_ptr<IStorage> create_storage(TraderID id, MarketID market, SymbolID symbol);

protected:

    docdb::PDatabase _db;
    docdb::Map<docdb::RowDocument> _tickers;
    docdb::Map<docdb::SerializingDocument<Fill> > _fills;
    docdb::Map<JsonDocument> _trader_state;


};





}
