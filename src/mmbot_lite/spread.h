#pragma once

#include <memory>
#include <optional>

#include "types/clone_ptr.h"


namespace mmbot {



class ISpread {
public:

    enum class Flag {
        ///normal price
        normal,
        ///middle price - (we are at middle of pattern)
        middle,
        ///edge price - (we are at edge of pattern)
        edge
    };

    struct Result {
        std::optional<double> buy;
        std::optional<double> sell;
        Flag buy_flag;
        Flag sell_flag;

    };

    ///Retrieve current result
    virtual Result get_result(double equilibrium) const = 0;
    ///Put starting point
    /**
     * It resets state and puts starting point
     * @param y point value
     */
    virtual void start(double y) = 0;
    ///Add normal point
    virtual void point(double y) = 0;
    ///Add execution point
    virtual void execution(double y) = 0;
    ///Retrieve minimal count of points to return usable result
    virtual unsigned int get_min_point_count() const = 0;

    virtual ISpread *clone() const = 0;

    virtual ~ISpread() = default;
};



}


