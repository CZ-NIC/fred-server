#include "src/bin/corba/util/corba_conversions_nullable_types.hh"


namespace CorbaConversion {
namespace Util {

Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<long long>& in)
{

    if (in.isnull())
    {
        return NULL;
    }

    long long temp = in.get_value();
    if (temp < 0)
    {
        throw std::out_of_range("should be >= 0");
    }
    return new Registry::NullableULongLong(temp);
}


Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<unsigned long long>& in)
{

    if (in.isnull())
    {
        return NULL;
    }

    return new Registry::NullableULongLong(in.get_value());
}


Nullable<unsigned long long> unwrap_nullable_ulonglong(const Registry::NullableULongLong* in)
{

    if (in == NULL)
    {
        return Nullable<unsigned long long>();
    }

    // else
    return Nullable<unsigned long long>(static_cast<unsigned long long>(in->_value()));
}


Optional<unsigned long long> unwrap_nullable_ulonglong_to_optional(const Registry::NullableULongLong* in)
{
    if (in == NULL)
    {
        return Optional<unsigned long long>();
    }

    // else
    return Optional<unsigned long long>(static_cast<unsigned long long>(in->_value()));
}


Nullable<std::string> unwrap_nullable_string(const Registry::NullableString* in)
{

    if (in == NULL)
    {
        return Nullable<std::string>();

    }

    // else
    return Nullable<std::string>(
            std::string(
                    static_cast<const char*>(in->_value()))
            );
}


Optional<std::string> unwrap_nullable_string_to_optional(const Registry::NullableString* in)
{

    if (in == NULL)
    {
        return Optional<std::string>();
    }

    // else
    return Optional<std::string>(
            std::string(
                    static_cast<const char*>(in->_value()))
            );
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
