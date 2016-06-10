#include "src/epp/parameter_errors.h"

#include <stdexcept>

namespace Epp {

namespace {

ParameterErrors::Where scalar() { return ParameterErrors::Where(); }
ParameterErrors::Where vector(unsigned short _index)
{
    ParameterErrors::Where where;
    where.indexes.insert(_index);
    return where;
}

}//namespace Epp::{anonymous}

ParameterErrors& ParameterErrors::add_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)
{
    const What what(_param, _reason);
    const WhatWhere::const_iterator what_where_ptr = what_where_.find(what);
    if (what_where_ptr != what_where_.end()) {
        throw std::runtime_error(what_where_ptr->second.is_scalar() ? "scalar parameter error already exists"
                                                                    : "vector parameter error already exists");
    }
    what_where_.insert(std::make_pair(what, scalar()));
    return *this;
}

ParameterErrors& ParameterErrors::add_vector_parameter_error(
    Param::Enum _param,
    unsigned short _index,
    Reason::Enum _reason)
{
    const What what(_param, _reason);
    const WhatWhere::iterator what_where_ptr = what_where_.find(what);
    if (what_where_ptr != what_where_.end()) {
        if (what_where_ptr->second.is_scalar()) {
            throw std::runtime_error("scalar parameter error already exists");
        }
        what_where_ptr->second.add_element(_index);
        return *this;
    }
    what_where_.insert(std::make_pair(what, vector(_index)));
    return *this;
}

bool ParameterErrors::is_empty()const
{
    return what_where_.empty();
}

bool ParameterErrors::has_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)const
{
    const What what(_param, _reason);
    const WhatWhere::const_iterator what_where_ptr = what_where_.find(what);
    if (what_where_ptr == what_where_.end()) {
        return false;
    }
    return what_where_ptr->second.is_scalar();
}

bool ParameterErrors::has_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const
{
    const What what(_param, _reason);
    const WhatWhere::const_iterator what_where_ptr = what_where_.find(what);
    if (what_where_ptr == what_where_.end()) {
        return false;
    }
    return what_where_ptr->second.is_vector();
}

bool ParameterErrors::has_vector_parameter_error_at(
    Param::Enum _param,
    unsigned short _index,
    Reason::Enum _reason)const
{
    const What what(_param, _reason);
    const WhatWhere::const_iterator what_where_ptr = what_where_.find(what);
    if (what_where_ptr == what_where_.end()) {
        return false;
    }
    return what_where_ptr->second.has_element(_index);
}

const ParameterErrors::Where& ParameterErrors::get_vector_parameter_error(
    Param::Enum _param,
    Reason::Enum _reason)const
{
    const What what(_param, _reason);
    const WhatWhere::const_iterator what_where_ptr = what_where_.find(what);
    if ((what_where_ptr != what_where_.end()) &&
        what_where_ptr->second.is_vector())
    {
        return what_where_ptr->second;
    }
    throw std::runtime_error("vector parameter error not found");
}

ParameterErrors::What::What(Param::Enum _param, Reason::Enum _reason)
:   param(_param),
    reason(_reason)
{ }

bool ParameterErrors::What::operator<(const What &_b)const
{
    const What &_a = *this;
    return  (_a.param <  _b.param) ||
           ((_a.param == _b.param) && (_a.reason < _b.reason));
}

ParameterErrors::Where& ParameterErrors::Where::add_element(unsigned short _index)
{
    if (indexes.insert(_index).second) {
        return *this;
    }
    throw std::runtime_error("element of vector parameter error already exists at the same position");
}

}//namespace Epp
