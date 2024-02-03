#pragma once

#include "market.h"

#include <utility>
#include <variant>
namespace mmbot {


struct MarketStructure;
struct WalletInfo;
class IBroker;


class IMarketAccount {
public:


    virtual ~IMarketAccount() = default;
    ///Creates market instance
    virtual std::unique_ptr<IMarket> create_market(std::string_view symbol) const = 0;

    ///Retrieve market structure for selecting symbol
    virtual MarketStructure get_market_structure() const = 0;

    ///Get symbol information
    virtual MarketInfo get_info(std::string_view symbol) const = 0;

    virtual WalletInfo get_wallet() const = 0;

    virtual const IBroker &get_broker() const = 0;


};



struct MarketStructure {

    using Symbol = std::string;
    using Category = std::vector<MarketStructure>;

    std::string title;
    std::variant<Symbol, Category> symbol_or_category;
};


struct WalletInfo {

    struct Asset {
        std::string name;
        double balance;
        double locked;
    };

    struct Wallet {
        std::string name;
        std::vector<Asset> assets;
    };

    std::vector<Wallet> wallets;
};

}
