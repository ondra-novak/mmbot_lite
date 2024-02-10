#pragma once

#include "json.h"
#include "clone_ptr.h"
#include "formdef.h"


namespace json20 {
    class value_t;
    struct key_value_t;
}

namespace mmbot {


template<typename T>
class IGenericFactory {
public:

    using RetVal = T;

    virtual RetVal create(const json20::value_t &cfg) const = 0;
    virtual const CntrForm &form_desc() const = 0;
    virtual std::string_view name() const = 0;
    virtual ~IGenericFactory() = default;
};


template<typename T>
class FactoryRepository {
public:

    static FactoryRepository instance;
    json20::value_t to_json() const;
    const T *find(std::string_view name) const;

protected:
    std::vector<const T *> _items;

    FactoryRepository() {}

};


template<typename Fn>
class GenericFactoryImpl: public IGenericFactory<std::decay_t<std::invoke_result_t<Fn, JsonValue> > > {
public:

    using RetVal = typename IGenericFactory<std::decay_t<std::invoke_result_t<Fn, json20::value_t> > >::RetVal;

    GenericFactoryImpl(std::string_view name, CntrForm form_def, Fn fn)
        :_name(name)
        ,_form_def(std::move(form_def))
        ,_fn(fn) {}

    virtual RetVal create(const json20::value_t &cfg) const {
        return _fn(cfg);
    }
    virtual const CntrForm &form_desc() const {
        return _form_def;

    }
    virtual std::string_view name() const {
        return _name;

    }
protected:
    std::string_view _name;
    CntrForm _form_def;
    Fn _fn;
};

template<typename Fn>
GenericFactoryImpl(std::string_view, json20::value_t, Fn) -> GenericFactoryImpl<Fn>;



}
