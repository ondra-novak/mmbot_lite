
#include "market.h"

#include <iostream>
#include <sstream>

#include "numerics.h"

#include <cmath>
int main(int argc, char **argv) {


    double z = mmbot::Numerics<>::find_root_to_inf(argc,[&](double x){
        return 1e-85*std::cosh(200*(1-x))-0.01;
    });

    std::cout << z << std::endl;
}
