#pragma once

///Calculates UPnL and RPnL based on trades
/**
 * Object is immutable. Every trade generates new instance
 */
class ACB {
public:


    ACB() = default;

	///Initialize object
	/**
	 * @param init_price initial price where init_pos has been opened. If the init_pos is zero,
	 *  this value is ignored
	 * @param init_pos initial position
	 * @param init_rpnl initializes RPnL
	 */
	ACB(double init_price, double init_pos, double init_rpnl = 0)
		:suma(init_pos?init_price * init_pos:0)
		,pos(init_pos)
		,rpnl(init_rpnl)
	{}

	///Record execution at price and size. The result is returned as new state
	/**
	 * @param price price of the trade
	 * @param size size of the trade
	 * @return new state
	 *
	 * Fee must be substracted. Effective price and size must be passed
	 */
	ACB execution(double price, double size, double fees = 0) const {
		ACB res;
		if (pos * size >= 0) {
			res.suma = suma + size * price;;
			res.pos = pos + size;
			res.rpnl = rpnl - fees;
		} else if (pos * (pos + size) < 0) {
			double avg = suma/pos;
			res.rpnl = rpnl + (price - avg) * pos - fees;
			size += pos;
			res.pos = 0;
			res.suma = 0;
			return res.execution(price,size);
		} else {
			if (std::abs(pos + size) < (std::abs(pos)+std::abs(size))*1e-10) {
				size = -pos; //solve rounding errors
			}
			double avg = suma/pos;
			res.rpnl = rpnl - (price - avg) * size - fees;
			res.pos = pos + size;
			res.suma = avg * res.pos;
		}
		return res;
	}

	ACB resetRPnL() const {
		ACB res;
		res.suma = suma;
		res.pos = pos;
		res.rpnl = 0;
		return res;
	}

	///Retrieve
	double getOpen() const {return suma/pos;}
	double getPos() const {return pos;}
	double getSuma() const {return suma;}
	double getRPnL() const {return rpnl;}
	double getUPnL(double price) const {
		return price*pos - suma;
	}
	double getEquity(double price) const {return getRPnL()+getUPnL(price);}

protected:

	double suma = 0;
	double pos = 0;
	double rpnl = 0;


};




