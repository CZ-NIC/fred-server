/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
*  @file
*  header of common corba types wrapper
*/
#ifndef CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492

#include "util/db/nullable.h"

#include <string>
#include <stdexcept>
#include <omniORB4/CORBA.h>
#include </usr/include/boost/type_traits.hpp>

/**
 * All about CORBA interface.
 */
namespace Corba
{
/**
 * Conversions between CORBA and "standard" types.
 * \code{.cpp}
void test(CORBA::String_member &sm)
{
    const char *const cs = Corba::Conversion::from(sm).into< const char* >();//from CORBA::String_member into const char*
    const std::string s  = Corba::Conversion::from(sm).into< std::string >();//from CORBA::String_member into std::string

    Corba::Conversion::from(sm).into(cs);                                    //from CORBA::String_member into const char*
    Corba::Conversion::from(sm).into(s);                                     //from CORBA::String_member into std::string

    Corba::Conversion::into(sm).from(cs);                                    //from const char* into CORBA::String_member
    Corba::Conversion::into(sm).from(s);                                     //from std::string into CORBA::String_member
}

namespace Corba {
namespace Conversion {

//How to define particular convertor from CORBA type into "standard" type
template < >
struct from_into< Registry::MojeID::Date, boost::gregorian::date >
: from_into_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const
    {
        return dst = boost::gregorian::from_simple_string(src);
    }
};

//How to define particular convertor from "standard" type into CORBA type
template < >
struct into_from< Registry::MojeID::Date, boost::gregorian::date >
: into_from_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = CORBA::string_dup(boost::gregorian::to_iso_extended_string(src).c_str());
    }
};

}//Corba::Conversion
}//Corba
 * \endcode
 */
namespace Conversion
{

/**
 * Helping template class for conversions from CORBA types into standard types.
 * @param CORBA_TYPE objects of this type will be converted into values of another types
 */
template < typename CORBA_TYPE >
class From;

/**
 * Basic template function creates temporary object which has functions for conversion into value of another types.
 * @tparam CORBA_TYPE type of source value
 * @param v instance of CORBA_TYPE which will be converted into values of another types
 */
template < typename CORBA_TYPE >
From< CORBA_TYPE >  from(const CORBA_TYPE &v);

template < typename CORBA_TYPE >
From< CORBA_TYPE* > from(CORBA_TYPE *ptr);

template < typename CORBA_TYPE >
From< CORBA_TYPE* > from(const CORBA_TYPE *ptr);

/**
 * Specialization for CORBA type generated by _CORBA_Value_Var template.
 * @tparam CORBA_TYPE basic class used in _CORBA_Value_Var template
 * @tparam CORBA_TYPE_HELPER helper class used in _CORBA_Value_Var template
 * @param v instance of _CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER > type
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
From< CORBA_TYPE* > from(const _CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER > &v);

/**
 * Specialization for CORBA type generated by _CORBA_Value_Member template.
 * @tparam CORBA_TYPE basic class used in _CORBA_Value_Member template
 * @tparam CORBA_TYPE_HELPER helper class used in _CORBA_Value_Member template
 * @param v instance of _CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER > type
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
From< CORBA_TYPE* > from(const _CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER > &v);

/**
 * Basic template class which specializations convert value of CORBA_TYPE into value of CONVERTIBLE_TYPE.
 * @param CORBA_TYPE type of source value
 * @param CONVERTIBLE_TYPE type of destination object into which will be source value converted
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct from_into;

/**
 * Helping template class which defines derived kinds of types.
 * @param CORBA_TYPE type of source value
 * @param CONVERTIBLE_TYPE type of destination object into which will be source value converted
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct from_into_base;

template < typename CORBA_TYPE >
class From
{
public:
    typedef CORBA_TYPE source_type;///< type of source value
    /**
     * Converts value of source_type into value of DST type.
     * @tparam DST type of destination value
     * @return converted value
     */
    template < typename DST >
    DST into()const { DST tmp; return this->into(tmp); }
    /**
     * Sets object of DST type with value of source_type.
     * @tparam DST type of destination value
     * @param dst destination object
     * @return reference to the object with converted value
     */
    template < typename DST >
    DST& into(DST &dst)const { return from_into< source_type, DST >()(source_value_, dst); }
    /**
     * Specialization for std::string type.
     * @param dst destination object
     * @return reference to the object with converted value
     */
    inline std::string& into(std::string &dst)const;
    /**
     * Specialization for nullable types.
     * @param dst destination object
     * @return reference to the object with converted value
     */
    template < typename DST >
    Nullable< DST >& into(Nullable< DST > &dst)const
    {
        DST value;
        from(source_value_).into(value);
        return dst = value;
    }
private:
    From(const source_type &_value):source_value_(_value) { }
    From(const From &_src):source_value_(_src.source_value_) { }
    const source_type &source_value_;
    friend From< source_type > from< source_type >(const source_type&);
};

/**
 * Helping template class collects informations and functions about types based on Nullable template class.
 * @param T investigated type
 */
template < typename T >
struct nullable_traits
{
    typedef T             type;         ///< investigated type
    typedef Nullable< T > nullable_type;///< corresponding nullable type
    /**
     * Get value from object of corresponding nullable type.
     * @param n object of corresponding nullable type
     * @return wrapped type value in case of object isn't null
     * @throw std::runtime_error in case of object is null
     */
    static T get_value(const nullable_type &n)
    {
        if (!n.isnull()) {
            return n.get_value();
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: unable convert NULL value");
    }
    /**
     * Set value representing null.
     * @param v object which has to be set
     * @return never returns
     * @throw always throws std::runtime_error because type doesn't have nullable semantics
     */
    static type& set_null_value(type &v)
    {
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: type doesn't have nullable semantics");
    }
    /**
     * Converts value of SRC type into value of investigated type using from_into template class.
     * @param src pointer to source value
     * @param dst object which has to be set
     * @return converted value
     * @throw throws std::runtime_error if source value is null
     */
    template < typename SRC >
    static type& convert(const SRC *src, type &dst)
    {
        if (src != NULL) {
            return from_into< SRC*, type >()(src, dst);
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: unable convert NULL value");
    }
};

/**
 * Specialization for nullable type family.
 * @param T wrapped type
 */
template < typename T >
struct nullable_traits< Nullable< T > >
{
    typedef T             type;         ///< corresponding wrapped type
    typedef Nullable< T > nullable_type;///< investigated nullable type
    /**
     * Get value from object of corresponding nullable type.
     * @param n object of nullable type
     * @return the same object as param n
     * @throw never throws
     */
    static const nullable_type& get_value(const nullable_type &n) { return n; }
    /**
     * Set value representing null.
     * @param v object which has to be set
     * @return object representing null value
     * @throw never throws
     */
    static nullable_type& set_null_value(nullable_type &v)
    {
        return v = nullable_type();
    }
    /**
     * Converts value of SRC type into value of investigated type using from_into template class.
     * @param src pointer to source value
     * @param dst object which has to be set
     * @return converted value
     */
    template < typename SRC >
    static nullable_type& convert(const SRC *src, nullable_type &dst)
    {
        if (src == NULL) {
            return dst = nullable_type();
        }
        type tmp;
        nullable_traits< type >::convert(src, tmp);
        return dst = tmp;
    }
};

/**
 * Specialization for pointer CORBA types which add nullable semantics.
 * @param CORBA_TYPE type of source value
 */
template < typename CORBA_TYPE >
class From< CORBA_TYPE* >
{
public:
    typedef CORBA_TYPE source_type;///< value of this type is pointed by source
    /**
     * Converts value of source_type into value of DST type.
     * @tparam DST type of destination value
     * @return converted value
     * @throw throws std::exception in case of source value is null and DST type doesn't have nullable semantics
     */
    template < typename DST >
    DST into()const
    {
        typename nullable_traits< DST >::nullable_type dst;
        return nullable_traits< DST >::get_value(this->into(dst));
    }
    /**
     * Sets object of DST type with converted value of source_type type.
     * @tparam DST type of destination value
     * @param dst object to store destination value
     * @return object with converted value
     * @throw throws std::exception in case of source value is null and DST type doesn't have nullable semantics
     */
    template < typename DST >
    DST& into(DST &dst)const
    {
        if (source_value_ptr_ == NULL) {
            return nullable_traits< DST >::set_null_value(dst);
        }
        typedef typename nullable_traits< DST >::type wrapped_type;
        wrapped_type tmp;
        from(source_value_ptr_->_value()).into(tmp);
        return dst = tmp;
    }
    /**
     * Sets object of DST type with converted value of source_type type or with null_value if source value is null.
     * @tparam DST type of destination value
     * @tparam ASSIGNABLE_TO_DST value of this type can be assigned to the destination object
     * @param dst object to store destination value
     * @param null_value represents null value
     * @return object with converted value
     */
    template < typename DST, typename ASSIGNABLE_TO_DST >
    DST& into(DST &dst, const ASSIGNABLE_TO_DST &null_value)const
    {
        if (source_value_ptr_ == NULL) {
            return dst = null_value;
        }
        return dst = DST(source_value_ptr_->_value());
    }
private:
    From(const source_type *_ptr):source_value_ptr_(_ptr) { }
    From(const From &_src):source_value_ptr_(_src.source_value_ptr_) { }
    const source_type *const source_value_ptr_;
    template < typename CT, typename CTH >
    friend From< CT* > from(const _CORBA_Value_Var< CT, CTH >&);
    template < typename CT, typename CTH >
    friend From< CT* > from(const _CORBA_Value_Member< CT, CTH >&);
    template < typename C >
    friend From< C >   from(const C&);
    template < typename C >
    friend From< C* >  from(C*);
    template < typename C >
    friend From< C* >  from(const C*);
};

/**
 * Specialization for CORBA type char* which represents general string type.
 */
template < >
class From< char* >
{
public:
    typedef char source_type;///< value of this type is pointed by source
    /**
     * Sets object of string type with source C-style string value.
     * @param dst object to store destination value
     * @return object with stored value
     */
    inline std::string& into(std::string &dst)const;
    /**
     * Sets object of string type abounded nullable semantics with source C-style string value.
     * @param dst object to store destination value
     * @return object with stored value
     */
    Nullable< std::string >& into(Nullable< std::string > &dst)const
    {
        if (source_value_ptr_ == NULL) {
            return dst = Nullable< std::string >();
        }
        std::string tmp;
        return this->into(tmp).empty() ? dst = Nullable< std::string >()
                                       : dst = tmp;
    }
    /**
     * Converts value of source_type into value of DST type.
     * @tparam DST type of destination value
     * @return converted value
     * @throw throws std::exception in case of source value is null and DST type doesn't have nullable semantics
     */
    template < typename DST >
    DST into()const
    {
        typename nullable_traits< DST >::nullable_type dst;
        return nullable_traits< DST >::get_value(this->into(dst));
    }
    /**
     * Sets object of DST type with converted value of source_type type.
     * @tparam DST type of destination value
     * @param dst object to store destination value
     * @return object with converted value
     * @throw throws std::exception in case of source value is null and DST type doesn't have nullable semantics
     */
    template < typename DST >
    DST& into(DST &dst)const
    {
        return nullable_traits< DST >::convert(source_value_ptr_, dst);
    }
    /**
     * Sets object of DST type with converted value of source_type type or with null_value if source value is null.
     * @tparam DST type of destination value
     * @param dst object to store destination value
     * @param null_value represents null value
     * @return object with converted value
     */
    template < typename DST >
    DST& into(DST &dst, const DST &null_value)const
    {
        if (source_value_ptr_ == NULL) {
            return dst = null_value;
        }
        return from_into< source_type*, DST >()(source_value_ptr_, dst);
    }
private:
    From(const source_type *_ptr):source_value_ptr_(_ptr) { }
    From(const From &_src):source_value_ptr_(_src.source_value_ptr_) { }
    /**
     * Sets object of const char* type with source value of the same type.
     * @param dst object to store destination value
     * @return object with stored value
     * @note hidden disabled operation
     */
    const char*& into(const char* &dst)const;
    const source_type *const source_value_ptr_;
    template < typename C >
    friend From< C* > from(C*);
    template < typename C >
    friend From< C* > from(const C*);
};

template < typename CORBA_TYPE >
From< CORBA_TYPE >  from(const CORBA_TYPE &v) { return v; }

template < typename CORBA_TYPE >
From< CORBA_TYPE* >  from(CORBA_TYPE *ptr) { return const_cast< const CORBA_TYPE* >(ptr); }

template < typename CORBA_TYPE >
From< CORBA_TYPE* >  from(const CORBA_TYPE *ptr) { return ptr; }

template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
From< CORBA_TYPE* > from(const _CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER > &v)
{
    return static_cast< CORBA_TYPE* >(v);
}

template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
From< CORBA_TYPE* > from(const _CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER > &v)
{
    return static_cast< CORBA_TYPE* >(v);
}

template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct from_into_base
{
    typedef CORBA_TYPE        corba_type;      ///< name of source CORBA type
    typedef const CORBA_TYPE &src_value;       ///< type representing source value
    typedef CONVERTIBLE_TYPE  convertible_type;///< name of destination type
    typedef CONVERTIBLE_TYPE &dst_value_ref;   ///< type representing destination object
};

/**
 * Specialization for types pointed to source object.
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct from_into_base< CORBA_TYPE*, CONVERTIBLE_TYPE >
{
    typedef CORBA_TYPE       *corba_type;      ///< name of source CORBA type
    typedef const CORBA_TYPE *src_value;       ///< type representing source value
    typedef CONVERTIBLE_TYPE  convertible_type;///< name of destination type
    typedef CONVERTIBLE_TYPE &dst_value_ref;   ///< type representing destination object
};

/**
 * Specialization for conversion from CORBA::String into std::string.
 * @note String content will be trimmed in the course of conversion.
 */
template < >
struct from_into< char*, std::string >
: from_into_base< char*, std::string >
{
    /**
     * Converts source value of CORBA::String type into object of std::string type.
     * @param src source value
     * @param dst reference to destination object which has to be set
     * @return destination object reference
     * @note Destination string contains trimmed value.
     */
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

/**
 * Specialization for conversion from CORBA::String_var into Nullable< std::string >.
 */
template < >
struct from_into< ::CORBA::String_var, Nullable< std::string > >
: from_into_base< ::CORBA::String_var, Nullable< std::string > >
{
    /**
     * Sets source value of CORBA::String_var type into object of Nullable< std::string > type.
     * @param src source value
     * @param dst reference to destination object which has to be set
     * @return destination object reference
     */
    dst_value_ref operator()(src_value src, dst_value_ref dst)const
    {
        const char *const c_str = static_cast< const char* >(src);
        if (c_str != NULL) {
            return dst = c_str;
        }
        return dst = Nullable< std::string >();
    }
};

/**
 * Specialization for conversion from CORBA::String_member into Nullable< std::string >.
 */
template < >
struct from_into< ::CORBA::String_member, Nullable< std::string > >
: from_into_base< ::CORBA::String_member, Nullable< std::string > >
{
    /**
     * Sets source value of CORBA::String_member type into object of Nullable< std::string > type.
     * @param src source value
     * @param dst reference to destination object which has to be set
     * @return destination object reference
     */
    dst_value_ref operator()(src_value src, dst_value_ref dst)const
    {
        const char *const c_str = static_cast< const char* >(src);
        if (c_str != NULL) {
            return dst = c_str;
        }
        return dst = Nullable< std::string >();
    }
};

template < typename CORBA_TYPE >
std::string& From< CORBA_TYPE >::into(std::string &dst)const
{
    Nullable< std::string > tmp;
    from(source_value_).into(tmp);
    if (!tmp.isnull()) {
        return from_into< char*, std::string >()(tmp.get_value().c_str(), dst);//result will be trimmed
    }
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: result is NULL");
}

std::string& From< char* >::into(std::string &dst)const
{
    return from_into< source_type*, std::string >()(source_value_ptr_, dst);
}

/**
 * Helping template class for conversions from standard types into CORBA types.
 * @param CORBA_TYPE objects of this type will be set with values of another types
 */
template < typename CORBA_TYPE >
class Into;

/**
 * Basic template function creates temporary object which has functions for conversion from values of another types.
 * @tparam CORBA_TYPE type of destination object
 * @param v instance of CORBA_TYPE which will be set from values of another types
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE >
Into< CORBA_TYPE >  into(CORBA_TYPE &v);

/**
 * Specialization for CORBA type generated by _CORBA_Value_Var template.
 * @tparam CORBA_TYPE basic class used in _CORBA_Value_Var template
 * @tparam CORBA_TYPE_HELPER helper class used in _CORBA_Value_Var template
 * @param v instance of _CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER > type
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER >&);

/**
 * Specialization for CORBA type generated by _CORBA_Value_Member template.
 * @tparam CORBA_TYPE basic class used in _CORBA_Value_Member template
 * @tparam CORBA_TYPE_HELPER helper class used in _CORBA_Value_Member template
 * @param v instance of _CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER > type
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER >&);

/**
 * Specialization for CORBA type generated by _CORBA_Value_OUT_arg template.
 * @tparam CORBA_TYPE basic class used in _CORBA_Value_OUT_arg template
 * @tparam CORBA_TYPE_HELPER helper class used in _CORBA_Value_OUT_arg template
 * @param v instance of _CORBA_Value_OUT_arg< CORBA_TYPE, CORBA_TYPE_HELPER > type
 * @return temporary object with conversion functions
 */
template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_OUT_arg< CORBA_TYPE, CORBA_TYPE_HELPER >&);

/**
 * Basic template class which specializations set object of CORBA_TYPE from value of CONVERTIBLE_TYPE.
 * @param CORBA_TYPE type of destination object into which will be source value converted
 * @param CONVERTIBLE_TYPE type of source value
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct into_from
{
    typedef CORBA_TYPE              corba_type;   ///< type of destination object
    typedef CORBA_TYPE             &dst_value_ref;///< reference to the destination object
    typedef const CONVERTIBLE_TYPE &src_value;    ///< type of source value
    /**
     * Converts source value into destination object.
     * @param dst destination object
     * @param src source value
     * @return reference to the destination object
     */
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = src;
    }
};

/**
 * Helping template class which defines derived kinds of types.
 * @param CORBA_TYPE type of destination object into which will be source value converted
 * @param CONVERTIBLE_TYPE type of source value
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct into_from_base;

template < typename CORBA_TYPE >
class Into
{
public:
    typedef CORBA_TYPE  corba_type;   ///< type of destination object
    typedef CORBA_TYPE &dst_value_ref;///< reference to the destination object
    /**
     * Sets object of corba_type with value of SRC type.
     * @tparam SRC type of source value
     * @param  src_value source value
     * @return reference to the object with converted value
     */
    template < typename SRC >
    dst_value_ref from(const SRC &src_value)const
    {
        return into_from< corba_type, SRC >()(destination_, src_value);
    }
    /**
     * Specialization for std::string type.
     * @param src_value source value
     * @return reference to the object with converted value
     */
    dst_value_ref from(const std::string &src_value)const
    {
        return this->from(src_value.c_str());
    }
private:
    Into(dst_value_ref _object):destination_(_object) { }
    Into(const Into &_src):destination_(_src.destination_) { }
    dst_value_ref destination_;
    friend Into< corba_type > into< corba_type >(dst_value_ref);
};

/**
 * Specialization for pointer CORBA types which add nullable semantics.
 * @param CORBA_TYPE type of source value
 */
template < typename CORBA_TYPE >
class Into< CORBA_TYPE* >
{
public:
    typedef CORBA_TYPE   corba_type;   ///< type of destination object
    typedef CORBA_TYPE* &dst_value_ref;///< reference to the destination object
    /**
     * Sets object of corba_type with value of SRC type where src_value is a pointer.
     * @tparam SRC type of source value
     * @param src_value pointer to the source value
     * @return reference to the object with converted value
     */
    template < typename SRC >
    dst_value_ref from(const SRC *src_value)const
    {
        if (src_value != NULL) {
            return into_from< corba_type*, const SRC* >()(destination_, src_value);
        }
        if (destination_ == NULL) {
            return destination_;
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: potential memory leak "
                                 "because overwritten destination doesn't refer to the NULL pointer");
    }
    /**
     * Sets object of corba_type with value of SRC type.
     * @tparam SRC type of source value
     * @param src_value source value
     * @return reference to the object with converted value
     */
    template < typename SRC >
    dst_value_ref from(const SRC &src_value)const
    {
        return into_from< corba_type*, SRC >()(destination_, src_value);
    }
    /**
     * Specialization for std::string type.
     * @param src_value source value
     * @return reference to the object with converted value
     */
    dst_value_ref from(const std::string &src_value)const
    {
        return this->from(src_value.c_str());
    }
    /**
     * Specialization for Nullable types.
     * @tparam SRC type of source value wrapped by Nullable template class
     * @param src_value wrapped source value
     * @return reference to the object with converted value
     */
    template < typename SRC >
    dst_value_ref from(const Nullable< SRC > &src_value)const
    {
        if (!src_value.isnull()) {
            return this->from(src_value.get_value());
        }
        if (destination_ == NULL) {
            return destination_;
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: potential memory leak "
                                 "because overwritten destination doesn't refer to the NULL pointer");
    }
private:
    Into(dst_value_ref _object):destination_(_object) { }
    Into(const Into &_src):destination_(_src.destination_) { }
    dst_value_ref destination_;
    template < typename CT >
    friend Into< CT >  into(CT&);
    template < typename CT, typename CTH >
    friend Into< CT* > into(_CORBA_Value_Var< CT, CTH >&);
    template < typename CT, typename CTH >
    friend Into< CT* > into(_CORBA_Value_Member< CT, CTH >&);
    template < typename CT, typename CTH >
    friend Into< CT* > into(_CORBA_Value_OUT_arg< CT, CTH >&);
};

template < typename CORBA_TYPE >
Into< CORBA_TYPE >  into(CORBA_TYPE &v)
{
    return v;
}

template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_Var< CORBA_TYPE, CORBA_TYPE_HELPER > &v)
{
    return v.out();//reference to the CORBA_TYPE type instance NULL pointer
}

template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_Member< CORBA_TYPE, CORBA_TYPE_HELPER > &v)
{
    return v.out();//reference to the CORBA_TYPE type instance NULL pointer
}

template < typename CORBA_TYPE, typename CORBA_TYPE_HELPER >
Into< CORBA_TYPE* > into(_CORBA_Value_OUT_arg< CORBA_TYPE, CORBA_TYPE_HELPER > &v)
{
    return v.ptr();//reference to the CORBA_TYPE type instance NULL pointer
}

template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE, bool IS_TRIVIAL >
struct create_corba_from_convertible
{
    CORBA_TYPE*& operator()(CORBA_TYPE *&dst, const CONVERTIBLE_TYPE &src)const
    {
        if (dst == NULL) {
            return dst = new CORBA_TYPE(src);//e.g. NullableString(const char* _v)
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: potential memory leak "
                                 "because overwritten destination doesn't refer to the NULL pointer");
    }
};

template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct create_corba_from_convertible< CORBA_TYPE, CONVERTIBLE_TYPE, false >
{
    CORBA_TYPE*& operator()(CORBA_TYPE *&dst, const CONVERTIBLE_TYPE &src)const
    {
        if (dst == NULL) {
            dst = new CORBA_TYPE;         //e.g.          NullableAddress::NullableAddress()
            into(dst->_value()).from(src);//e.g. Address& NullableAddress::_value()
            return dst;
        }
        throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " failure: potential memory leak "
                                 "because overwritten destination doesn't refer to the NULL pointer");
    }
};

/**
 * Specialization in case of CORBA type with nullable semantics.
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct into_from< CORBA_TYPE*, CONVERTIBLE_TYPE >
{
    typedef CORBA_TYPE              corba_type;   ///< type of destination object
    typedef CORBA_TYPE*            &dst_value_ref;///< reference to the destination object
    typedef const CONVERTIBLE_TYPE &src_value;    ///< type for passing source value via function argument
    /**
     * Converts source value into destination object.
     * @param dst destination object
     * @param src source value
     * @return reference to the destination object
     */
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        static const bool is_trivial = boost::has_trivial_constructor< CONVERTIBLE_TYPE >::value;
        create_corba_from_convertible< CORBA_TYPE, CONVERTIBLE_TYPE, is_trivial > create_corba;
        return create_corba(dst, src);
    }
};

template < typename TYPE >
struct into_from< TYPE*, TYPE* >;

template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct into_from_base
{
    typedef CORBA_TYPE              corba_type;      ///< name of destination CORBA type
    typedef CORBA_TYPE             &dst_value_ref;   ///< type for passing destination object into function argument
    typedef CONVERTIBLE_TYPE        convertible_type;///< name of source type
    typedef const CONVERTIBLE_TYPE &src_value;       ///< type for passing source value into function argument
};

/**
 * Specialization for source values passed by pointers.
 * @param CORBA_TYPE type of destination object into which will be source value converted
 * @param CONVERTIBLE_TYPE type pointed by source value
 */
template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct into_from_base< CORBA_TYPE, CONVERTIBLE_TYPE* >
{
    typedef CORBA_TYPE              corba_type;      ///< name of destination CORBA type
    typedef CORBA_TYPE             &dst_value_ref;   ///< type for passing destination object into function argument
    typedef CONVERTIBLE_TYPE        convertible_type;///< name of source type
    typedef const CONVERTIBLE_TYPE *src_value;       ///< type for passing source value into function argument
};

/**
 * Specialization for conversion from const char* into CORBA::String_var.
 */
template < >
struct into_from< ::CORBA::String_var, const char* >
: into_from_base< ::CORBA::String_var, const char* >
{
    /**
     * Converts source value into destination object.
     * @param dst destination object
     * @param src source value
     * @return reference to the destination object
     */
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = src;
    }
};

/**
 * Specialization for conversion from const char* into CORBA::String_member.
 */
template < >
struct into_from< ::CORBA::String_member, const char* >
: into_from_base< ::CORBA::String_member, const char* >
{
    /**
     * Converts source value into destination object.
     * @param dst destination object
     * @param src source value
     * @return reference to the destination object
     */
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = src;
    }
};

/**
 * Specialization for conversion from const char* into CORBA::String_out.
 */
template < >
struct into_from< ::CORBA::String_out, const char* >
: into_from_base< ::CORBA::String_out, const char* >
{
    /**
     * Converts source value into destination object.
     * @param dst destination object
     * @param src source value
     * @return reference to the destination object
     */
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = src;
    }
};

}//Corba::Conversion
}//Corba

#endif//CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492
