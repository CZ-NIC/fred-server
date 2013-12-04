#include "corba/util/corba_conversions_nullable_types.h"

namespace Corba {
    Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<long long>& in) {

        if(in.isnull() == false) {
            long long temp;
            temp = static_cast<long long>(in);
            if(temp < 0) {
                throw std::out_of_range("should be >= 0");
            }
            return new Registry::NullableULongLong(temp);
        }

        return new Registry::NullableULongLong;
    }

    Registry::NullableULongLong* wrap_nullable_ulonglong(const Nullable<unsigned long long>& in) {

        if(in.isnull() == false) {
            return new Registry::NullableULongLong(
                static_cast<unsigned long long>(in));
        }

        return new Registry::NullableULongLong;
    }

    Nullable<unsigned long long> unwrap_nullable_ulonglong(const Registry::NullableULongLong * in) {

        if ( in != NULL ) {
            return Nullable<unsigned long long>(static_cast<unsigned long long>(in->_value()));
        }

        // else
        return Nullable<unsigned long long>();
    }
}
