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
 *  header of mojeid state machine implementation
 */

#ifndef STATE_MACHINE_H_6F28C6F85A47DBFD2DEEDA5E508AB751//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define STATE_MACHINE_H_6F28C6F85A47DBFD2DEEDA5E508AB751

#include <boost/static_assert.hpp>
#include <boost/mpl/count_if.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/type_traits/is_same.hpp>

#include <stdexcept>

/// State machine matters.
namespace StateMachine {

template < class DERIVED >
struct state_on_change_action
{
    template < typename EVENT, typename STATE_PRESENT >
    static void on_entry(const EVENT&, const STATE_PRESENT&) { }
    template < typename EVENT, typename STATE_PRESENT >
    static void on_exit(const EVENT&, const STATE_PRESENT&) { }
};

/// Exception thrown when corresponding transition isn't specified in a transition table.
struct transition_not_allowed:std::runtime_error
{
    transition_not_allowed(const std::string &_msg):std::runtime_error(_msg) { }
};

/**
 * Sceleton class for state machine implementation.
 * @tparam DERIVED must define transition_table, empty_state_collection and list_of_checked_states types
 *                 and single_state and state_collection templates.
 * @section state_machine_base_sample Sample of intended usage
 * @code{.cpp}
struct State//two different single states
{
    enum Value
    {
        A_BLA_BLA,
        B_BLA_BLA,
    };
    struct Present
    {
        template < Value V >
        bool get()const { return (V == A_BLA_BLA) ? a : b; }
        bool a:1;
        bool b:1;
    };
};

class easy_state_machine:public StateMachine::base< easy_state_machine >
{
public:
    typedef StateMachine::base< easy_state_machine > base_state_machine;

    template < State::Value SINGLE_STATE >
    struct single:boost::integral_constant< State::Value, SINGLE_STATE > { };

    //two single states converted into types A and B
    struct A:single< State::A_BLA_BLA > { };
    struct B:single< State::B_BLA_BLA > { };

    //represents combination of two single states A and B
    template < bool HAS_A, bool HAS_B >
    struct ab_collection:StateMachine::state_on_change_action< ab_collection< HAS_A, HAS_B > >
    {
        static const bool has_a = HAS_A;
        static const bool has_b = HAS_B;

        template < typename STATE >
        struct add
        {
            typedef ab_collection< has_a || boost::is_same< STATE, A >::value,
                                   has_b || boost::is_same< STATE, B >::value > type;
        };
    };

    //helping template generates ab_collection from boost::mpl::set
    template < typename STATES >
    struct collect
    {
        typedef ab_collection< boost::mpl::contains< STATES, A >::value,
                               boost::mpl::contains< STATES, B >::value > type;
    };

    typedef typename collect< boost::mpl::set<      > >::type ab;//no A, no B
    typedef typename collect< boost::mpl::set< A    > >::type Ab;//   A, no B
    typedef typename collect< boost::mpl::set<    B > >::type aB;//no A,    B
    typedef typename collect< boost::mpl::set< A, B > >::type AB;//   A,    B

    //required by StateMachine::base
    template < typename STATE >
    struct single_state
    {
        //return true if STATE present in STATE_PRESENT object
        template < typename STATE_PRESENT >
        static bool present_in(const STATE_PRESENT &states)
        {
            return states.STATE_PRESENT::template get< STATE::value >();
        }
        //add single STATE into collection STATES
        template < typename STATES >
        struct add_into
        {
            typedef typename STATES::template add< STATE >::type type;
        };
    };

    //required by StateMachine::base
    template < typename STATES >
    struct state_collection
    {
        //transits from STATES into NEXT on EVENT
        template < typename NEXT, typename EVENT >
        static void set(const EVENT &event)
        {
            std::cout << "\tstate_collection< STATES >::set< NEXT >(const EVENT&) call" << std::endl;
        }
    };

    struct event
    {
        struct move_to_aB_request { };
        struct move_to_aB_process { };
    };

    struct guard:base_state_machine::guard
    {
        struct test
        {
            void operator()(const event::move_to_aB_request&, const State::Present&)const
            {
                std::cout << "\tguard::test()(event::move_to_aB_request)" << std::endl;
            }
            void operator()(const event::move_to_aB_process&, const State::Present&)const
            {
                std::cout << "\tguard::test()(event::move_to_aB_process)" << std::endl;
            }
        };
    };

    struct action:base_state_machine::action
    {
        struct move_to_aB_request
        {
            void operator()(const event::move_to_aB_request&, const State::Present&)const
            {
                std::cout << "\taction::move_to_aB_request(event::move_to_aB_request)" << std::endl;
            }
        };
        struct move_to_aB_process
        {
            void operator()(const event::move_to_aB_process&, const State::Present&)const
            {
                std::cout << "\taction::move_to_aB_process(event::move_to_aB_process)" << std::endl;
            }
        };
    };

    //required by StateMachine::base
    //defines actions on event in a given state
    typedef boost::mpl::set<
    //         Start Event                      Action                      Guard        Next
    //       +------+-------------------------+---------------------------+------------+-----+
        a_row< Ab,   event::move_to_aB_request, action::move_to_aB_request, guard::test, Ab  >,
        a_row< Ab,   event::move_to_aB_process, action::move_to_aB_process, guard::test, aB  >
                           > transition_table;
    //required by StateMachine::base
    typedef ab empty_state_collection;//collection of single states without any state
    //required by StateMachine::base
    typedef boost::mpl::list< A, B > list_of_checked_states;//list of possible single states
};

namespace StateMachine {

template < >
struct state_on_change_action< easy_state_machine::Ab >// specialization of Ab::on_exit
{
    static void on_exit(const easy_state_machine::event::move_to_aB_process&, const State::Present&)
    {
        std::cout << "\tAb::on_exit(event::move_to_aB_process)" << std::endl;
    }
};

template < >
struct state_on_change_action< easy_state_machine::aB >// specialization of aB::on_entry
{
    static void on_entry(const easy_state_machine::event::move_to_aB_process&, const State::Present&)
    {
        std::cout << "\taB::on_entry(event::move_to_aB_process)" << std::endl;
    }
};

}

int main()
{
    State::Present s; s.a = true; s.b = false;//state is Ab
    std::cout << "a_row< Ab, event::move_to_aB_request, action::move_to_aB_request, guard::test, Ab >" << std::endl;
    //current state is Ab and event move_to_aB_request occurred
    easy_state_machine::process(easy_state_machine::event::move_to_aB_request(), s);

    std::cout << std::endl;
    std::cout << "a_row< Ab, event::move_to_aB_process, action::move_to_aB_process, guard::test, aB >" << std::endl;
    //current state is Ab and event move_to_aB_process occurred
    easy_state_machine::process(easy_state_machine::event::move_to_aB_process(), s);

    std::cout << std::endl;
    std::cout << "a_row< aB, event::move_to_aB_process, ... > doesn't exist" << std::endl;
    s.a = false; s.b = true;//state is aB
    try {
        //current state is aB and event move_to_aB_process occurred
        easy_state_machine::process(easy_state_machine::event::move_to_aB_process(), s);
    }
    catch (const easy_state_machine::transition_not_allowed_on< easy_state_machine::event::move_to_aB_process >&) {
        std::cerr << "\ttransition not allowed for event::move_to_aB_process" << std::endl;
    }
    return EXIT_SUCCESS;
}
 * @endcode
 * Result looks like:
 * @code{.txt}
a_row< Ab, event::move_to_aB_request, action::move_to_aB_request, guard::test, Ab >
	guard::test()(event::move_to_aB_request)
	action::move_to_aB_request(event::move_to_aB_request)

a_row< Ab, event::move_to_aB_process, action::move_to_aB_process, guard::test, aB >
	guard::test()(event::move_to_aB_process)
	action::move_to_aB_process(event::move_to_aB_process)
	Ab::on_exit(event::move_to_aB_process)
	state_collection< STATES >::set< NEXT >(const EVENT&) call
	aB::on_entry(event::move_to_aB_process)

a_row< aB, event::move_to_aB_process, ... > doesn't exist
	transition not allowed for event::move_to_aB_process
 * @endcode
 */
template < class DERIVED >
class base
{
public:
    /**
     * The main function called when an event occurs.
     * @tparam EVENT type of occured event
     * @tparam STATE_PRESENT type with actual informations about present of all relevant states
     * @param event object with information about occured event
     * @param states actual present of all relevant states
     * 
     * It calls corresponding event handler depends on current states which looks into
     * DERIVED::transition_table and does what is specified.
     *
     * Sequence of actions:
     * @code{.cpp}
     *     GUARD()(event, states);
     *     ACTION()(event, states);
     *     CURRENT::on_exit(event, states);
     *     DERIVED::state_collection< CURRENT >::set< NEXT >(event);
     *     NEXT::on_entry(event, states);
     * @endcode
     */
    template < typename EVENT, typename STATE_PRESENT >
    static void process(const EVENT &event, const STATE_PRESENT &states)
    {
        calculate_sum_state< typename derived_class::empty_state_collection,
                             typename derived_class::list_of_checked_states >::process(event, states);
    }

    /// Exception thrown when corresponding transition isn't specified in a transition table.
    struct transition_not_allowed:StateMachine::transition_not_allowed
    {
        transition_not_allowed(const std::string &_msg):StateMachine::transition_not_allowed(_msg) { }
    };

    /// Exception thrown when corresponding reaction on given EVENT isn't specified in a transition table.
    template < typename EVENT >
    struct transition_not_allowed_on:transition_not_allowed
    {
        transition_not_allowed_on(const std::string &_msg):transition_not_allowed(_msg) { }
    };
protected:
    /// Guards called before transition.
    struct guard
    {
        /// Default guard, does nothing.
        struct no_guard
        {
            template < typename EVENT, typename STATE_PRESENT >
            void operator()(const EVENT&, const STATE_PRESENT&)const { }
        };
    };

    /// Actions called when event occur.
    struct action
    {
        /// Default action used when transition wasn't found in a transition table.
        template < typename CURRENT, typename EVENT >
        struct transition_disabled
        {
            template < typename STATE_PRESENT >
            void operator()(const EVENT&, const STATE_PRESENT&)const
            {
                throw transition_not_allowed_on< EVENT >(__PRETTY_FUNCTION__);
            }
        };
    };

    /**
     * Specifies what to do when some event occurs and some states reign.
     * @tparam CURRENT actual state
     * @tparam EVENT what happened
     * @tparam ACTION what to do
     * @tparam GUARD check called before action; it has to throw exception when something wrong happened
     * @tparam NEXT at the end transits into this state
     */
    template < typename CURRENT_STATE,
               typename EVENT,
               typename ACTION,
               typename GUARD = typename guard::no_guard,
               typename NEXT_STATE = CURRENT_STATE >
    struct a_row
    {
        typedef CURRENT_STATE current_state;
        typedef EVENT         event;
        typedef ACTION        action;
        typedef GUARD         guard;
        typedef NEXT_STATE    next_state;
    };
private:
    typedef DERIVED derived_class;

    template < typename CURRENT_STATE, typename EVENT, typename STATE_PRESENT >
    static void on_event(const EVENT &event, const STATE_PRESENT &states)
    {
        typedef typename base::template get< CURRENT_STATE, EVENT >::transition transition;
        typedef CURRENT_STATE                   current_state;
        typedef typename transition::guard      guard;
        typedef typename transition::action     action;
        typedef typename transition::next_state next_state;

        guard guard_invoke;
        guard_invoke(event, states);

        action action_invoke;
        action_invoke(event, states);

        typedef typename from< current_state >::template into< next_state > from_into;
        from_into::transit(event, states);
    }

    template < typename CURRENT_STATE, typename EVENT,
               typename A_ROW >
    struct hit:boost::mpl::and_< boost::is_same< CURRENT_STATE, typename A_ROW::current_state >,
                                 boost::is_same< EVENT,         typename A_ROW::event         > >
    { };

    template < typename CURRENT_STATE, typename EVENT >
    struct event_traits
    {
        typedef typename derived_class::transition_table transition_table;
        typedef boost::mpl::count_if< transition_table, hit< CURRENT_STATE, EVENT, boost::mpl::_1 > > count;
        static const bool transition_found = 0 < count::value;
        BOOST_STATIC_ASSERT_MSG((count::value == 0) || (count::value == 1), "too many rows with the same key");
    };

    template < typename CURRENT_STATE, typename EVENT,
               bool TRANSITION_FOUND = event_traits< CURRENT_STATE, EVENT >::transition_found >
    struct get
    {
        typedef typename derived_class::transition_table transition_table;
        typedef boost::mpl::find_if< transition_table, base::hit< CURRENT_STATE, EVENT, boost::mpl::_1 > > find_transition;
        typedef typename boost::mpl::deref< typename find_transition::type >::type transition;
    };

    template < typename CURRENT_STATE, typename EVENT >
    struct get< CURRENT_STATE, EVENT, false >
    {
        typedef typename action::template transition_disabled< CURRENT_STATE, EVENT > default_action;
        typedef a_row< CURRENT_STATE, EVENT, default_action > transition;
    };

    template < typename CURRENT_STATE >
    struct from
    {
        typedef CURRENT_STATE from_states;
        template < typename NEXT_STATE, bool NO_TRANSITION = boost::is_same< CURRENT_STATE, NEXT_STATE >::value >
        struct into
        {
            typedef NEXT_STATE to_states;
            template < typename EVENT, typename STATE_PRESENT >
            static void transit(const EVENT &event, const STATE_PRESENT &states)
            {
                from_states::on_exit(event, states);
                derived_class::template state_collection< from_states >::template set< to_states >(event);
                to_states::on_entry(event, states);
            }
        };
        template < typename NEXT_STATE >
        struct into< NEXT_STATE, true >
        {
            typedef from_states to_states;
            template < typename EVENT, typename STATE_PRESENT >
            static void transit(const EVENT&, const STATE_PRESENT&) { }
        };
    };

    template < typename SUM_STATE, typename TO_CHECK, bool NOTHING_TO_CHECK = boost::mpl::empty< TO_CHECK >::value >
    struct calculate_sum_state
    {
        template < typename EVENT, typename STATE_PRESENT >
        static void process(const EVENT &event, const STATE_PRESENT &states)
        {
            typedef typename boost::mpl::front< TO_CHECK >::type             checked;
            typedef typename boost::mpl::pop_front< TO_CHECK >::type         left_states_to_check;
            typedef typename derived_class::template single_state< checked > state_to_check;
            if (state_to_check::present_in(states)) {
                typedef typename state_to_check::template add_into< SUM_STATE >::type partial_sum_state;
                calculate_sum_state< partial_sum_state, left_states_to_check >::process(event, states);
            }
            else {
                typedef SUM_STATE partial_sum_state;
                calculate_sum_state< partial_sum_state, left_states_to_check >::process(event, states);
            }
        }
    };

    template < typename CURRENT_STATE, typename TO_CHECK >
    struct calculate_sum_state< CURRENT_STATE, TO_CHECK, true >
    {
        template < typename EVENT, typename STATE_PRESENT >
        static void process(const EVENT &event, const STATE_PRESENT &states)
        {
            on_event< CURRENT_STATE >(event, states);
        }
    };
};

}//namespace StateMachine

#endif//STATE_MACHINE_H_6F28C6F85A47DBFD2DEEDA5E508AB751
