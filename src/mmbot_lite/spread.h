#pragma once

#include <memory>
#include <optional>

#include "clone_ptr.h"

namespace mmbot {


class ISpreadGen {
public:

    struct Result {
        std::optional<double> buy;
        std::optional<double> sell;
    };

    class State {
    public:
        virtual ~State() = default;
        virtual State *clone() const = 0;
    };

    using PState = clone_ptr<State>;

    virtual ~ISpreadGen() = default;
    ///Retrieve current result
    virtual Result get_result(const PState &state, double equilibrium) const = 0;
    ///Record new point
    /**
     * @param y new point value
     * @param execution set true, if execution has been reported
     * @return new state of the generator
     */
    virtual void point(PState &state, double y, bool execution) const = 0;
    ///Retrieve how much history this generator needs
    virtual unsigned int get_required_history_length() const  = 0;

    virtual PState start() const = 0;

    virtual ISpreadGen *clone() const = 0;

};


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


