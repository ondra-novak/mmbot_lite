cmake_minimum_required(VERSION 3.1)

add_executable(mmbot_lite
     main.cpp
     market_simulator.cpp
     market_simulator_read_file.cpp
     bollinger_spread.cpp
     StrategyDCAM.cpp
     trader.cpp
)
