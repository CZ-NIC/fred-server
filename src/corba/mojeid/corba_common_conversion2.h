#ifndef CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492

#include "util/db/nullable.h"

#include <string>
#include <omniORB4/CORBA.h>

template < typename C, typename T >
struct corba_type_conversion;

template < typename C > struct corba_value_nonconst;
template < typename C > corba_value_nonconst< C > corba_value(C&);

template < typename C >
struct corba_value_nonconst
{
    template < typename T >
    T get()const { T dst; return corba_type_conversion< C, T >::set(dst, c); }
    template < typename T >
    T& get(T &dst)const { return corba_type_conversion< C, T >::set(dst, c); }
    template < typename T >
    T& get(T &dst, const T &null_value)const { return corba_type_conversion< C, T >::set(dst, c, null_value); }
    template < typename T >
    C& set(const T &src) { return corba_type_conversion< C, T >::set(c, src); }
private:
    corba_value_nonconst(C &v):c(v) { }
    corba_value_nonconst(const corba_value_nonconst &src):c(src.c) { }
    C &c;
    friend corba_value_nonconst corba_value< C >(C&);
};

template < typename C >
corba_value_nonconst< C > corba_value(C &v) { return v; }

template < typename C > struct corba_value_const;
template < typename C > corba_value_const< C > corba_value(const C&);

template < typename C >
struct corba_value_const
{
    template < typename T >
    T get()const { T dst; return corba_type_conversion< C, T >::set(dst, c); }
    template < typename T >
    T& get(T &dst)const { return corba_type_conversion< C, T >::set(dst, c); }
    template < typename T >
    T& get(T &dst, const T &null_value)const { return corba_type_conversion< C, T >::set(dst, c, null_value); }
    template < typename T >
    T get(const T &null_value)const { T dst; return this->get(dst, null_value); }
private:
    corba_value_const(const C &v):c(v) { }
    corba_value_const(const corba_value_const &src):c(src.c) { }
    const C &c;
    friend corba_value_const corba_value< C >(const C&);
};

template < typename C >
corba_value_const< C > corba_value(const C &v) { return v; }

template < typename CORBA_TYPE, typename CONVERTIBLE_TYPE >
struct corba_type_conversion_base
{
    typedef CORBA_TYPE       corba_type;
    typedef CONVERTIBLE_TYPE convertible_type;
};

template < >
struct corba_type_conversion< ::CORBA::String_out, std::string >
: corba_type_conversion_base< ::CORBA::String_out, std::string >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = CORBA::string_dup(src.c_str());
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = const_cast< corba_type& >(src).ptr();
    }
};

template < >
struct corba_type_conversion< ::CORBA::String_out, const char* >
: corba_type_conversion_base< ::CORBA::String_out, const char* >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = CORBA::string_dup(src);
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = const_cast< corba_type& >(src).ptr();
    }
};

template < >
struct corba_type_conversion< ::CORBA::String_var, std::string >
: corba_type_conversion_base< ::CORBA::String_var, std::string >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = src.c_str();
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = src.in();
    }
};

template < >
struct corba_type_conversion< ::CORBA::String_var, const char* >
: corba_type_conversion_base< ::CORBA::String_var, const char* >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = src;
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = src.in();
    }
};

template < >
struct corba_type_conversion< ::CORBA::String_member, std::string >
: corba_type_conversion_base< ::CORBA::String_member, std::string >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = src.c_str();
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = src.in();
    }
};

template < >
struct corba_type_conversion< ::CORBA::String_member, const char* >
: corba_type_conversion_base< ::CORBA::String_member, const char* >
{
    static corba_type& set(corba_type& dst, const convertible_type &src)
    {
        return dst = src;
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        return dst = src.in();
    }
};

template < typename NC, typename T >
struct nullable_corba_type
{
    static NC* create(const T &src) { return new NC(src); }
};

template < typename NC >
struct nullable_corba_type< NC, std::string >
{
    static NC* create(const std::string &src) { return new NC(src.c_str()); }
};

template < typename NC, typename T >
struct corba_type_conversion< NC*, T >
{
    typedef NC *corba_type;
    typedef T   convertible_type;
    static corba_type& set(corba_type &dst, const convertible_type &src)
    {
        if (src.isnull()) {
            return dst = NULL;
        }
        return dst = nullable_corba_type< NC, T >(src.get_value());
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src, const convertible_type &null_value)
    {
        if (src == NULL) {
            return dst = null_value;
        }
        return dst = convertible_type(src->_value());
    }
};

template < typename NC, typename T >
struct corba_type_conversion< NC*, Nullable< T > >
: corba_type_conversion_base< NC*, Nullable< T > >
{
    typedef NC           *corba_type;
    typedef Nullable< T > convertible_type;
    static corba_type& set(corba_type &dst, const convertible_type &src)
    {
        if (src.isnull()) {
            return dst = NULL;
        }
        return dst = nullable_corba_type< NC, T >(src.get_value());
    }
    static convertible_type& set(convertible_type &dst, const corba_type &src)
    {
        if (src == NULL) {
            return dst = convertible_type();
        }
        return dst = convertible_type(src->_value());
    }
};

#endif//CORBA_COMMON_CONVERSION2_H_9496EF7266599697662705CBCA48A492
