#pragma once
#include <docdb/database.h>
#include <docdb/map.h>


#include "market.h"
#include "istorage.h"

#include "dbtypes.h"
namespace mmbot {


static_assert(docdb::IsSerializableMethod<Fill , docdb::ToBinarySerializer<char *> >);

class DatabaseCntr {
public:





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

    void store_strategy_state(TraderID id, const PersistentStorage &store);

    PersistentStorage read_strategy_state(TraderID id);

    std::unique_ptr<IStorage> create_storage(TraderID id, MarketID market, SymbolID symbol);

protected:

    docdb::PDatabase _db;
    docdb::Map<docdb::RowDocument> _tickers;
    docdb::Map<docdb::SerializingDocument<Fill> > _fills;
    docdb::Map<docdb::SerializingDocument<PersistentStorage> > _strategy_state;


};





}
