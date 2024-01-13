
#include "market.h"

#include <iostream>
#include <sstream>


int main() {
    double a = 0;
    double b = 0;
    double c = 0;
    long d = 0;

    std::string s = "1 1.2  212.32 123";
    std::istringstream ss(s);
    ss >> a >> b >> c  >> d;

}
