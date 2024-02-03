#pragma once
#include "json20/value.h"

#include "factory.h"

namespace mmbot {

template<typename T>
FactoryRepository<T> FactoryRepository<T>::instance;



template<typename T>
inline json20::value_t FactoryRepository<T>::to_json() const {
    return json20::value_t(this->_items.begin(), this->_items.end(), [&](const T *item){
        const CntrForm &form = item->form_desc();
        return json20::key_value_t{
            item->name(),
            json20::value_t(form.begin(), form.end(), [&](const auto &x){
                return x.to_json();
            })
        };
    });

}
template<typename T>
inline const T *FactoryRepository<T>::find(std::string_view name) const {
    auto iter = std::find_if(_items.begin(), _items.end(), [&](const T *v){
        return v->name() == name;
    });
    if (iter == _items.end()) return nullptr;
    else return *iter;
}




}
