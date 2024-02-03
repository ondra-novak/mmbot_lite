#pragma once

#include "market.h"

#include "strategy.h"
#include "istorage.h"
#include <docdb/serializing_document.h>
namespace mmbot {


using MarketID = std::string_view;
using SymbolID = std::string_view;
using MinuteID = std::uint32_t;
using TickerKey = docdb::FixedKey<MarketID, SymbolID, MinuteID>;
using TickerValue = docdb::FixedRow<double, double>;

using FillKey = docdb::FixedKey<TraderID, Time>;



struct DBRecord {

    enum Type {
        strategy_state,
        fill
    };


    TraderID trader;
    Type type;

    union {
        PersistentStorage _strategy_state;
        Fill _fill;
    };



};



}
