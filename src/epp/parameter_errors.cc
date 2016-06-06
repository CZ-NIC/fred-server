#include "src/epp/parameter_errors.h"

#include <stdexcept>

namespace Epp {

ParameterErrors& ParameterErrors::add_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)
{
    ParamReasons::iterator param_reasons_ptr = param_reasons_.find(_param);
    Reasons &reasons = param_reasons_ptr == param_reasons_.end()
        ? param_reasons_[_param]
        : param_reasons_ptr->second;

    reasons.add_scalar_parameter_reason(_reason);
    return *this;
}

ParameterErrors& ParameterErrors::add_vector_parameter_error(
    Param::Enum _param,
    unsigned short _index,
    Reason::Enum _reason)
{
    ParamReasons::iterator param_reasons_ptr = param_reasons_.find(_param);
    Reasons &reasons = param_reasons_ptr == param_reasons_.end()
        ? param_reasons_[_param]
        : param_reasons_ptr->second;

    reasons.add_vector_parameter_reason(_reason, _index);
    return *this;
}

bool ParameterErrors::is_empty()const
{
    return param_reasons_.empty();
}

bool ParameterErrors::has_scalar_parameter_error(Param::Enum _param, Reason::Enum _reason)const
{
    const ParamReasons::const_iterator param_reasons_ptr = param_reasons_.find(_param);
    if (param_reasons_ptr == param_reasons_.end()) {
        return false;
    }
    return param_reasons_ptr->second.has_scalar_parameter_reason(_reason);
}

bool ParameterErrors::has_vector_parameter_error(Param::Enum _param, Reason::Enum _reason)const
{
    const ParamReasons::const_iterator param_reasons_ptr = param_reasons_.find(_param);
    if (param_reasons_ptr == param_reasons_.end()) {
        return false;
    }
    return param_reasons_ptr->second.has_vector_parameter_reason(_reason);
}

const ParameterErrors::Indexes& ParameterErrors::get_vector_parameter_error(
    Param::Enum _param,
    Reason::Enum _reason)const
{
    const ParamReasons::const_iterator param_reasons_ptr = param_reasons_.find(_param);
    if (param_reasons_ptr != param_reasons_.end()) {
        return param_reasons_ptr->second.get_vector_parameter_reason(_reason);
    }
    throw std::runtime_error("vector parameter error not found");
}

bool ParameterErrors::Reasons::has_scalar_parameter_reason(Reason::Enum _reason)const
{
    const ReasonAtPositions::const_iterator positions_ptr = reason_at_positions_.find(_reason);
    if (positions_ptr == reason_at_positions_.end()) {
        return false;
    }
    if (positions_ptr->second.empty()) {
        return true;
    }
    throw std::runtime_error("vector parameter error found");
}

bool ParameterErrors::Reasons::has_vector_parameter_reason(Reason::Enum _reason)const
{
    const ReasonAtPositions::const_iterator positions_ptr = reason_at_positions_.find(_reason);
    if (positions_ptr == reason_at_positions_.end()) {
        return false;
    }
    if (!positions_ptr->second.empty()) {
        return true;
    }
    throw std::runtime_error("scalar parameter error found");
}

const ParameterErrors::Indexes& ParameterErrors::Reasons::get_vector_parameter_reason(Reason::Enum _reason)const
{
    const ReasonAtPositions::const_iterator positions_ptr = reason_at_positions_.find(_reason);
    if (positions_ptr == reason_at_positions_.end()) {
        throw std::runtime_error("vector parameter error not found");
    }
    if (!positions_ptr->second.empty()) {
        return positions_ptr->second;
    }
    throw std::runtime_error("scalar parameter error found");
}

ParameterErrors::Reasons& ParameterErrors::Reasons::add_scalar_parameter_reason(Reason::Enum _reason)
{
    const ReasonAtPositions::const_iterator positions_ptr = reason_at_positions_.find(_reason);
    if (positions_ptr == reason_at_positions_.end()) {
        reason_at_positions_.insert(std::make_pair(_reason, Indexes()));
        return *this;
    }
    throw std::runtime_error(positions_ptr->second.empty() ? "scalar parameter error already exists"
                                                           : "vector parameter error already exists");
}

ParameterErrors::Reasons& ParameterErrors::Reasons::add_vector_parameter_reason(
    Reason::Enum _reason,
    short unsigned _index)
{
    ReasonAtPositions::iterator positions_ptr = reason_at_positions_.find(_reason);
    if (positions_ptr == reason_at_positions_.end()) {
        Indexes indexes;
        indexes.insert(_index);
        reason_at_positions_.insert(std::make_pair(_reason, indexes));
        return *this;
    }
    if (positions_ptr->second.empty()) {
        throw std::runtime_error("scalar parameter error already exists");
    }
    if (positions_ptr->second.insert(_index).second) {
        return *this;
    }
    throw std::runtime_error("vector parameter error already exists at the same position");
}

}//namespace Epp
