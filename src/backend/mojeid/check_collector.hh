/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  Check collection template classes
 */

#ifndef CHECK_COLLECTOR_HH_9AAF6BC7D16249E79BE9E2401EFAD32B
#define CHECK_COLLECTOR_HH_9AAF6BC7D16249E79BE9E2401EFAD32B

#include <boost/mpl/clear.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/void.hpp>
#include <boost/regex.hpp>
#include <boost/static_assert.hpp>
#include <stdexcept>

namespace Fred {
namespace Backend {
namespace MojeId {

/**
 * Encapsulates up to 5 function arguments into one object.
 * @param T0 type of first argument
 * @param T1 type of second argument
 * @param T2 type of third argument
 * @param T3 type of fourth argument
 * @param T4 type of fifth argument
 */
template <typename T0, typename T1 = boost::mpl::void_, typename T2 = boost::mpl::void_, typename T3 = boost::mpl::void_, typename T4 = boost::mpl::void_>
struct Args
{
    typedef T0 Current;
    typedef Args<T1, T2, T3, T4> Tail;
    Args(T0& _v, const Tail& _t)
        : v(_v), t(_t)
    {
    }
    Current& first()
    {
        return v;
    }
    const Tail& rest() const
    {
        return t;
    }
    template < ::size_t IDX, typename T = Current>
    struct At
    {
        typedef typename Tail::template At<IDX - 1>::type type;
        static type& value(const Args& _args)
        {
            return _args.rest().template value<IDX - 1>();
        }
    };
    template <typename T>
    struct At<0, T>
    {
        typedef T type;
        static type& value(const Args& _args)
        {
            return _args.v;
        }
    };
    template < ::size_t IDX>
    typename At<IDX>::type& value() const
    {
        return At<IDX>::value(*this);
    }
    Current& v;
    Tail t;
};

/**
 * Specialization for one argument.
 */
template <typename T0>
struct Args<T0>
{
    typedef T0 Current;
    Args(T0& _v0)
        : v(_v0)
    {
    }
    Current& first()
    {
        return v;
    }
    template < ::size_t IDX, typename T = Current>
    struct At;
    template <typename T>
    struct At<0, T>
    {
        typedef T type;
        static type& value(const Args& _args)
        {
            return _args.v;
        }
    };
    template < ::size_t IDX>
    typename At<IDX>::type& value() const
    {
        return v;
    }
    Current& v;
};

/**
 * Encapsulates one argument of arbitrary type into one object.
 * @param a0 first argument
 * @return object with this argument
 */
template <typename T0>
Args<T0> make_args(T0& a0)
{
    return Args<T0>(a0);
}

/**
 * Joins two arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @return collection of two arguments
 */
template <typename T0, typename T1>
Args<T0, T1> make_args(T0& a0, T1& a1)
{
    return Args<T0, T1>(a0, make_args(a1));
}

/**
 * Joins three arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @return collection of three arguments
 */
template <typename T0, typename T1, typename T2>
Args<T0, T1, T2> make_args(T0& a0, T1& a1, T2& a2)
{
    return Args<T0, T1, T2>(a0, make_args(a1, a2));
}

/**
 * Joins four arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @param a3 fourth argument
 * @return collection of four arguments
 */
template <typename T0, typename T1, typename T2, typename T3>
Args<T0, T1, T2, T3> make_args(T0& a0, T1& a1, T2& a2, T3& a3)
{
    return Args<T0, T1, T2, T3>(a0, make_args(a1, a2, a3));
}

/**
 * Joins five arguments of arbitrary types into one object.
 * @param a0 first argument
 * @param a1 second argument
 * @param a2 third argument
 * @param a3 fourth argument
 * @param a4 fifth argument
 * @return collection of five arguments
 */
template <typename T0, typename T1, typename T2, typename T3, typename T4>
Args<T0, T1, T2, T3, T4> make_args(T0& a0, T1& a1, T2& a2, T3& a3, T4& a4)
{
    return Args<T0, T1, T2, T3, T4>(a0, make_args(a1, a2, a3, a4));
}

/**
 * Unifies constructor call with different number of arguments into one uniform interface.
 * @param C class which constructor is called
 */
template <typename C>
struct ConstructWithArgs : C
{
    /**
     * Constructor with five arguments is used.
     */
    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    ConstructWithArgs(const Args<T0, T1, T2, T3, T4>& _a)
        : C(_a.template value<0>(),
                  _a.template value<1>(),
                  _a.template value<2>(),
                  _a.template value<3>(),
                  _a.template value<4>())
    {
    }
    /**
     * Constructor with four arguments is used.
     */
    template <typename T0, typename T1, typename T2, typename T3>
    ConstructWithArgs(const Args<T0, T1, T2, T3>& _a)
        : C(_a.template value<0>(),
                  _a.template value<1>(),
                  _a.template value<2>(),
                  _a.template value<3>())
    {
    }
    /**
     * Constructor with three arguments is used.
     */
    template <typename T0, typename T1, typename T2>
    ConstructWithArgs(const Args<T0, T1, T2>& _a)
        : C(_a.template value<0>(),
                  _a.template value<1>(),
                  _a.template value<2>())
    {
    }
    /**
     * Constructor with two arguments is used.
     */
    template <typename T0, typename T1>
    ConstructWithArgs(const Args<T0, T1>& _a)
        : C(_a.template value<0>(),
                  _a.template value<1>())
    {
    }
    /**
     * Constructor with one argument is used.
     */
    template <typename T0>
    ConstructWithArgs(const Args<T0>& _a)
        : C(_a.template value<0>())
    {
    }
};

/**
 * Wrapper for check class. This wrapper does nothing.
 * @param CHECK wrapped class
 */
template <class CHECK>
struct check_wrapper_exec_all
{
    typedef CHECK type; ///< returns unchanged CHECK => no wrapper
};

/**
 * Common exception class derived from std::runtime_error signals unsuccessful check.
 */
class CheckFailure : public std::runtime_error
{
public:
    /**
     * Sets error message.
     */
    CheckFailure()
        : std::runtime_error("check failure")
    {
    }
};
/**
 * Wrapper for check class. This wrapper throws exception in case the check failure.
 * @param CHECK wrapped class
 */
template <class CHECK>
struct check_wrapper_break_on_first_error : CHECK
{
    typedef check_wrapper_break_on_first_error type; ///< returns wrapped class
    /**
     * Executes partial check on arbitrary data type.
     * @param _a0 first argument
     */
    template <typename T0>
    check_wrapper_break_on_first_error(T0& _a0)
        : CHECK(_a0)
    {
        this->throw_on_error();
    }
    /**
     * Executes partial check on arbitrary two arguments.
     * @param _a0 first argument
     * @param _a1 second argument
     */
    template <typename T0, typename T1>
    check_wrapper_break_on_first_error(T0& _a0, T1& _a1)
        : CHECK(_a0, _a1)
    {
        this->throw_on_error();
    }
    /**
     * Exception class signals unsuccessful check. Derived from CheckFailure and from check class which
     * finished unsuccessfully, so it contains details about this failed check.
     */
    class Exception : public CheckFailure, public CHECK
    {
    public:
        /**
         * Creates copy of failed check.
         * @param _src points at failed check
         */
        Exception(const CHECK* _src)
            : CHECK(*_src)
        {
        }
    };

private:
    void throw_on_error() const
    {
        if (!this->CHECK::success())
        {
            throw Exception(this);
        }
    }
};

/**
 * Type traits template surveys if type list contains only nonsequence types.
 * @param LIST type list of which items aren't type lists too
 * @note contains_nonsequences_only< LIST >::value is true or false depending on LIST traits
 */
template <typename LIST>
struct contains_nonsequences_only
        : boost::mpl::and_<boost::mpl::not_<typename boost::mpl::is_sequence<typename boost::mpl::front<LIST>::type> >,
          contains_nonsequences_only<typename boost::mpl::pop_front<LIST>::type> >
{
};

template <>
struct contains_nonsequences_only<typename boost::mpl::clear<boost::mpl::list<> >::type> : boost::true_type
{
};

/**
 * Collection of partial checks.
 * This class publicly inherits from each partial check so details of all checks are still accessible.
 * @param CHECK_LIST list of partial checks
 * @param CHECK_WRAPPER can modify check behaviour
 * @param IS_HOMOGENEOUS depends on type of checks in CHECK_LIST
 */
template <typename CHECK_LIST, template <class> class CHECK_WRAPPER = check_wrapper_exec_all, bool IS_HOMOGENEOUS = contains_nonsequences_only<CHECK_LIST>::value>
struct Check;

/**
 * Specialization for checks with different arguments, currently restricted to a maximum 5 checks.
 * @param HETEROGENEOUS_CHECK_LIST list of partial checks, each constructed with its own set of arguments
 * @param CHECK_WRAPPER can modify check behaviour
 * @note  Each item of HETEROGENEOUS_CHECK_LIST has to be type list of homogeneous checks like this example:
~~~~~~~~~~~~~~{.cpp}
struct checkA0
{
    checkA0(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkA1
{
    checkA1(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkB
{
    checkB(std::string) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkC
{
    checkC(void*, int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

typedef Check< boost::mpl::list< boost::mpl::list< checkA0, checkA1 >,
                                 boost::mpl::list< checkB >,
                                 boost::mpl::list< checkC >
                               >
             > CheckABC;

bool check_finished_successfully(int a, const std::string &b, void *c0, int c1)
{
    return CheckABC(make_args(a), make_args(b), make_args(c0, c1)).success();
}
~~~~~~~~~~~~~~
 */
template <typename HETEROGENEOUS_CHECK_LIST, template <class> class CHECK_WRAPPER>
struct Check<HETEROGENEOUS_CHECK_LIST, CHECK_WRAPPER, false>
        : ConstructWithArgs<Check<typename boost::mpl::front<HETEROGENEOUS_CHECK_LIST>::type, CHECK_WRAPPER, true> >,
          Check<typename boost::mpl::pop_front<HETEROGENEOUS_CHECK_LIST>::type, CHECK_WRAPPER, false>
{
    typedef HETEROGENEOUS_CHECK_LIST Checks;
    BOOST_MPL_ASSERT((boost::mpl::not_<contains_nonsequences_only<Checks> >));
    typedef Check<typename boost::mpl::front<Checks>::type, CHECK_WRAPPER, true> Base;
    typedef ConstructWithArgs<Base> Current;
    typedef Check<typename boost::mpl::pop_front<Checks>::type, CHECK_WRAPPER, false> Tail;
    /**
     * Generates the same Check with other wrapper.
     * @param OTHER_CHECK_WRAPPER use this wrapper
     */
    template <template <class> class OTHER_CHECK_WRAPPER>
    struct ChangeWrapper
    {
        typedef Check<HETEROGENEOUS_CHECK_LIST, OTHER_CHECK_WRAPPER, false> type; ///< the same Check with changed wrapper
    };
    /**
     * Executes 5 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first group of checks
     * @param _a1 arguments for second group of checks
     * @param _a2 arguments for third group of checks
     * @param _a3 arguments for fourth group of checks
     * @param _a4 arguments for fifth group of checks
     * @note arguments can be created by using @ref make_args template function
     */
    template <typename ARG0, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
    Check(const ARG0& _a0, const ARG1& _a1, const ARG2& _a2, const ARG3& _a3, const ARG4& _a4)
        : Current(_a0),
          Tail(_a1, _a2, _a3, _a4)
    {
    }
    /**
     * Executes 4 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first group of checks
     * @param _a1 arguments for second group of checks
     * @param _a2 arguments for third group of checks
     * @param _a3 arguments for fourth group of checks
     * @note arguments can be created by using @ref make_args template function
     */
    template <typename ARG0, typename ARG1, typename ARG2, typename ARG3>
    Check(const ARG0& _a0, const ARG1& _a1, const ARG2& _a2, const ARG3& _a3)
        : Current(_a0),
          Tail(_a1, _a2, _a3)
    {
    }
    /**
     * Executes 3 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first group of checks
     * @param _a1 arguments for second group of checks
     * @param _a2 arguments for third group of checks
     * @note arguments can be created by using @ref make_args template function
     */
    template <typename ARG0, typename ARG1, typename ARG2>
    Check(const ARG0& _a0, const ARG1& _a1, const ARG2& _a2)
        : Current(_a0),
          Tail(_a1, _a2)
    {
    }
    /**
     * Executes 2 checks with different sets of arguments and stores their results into this object.
     * @param _a0 arguments for first group of checks
     * @param _a1 arguments for second group of checks
     * @note arguments can be created by using @ref make_args template function
     */
    template <typename ARG0, typename ARG1>
    Check(const ARG0& _a0, const ARG1& _a1)
        : Current(_a0),
          Tail(_a1)
    {
    }
    /**
     * Executes 1 check and stores their result into this object.
     * @param _a0 arguments for this group of checks
     * @note arguments can be created by using @ref make_args template function
     */
    template <typename ARG0>
    Check(const ARG0& _a0)
        : Current(_a0)
    {
    }
    /**
     * Checks finished successfully.
     * @return true if all partial checks finished successfully
     */
    bool success() const
    {
        return this->Base::success() && this->Tail::success();
    }
};

/**
 * Specialization for checks with the same arguments.
 * @param HOMOGENEOUS_CHECK_LIST list of partial checks, each constructed with the same set of arguments
 * @param CHECK_WRAPPER can modify check behaviour
 * @note  Each check has to contain const method `success` without arguments like this example:
~~~~~~~~~~~~~~{.cpp}
struct checkA0
{
    checkA0(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

struct checkA1
{
    checkA1(int) { }
    bool success()const { return !invalid; }
    bool invalid:1;
};

typedef Check< boost::mpl::list< checkA0,
                                 checkA1 >
             > CheckA;

bool check_finished_successfully(int a)
{
    return CheckA(a).success();
}
~~~~~~~~~~~~~~
 */
template <typename HOMOGENEOUS_CHECK_LIST, template <class> class CHECK_WRAPPER>
struct Check<HOMOGENEOUS_CHECK_LIST, CHECK_WRAPPER, true>
        : CHECK_WRAPPER<typename boost::mpl::front<HOMOGENEOUS_CHECK_LIST>::type>::type,
          Check<typename boost::mpl::pop_front<HOMOGENEOUS_CHECK_LIST>::type, CHECK_WRAPPER, true>
{
    typedef HOMOGENEOUS_CHECK_LIST Checks;
    BOOST_MPL_ASSERT((contains_nonsequences_only<Checks>));
    typedef typename boost::mpl::front<HOMOGENEOUS_CHECK_LIST>::type Current;
    typedef typename CHECK_WRAPPER<Current>::type WrappedCurrent;
    typedef Check<typename boost::mpl::pop_front<Checks>::type, CHECK_WRAPPER, true> Tail;
    /**
     * Generates the same Check with other wrapper.
     * @param OTHER_CHECK_WRAPPER use this wrapper
     */
    template <template <class> class OTHER_CHECK_WRAPPER>
    struct ChangeWrapper
    {
        typedef Check<HOMOGENEOUS_CHECK_LIST, OTHER_CHECK_WRAPPER, true> type; ///< the same Check with changed wrapper
    };
    /**
     * Executes collection of partial checks on arbitrary data type.
     * @param _a0 first argument
     */
    template <typename T0>
    Check(T0& _a0)
        : WrappedCurrent(_a0),
          Tail(_a0)
    {
    }
    /**
     * Executes collection of partial checks on arbitrary two arguments.
     * @param _a0 first argument
     * @param _a1 second argument
     */
    template <typename T0, typename T1>
    Check(T0& _a0, T1& _a1)
        : WrappedCurrent(_a0, _a1),
          Tail(_a0, _a1)
    {
    }
    /**
     * Checks finished successfully.
     * @return true if all partial checks finished successfully
     */
    bool success() const
    {
        return this->Current::success() && this->Tail::success();
    }
};

template <template <class> class CHECK_WRAPPER>
struct Check<typename boost::mpl::clear<boost::mpl::list<> >::type, CHECK_WRAPPER, false>
{
    Check()
    {
    }
    bool success() const
    {
        return true;
    }
};

template <template <class> class CHECK_WRAPPER>
struct Check<typename boost::mpl::clear<boost::mpl::list<> >::type, CHECK_WRAPPER, true>
{
    Check()
    {
    }
    template <typename T0>
    Check(T0&)
    {
    }
    template <typename T0, typename T1>
    Check(T0&, T1&)
    {
    }
    bool success() const
    {
        return true;
    }
};

} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif
