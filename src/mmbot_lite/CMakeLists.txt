cmake_minimum_required(VERSION 3.1)


add_executable(backtest
     backtest.cpp
     market_simulator.cpp
     market_simulator_read_file.cpp
     bollinger_spread.cpp
     StrategyDCAM.cpp
     trader.cpp
     reporting_csv.cpp
     database.cpp
     strategy_config.cpp
     types/formdef.cpp
     broker_config.cpp
)
    
target_link_libraries(backtest leveldb)