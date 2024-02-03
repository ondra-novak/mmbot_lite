#pragma once
#include <chrono>
#include <vector>
#include "identifier.h"

#include <tuple>
namespace mmbot {


template<typename T>
concept Enumerator = std::is_enum_v<T>;


struct PersistentValue {


    enum Type {
        empty,
        boolean,
        integer,
        uint_id,
        int_id,
        floating,
        datetime,
        identifier
    };

    Type type;
    union {
        bool b_val;
        int i_val;
        double f_val;
        std::uint64_t u_val;
        std::int64_t t_val;
        Identifier ident;
    };

    PersistentValue():type(empty) {}
    PersistentValue(bool b):type(boolean),b_val(b) {}
    PersistentValue(int v):type(integer),i_val(v) {}
    PersistentValue(std::int64_t v):type(int_id),t_val(v) {}
    PersistentValue(std::uint64_t v):type(uint_id),u_val(v) {}
    PersistentValue(double v):type(floating),f_val(v) {}
    template<Enumerator T>
    PersistentValue(T enm):type(integer),i_val(static_cast<int>(enm)) {}
    PersistentValue(std::chrono::system_clock::time_point v):type(datetime),t_val(v.time_since_epoch().count()) {}
    PersistentValue(std::string_view ident):type(identifier),ident(ident) {}

    template<typename Fn>
    auto visit(Fn &&fn) const {
        switch (type) {
            default:
            case empty: return fn(nullptr);
            case boolean: return fn(b_val);
            case integer: return fn(i_val);
            case floating: return fn(f_val);
            case uint_id:  return fn(u_val);
            case int_id:  return fn(t_val);
            case datetime: return fn(std::chrono::system_clock::time_point(
                    std::chrono::system_clock::duration(t_val)));
            case identifier: return fn(ident.to_string());
        }
    }

    template<typename T>
    T as() const {
        return visit([&](auto x) -> T {
            if constexpr(std::is_constructible_v<T, decltype(x)>) {
                return T(x);
            } else {
                return {};
            }
        });
    }
};


class PersistentStorage : public std::vector<PersistentValue> {
public:

    template<Enumerator T>
    PersistentValue &operator[](T v) {
        return std::vector<PersistentValue>::operator [](static_cast<unsigned int>(v));
    }
    template<Enumerator T>
    const PersistentValue &operator[](T v) const {
        return std::vector<PersistentValue>::operator [](static_cast<unsigned int>(v));
    }

    template<typename T, Enumerator ... Args>
    auto as(Args ... items) const {
        return std::make_tuple(std::vector<PersistentValue>::operator [](static_cast<unsigned int>(items)).as<T>()...);
    }

    template<Enumerator T>
    void set_count(T count_value) {
        std::vector<PersistentValue>::resize(static_cast<unsigned int>(count_value));
    }
    template<Enumerator T>
    bool has_count(T count_value) const {
        return std::vector<PersistentValue>::size() >= static_cast<unsigned int>(count_value);
    }

};

///UserStateField is state field, which is presented to the user
/**
 * The strategy can convert values to be more informative for user
 */

struct UserStateField{
    enum Transform {
        ///no transformation
        normal,
        ///enumeration
        enumeration,
        ///value is price (transform can be applied for inverse market)
        price,
        ///value is position (transform can be applied for inverse market)
        position,

    };
    Transform transform;
    PersistentValue value;
    std::string_view name;
    std::string_view values;
};

using UserState = std::vector<UserStateField>;


}
