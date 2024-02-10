#pragma once

#include "json.h"
#include <json20/serializer.h>
#include <json20/parser.h>

namespace mmbot {

class JsonDocument {
public:
    using Type = JsonValue;

    template<typename Iter>
    static Iter to_binary(const JsonValue &val, Iter output) {
        json20::serializer_t<json20::format_t::binary> srl(val);
        std::string_view x = srl.read();
        while (!x.empty()) {
            output = std::copy(x.begin(), x.end(), output);
            x = srl.read();
        }
        return output;
    }

    template<typename Iter>
    static JsonValue from_binary(Iter &at, Iter end) {
        std::string_view buff(at,end);
        json20::parser_t<json20::format_t::binary> prs;
        prs.write(buff);
        at += buff.size() - prs.get_unused_text().size();
        return prs.get_parsed();
    }


};



}
