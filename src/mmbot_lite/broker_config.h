#pragma once

#include "market_account.h"
#include "types/clone_ptr.h"
#include "types/factory.h"

#include <memory>
namespace mmbot {

struct BrokerInfo {
    std::string name;
    std::string description;
    std::string url;
    std::string icon_png;
};

class IBroker: public IGenericFactory<std::shared_ptr<IMarketAccount> > {
public:

    virtual BrokerInfo get_info() const;

};


using BrokerRepository = FactoryRepository<IBroker>;


}
