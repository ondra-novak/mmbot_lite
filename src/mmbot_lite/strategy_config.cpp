#include "strategy_config.h"
#include "types/factory_impl.h"

#include "bollinger_spread.h"

#include "StrategyDCAM.h"
namespace mmbot {



GenericFactoryImpl bbspread("bbspread",{
        {"",{
                {"mean",CntrSlider{15,1,100,0.1,1}},
                {"levels",CntrSlider{3,1,5}},
                {"level0",CntrSlider{1,0.1,10,0.1,1}},
                {"level1",CntrSlider{3,0.1,10,0.1,1},{{"levels",{2,3,4,5}}}},
                {"level2",CntrSlider{5,0.1,10,0.1,1},{{"levels",{3,4,5}}}},
                {"level3",CntrSlider{7,0.1,10,0.1,1},{{"levels",{4,5}}}},
                {"level4",CntrSlider{10,0.1,10,0.1,1},{{"levels",{5}}}},
        }},
        {"advanced",{
                {"dev",CntrLogSlider{100,10,1000,0.01,1}}
        }},
        {"zero_level",{
            {"zero_level",CntrBool{true}}
        }}
},[](const json20::value_t &data) {
    std::vector<double> lvls;
    double mean = data["mean"].get();
    double dev = data["dev"].get();
    bool zero = data["zero_level"].get();
    unsigned int levels = data["levels"].get();
    for (unsigned int i = 0; i < levels; ++i) {
        std::string lvln = "level"+std::to_string(i+1);
        lvls.push_back(data[lvln].get());
    }
    dev = dev * 0.01 * mean;
    return clone_ptr<ISpread>(
            new BBSpread(BBSpread::Config::fromHours(mean, dev, lvls, zero)));

});

static CntrForm dcam_form{
    {"",{
            {"initial_budget",CntrNumber{std::numeric_limits<double>::signaling_NaN()}},
            {"power",CntrLogSlider{25,0.1,1000,0.01,1}},
            {"multiplier",CntrLogSlider{1,0.01,100,0.01,2}},
            {"initial_yield",CntrSlider{1,0,20,0.1,2}},
            {"yield",CntrSlider{1,0,20,0.1,2}},
            {"max_leverage",CntrLogSlider{10,1,1000,0.01,0}},
    }}
};

template<typename T>
static typename StrategyDCAM<T>::Config init_dcam_config(const json20::value_t &data) {
    typename StrategyDCAM<T>::Config cfg;
    cfg.power = data["power"].get();
    cfg.multiplier = data["multiplier"].get();
    cfg.initial_budget = data["initial_budget"].get();
    cfg.initial_yield_mult = data["initial_yield"].get();
    cfg.yield_mult= data["yield"].get();
    cfg.max_leverage = data["max_leverage"].get();
    return cfg;
}


GenericFactoryImpl s_dcam_sinh("dcam_sinh",dcam_form,[](const json20::value_t &data){
    return clone_ptr<IStrategy>(new StrategyDCAM_SinH(init_dcam_config<FunctionSinH>(data)));
});
GenericFactoryImpl s_dcam_powern("dcam_powern",dcam_form,[](const json20::value_t &data){
    return clone_ptr<IStrategy>(new StrategyDCAM_PowerN(init_dcam_config<FunctionPowerN>(data)));
});


template<>
FactoryRepository<ISpreadFactory>::FactoryRepository()
        :_items{&bbspread} {}

template<>
FactoryRepository<IStrategyFactory>::FactoryRepository()
        :_items{&s_dcam_sinh, &s_dcam_powern} {}



template class FactoryRepository<ISpreadFactory>;
template class FactoryRepository<IStrategyFactory>;


}
