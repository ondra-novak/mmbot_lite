#pragma once

#include <memory>
#include <optional>

#include "clone_ptr.h"

namespace mmbot {

class ISpreadState {
public:

	virtual ~ISpreadState() {}
	virtual ISpreadState *clone() const = 0;

};


class ISpreadFunction {
public:

	struct Result {
		bool valid = false;
		double spread = 0;
		double center = 0;
		int trend = 0;
	};

	virtual clone_ptr<ISpreadState> start() const = 0;
	virtual ISpreadFunction *clone() const = 0;
	virtual Result point(std::unique_ptr<ISpreadState> &state, double y) const = 0;
	virtual ~ISpreadFunction() {}
};


std::unique_ptr<ISpreadFunction> defaultSpreadFunction(
		double sma,
		double stdev,
		double force_spread);
std::unique_ptr<ISpreadFunction> defaultSpreadFunction_direct(
        unsigned int sma,
        unsigned int stdev,
        double force_spread);

struct SpreadStats {
    double spread;
    double mult_buy;
    double mult_sell;
};

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

    virtual SpreadStats get_stats(PState &state, double equilibrium) const = 0;
};



}


