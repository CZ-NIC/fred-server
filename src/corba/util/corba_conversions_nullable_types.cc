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

    Registry::NullableDate* wrap_nullable_date(const Nullable<boost::gregorian::date>& in) {
        if (in.isnull()) {
            return NULL;
        }
        else {
            const boost::gregorian::date& in_value = in;

            if (in_value.is_special()) {
                return NULL;
            } else {
                Registry::Date result_data;
                result_data.year  = static_cast<int>(in_value.year());
                result_data.month = static_cast<int>(in_value.month());
                result_data.day   = static_cast<int>(in_value.day());

                return new Registry::NullableDate(result_data);
            }
        }
    }

    Registry::NullableDateTime* wrap_nullable_datetime(const Nullable<boost::posix_time::ptime>& in) {
        if (in.isnull()) {
            return NULL;
        }
        else {
            const boost::posix_time::ptime& in_value = in;

            if (in_value.is_special()) {
                return NULL;
            } else {
                Registry::DateTime result_data;
                result_data.date.year     = static_cast<int>(in_value.date().year());
                result_data.date.month   = static_cast<int>(in_value.date().month());
                result_data.date.day     = static_cast<int>(in_value.date().day());
                result_data.hour    = static_cast<int>(in_value.time_of_day().hours());
                result_data.minute  = static_cast<int>(in_value.time_of_day().minutes());
                result_data.second  = static_cast<int>(in_value.time_of_day().seconds());

                return new Registry::NullableDateTime(result_data);
            }
        }
    }

    Nullable<unsigned long long> unwrap_nullable_ulonglong(const Registry::NullableULongLong * in) {

        if ( in != NULL ) {
            return Nullable<unsigned long long>(static_cast<unsigned long long>(in->_value()));
        }

        // else
        return Nullable<unsigned long long>();
    }

    Optional<unsigned long long> unwrap_nullable_ulonglong_to_optional(const Registry::NullableULongLong * in) {
        if ( in != NULL ) {
            return Optional<unsigned long long>(static_cast<unsigned long long>(in->_value()));
        }

        // else
        return Optional<unsigned long long>();
    }

    Nullable<std::string> unwrap_nullable_string(const Registry::NullableString * in) {

        if (in != NULL) {
            return Nullable<std::string>(
                std::string(
                    static_cast<const char *>(in->_value()))
            );
        }
        else {
            return Nullable<std::string>();
        }
    }

    Optional<std::string> unwrap_nullable_string_to_optional(const Registry::NullableString * in) {

        if (in != NULL) {
            return Optional<std::string>(
                std::string(
                    static_cast<const char *>(in->_value()))
            );
        }
        else {
            return Optional<std::string>();
        }
    }
}
