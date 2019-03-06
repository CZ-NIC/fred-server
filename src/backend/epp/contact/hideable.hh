/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef HIDEABLE_HH_FC41C0BC572FDCD14B5347E83F4DE388//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define HIDEABLE_HH_FC41C0BC572FDCD14B5347E83F4DE388

#include <boost/optional.hpp>

#include <stdexcept>

namespace Epp {
namespace Contact {

enum class PrivacyPolicy
{
    show,
    hide
};

template <typename T>
class Hideable;

template <typename T>
Hideable<T> make_private_data(const T&);

template <typename T>
Hideable<T> make_public_data(const T&);

template <typename T>
Hideable<T> make_data_with_unspecified_privacy(const T&);

template <typename T>
class Hideable
{
public:
    typedef T Type;
    Hideable() = default;
    Hideable(const Hideable&) = default;
    Hideable(Hideable&&) = default;
    ~Hideable() { }
    Hideable& operator=(const Hideable&) = default;
    Hideable& operator=(Hideable&&) = default;
    const Type& operator*()const { return controlled_privacy_->value; }
    const Type* operator->()const { return &(this->operator*()); }
    bool is_public()const { return *(controlled_privacy_->publishability) == PrivacyPolicy::show; }
    bool is_private()const { return *(controlled_privacy_->publishability) == PrivacyPolicy::hide; }
    bool is_publishability_specified()const { return controlled_privacy_->publishability != boost::none; }
    template <typename R>
    Hideable<R> make_with_the_same_privacy(const R& value)const
    {
        if (this->is_publishability_specified())
        {
            if (this->is_public())
            {
                return make_public_data(value);
            }
            if (this->is_private())
            {
                return make_private_data(value);
            }
            throw std::logic_error("data must be public or private");
        }
        return make_data_with_unspecified_privacy(value);
    }
private:
    explicit Hideable(const Type& value)
        : controlled_privacy_(ControlledPrivacyValue(value, boost::none))
    { }
    Hideable(const Type& value, PrivacyPolicy publishability)
        : controlled_privacy_(ControlledPrivacyValue(value, publishability))
    { }
    struct ControlledPrivacyValue
    {
        ControlledPrivacyValue(const Type& _value,
                               const boost::optional<PrivacyPolicy>& _publishability)
            : value(_value),
              publishability(_publishability)
        { }
        Type value;
        boost::optional<PrivacyPolicy> publishability;
    };
    boost::optional<ControlledPrivacyValue> controlled_privacy_;
    template <typename R>
    friend Hideable<R> make_private_data(const R&);
    template <typename R>
    friend Hideable<R> make_public_data(const R&);
    template <typename R>
    friend Hideable<R> make_data_with_unspecified_privacy(const R&);
};

template <typename T>
using HideableOptional = Hideable<boost::optional<T>>;

template <typename T>
Hideable<T> make_private_data(const T& src)
{
    return Hideable<T>(src, PrivacyPolicy::hide);
}

template <typename T>
Hideable<T> make_public_data(const T& src)
{
    return Hideable<T>(src, PrivacyPolicy::show);
}

template <typename T>
Hideable<T> make_data_with_unspecified_privacy(const T& src)
{
    return Hideable<T>(src);
}

}//namespace Epp::Contact
}//namespace Epp

#endif//HIDEABLE_HH_FC41C0BC572FDCD14B5347E83F4DE388
