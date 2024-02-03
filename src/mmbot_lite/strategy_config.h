#pragma once

#include "types/factory.h"

namespace mmbot {

class IStrategy;
class ISpread;
class IMarketAccount;

using IStrategyFactory = IGenericFactory<clone_ptr<IStrategy> >;
using ISpreadFactory = IGenericFactory<clone_ptr<ISpread> >;

using StrategyRepository = FactoryRepository<IStrategyFactory>;
using SpreadRepository = FactoryRepository<ISpreadFactory>;


}

