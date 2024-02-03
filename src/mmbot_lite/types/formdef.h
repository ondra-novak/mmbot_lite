#pragma once

#include "persistent_storage.h"
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace json20 {
    class value_t;
    struct key_value_t;
}

namespace mmbot {

struct CntrNumber {
    double defval = 0;
    double step = 0;

    json20::value_t to_json() const;
};

struct CntrSlider {
    double defval = 0;
    double min = 0;
    double max = 10;
    double step = 1;
    unsigned int decimals = 0;

    json20::value_t to_json() const;
};

struct CntrLogSlider {
    double defval = 0;
    double min = 0;
    double max = 10;
    double step = 1;
    unsigned int decimals = 0;

    json20::value_t to_json() const;
};

struct CntrBool {
    bool defval;

    json20::value_t to_json() const;
};

struct CntrEnum {
    unsigned int defval;
    std::vector<std::string> options;

    json20::value_t to_json() const;
};

using ShowIfCond = std::vector<std::pair<std::string, std::vector<PersistentValue> > >;

struct CntrDef {
    std::string_view name;
    std::variant<CntrNumber, CntrSlider, CntrLogSlider, CntrBool, CntrEnum> def;
    ShowIfCond show_if = {};

    json20::value_t to_json() const;
};

struct CntrFormSection {
    std::string_view name;
    std::vector<CntrDef> defs;

    json20::key_value_t to_json() const;
};

using CntrForm = std::vector<CntrFormSection>;



}
