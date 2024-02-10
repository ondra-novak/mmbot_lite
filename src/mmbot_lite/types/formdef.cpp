#include "formdef.h"

#include <json20/value.h>

namespace mmbot {

json20::value_t CntrNumber::to_json() const {
    return {
        {"type","number"},
        {"default",defval},
        {"step",step}
    };
}

json20::value_t CntrSlider::to_json() const {
    return {
        {"type","slider"},
        {"default",defval},
        {"min",min},
        {"max",max},
        {"step",step},
        {"decimals",decimals}
    };
}

json20::value_t CntrLogSlider::to_json() const {
    return {
        {"type","log_slider"},
        {"default",defval},
        {"min",min},
        {"max",max},
        {"step",step},
        {"decimals",decimals}
    };
}

json20::value_t CntrBool::to_json() const {
    return {
        {"type","boolean"},
        {"default",defval},
    };
}

json20::value_t CntrEnum::to_json() const {
    return {
        {"type","enum"},
        {"defval",defval},
        {"options",json20::value_t(options.begin(), options.end(), [](const auto &x)->json20::value_t{return json20::value_t(x);})},
    };
}

json20::value_t CntrDef::to_json() const {
    json20::value_t v = std::visit([&](const auto &x){
        return x.to_json();
    }, def);
    v.set({
        {"name", name},
        {"show_if", show_if.empty()?json20::value_t()
                :json20::value_t(show_if.begin(), show_if.end(), [](const auto &d)->json20::key_value_t{
            return {json20::value_t(d.first), json20::value_t(d.second.begin(),d.second.end(), [](const auto &e){
                return json20::value_t(e);
            })};
        })}
    });
    return v;
}

json20::key_value_t CntrFormSection::to_json() const {
    return {json20::value_t(name), json20::value_t(defs.begin(), defs.end(), [&](const auto &x){
        return x.to_json();
     })
    };
}


}
