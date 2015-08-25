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

#include "src/fredlib/object/get_states_presence.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"

#include <boost/static_assert.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/mpl/copy_if.hpp>
#include <boost/mpl/count_if.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/erase_key.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/empty.hpp>

#include <stdexcept>

namespace Registry {
namespace MojeID {
/// MojeID contact verification states.
namespace VrfState {

/// Converts enum value into type.
template < Fred::Object::State::Value FRED_STATE >
struct single
{
    static const Fred::Object::State::Value state = FRED_STATE;
    static const unsigned value = 0x01 << FRED_STATE;
    BOOST_STATIC_ASSERT_MSG(0 < value, "FRED_STATE too big");
};

/// Represents conditionallyIdentifiedContact
struct C:single< Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT > { };

/// Represents identifiedContact
struct I:single< Fred::Object::State::IDENTIFIED_CONTACT > { };

/// Represents validatedContact
struct V:single< Fred::Object::State::VALIDATED_CONTACT > { };

/// Represents mojeidContact
struct M:single< Fred::Object::State::MOJEID_CONTACT > { };

/// Contains summary value of individual states
template < typename STATES, bool NO_STATE = boost::mpl::empty< STATES >::value >
struct summary_value
{
    typedef typename boost::mpl::front< STATES >::type            first;
    typedef typename boost::mpl::erase_key< STATES, first >::type tail;
    static const unsigned value = first::value + summary_value< tail >::value;
};

/// Specialization for empty set
template < typename STATES >
struct summary_value< STATES, true >
{
    static const unsigned value = 0;
};

/// Actions called on coresponding event
template < typename COMPOSED >
struct on_transition_actions
{
    template < typename EVENT >
    static void on_entry(const EVENT&) { }
    template < typename EVENT >
    static void on_exit(const EVENT&) { }
};

/// Combination of many single states.
template < typename STATES >
struct composed:summary_value< STATES >,
                on_transition_actions< composed< STATES > >
{
    typedef STATES states;
};

// names for particular states combinations
typedef composed< boost::mpl::set<            >::type > civm;
typedef composed< boost::mpl::set< C          >::type > Civm;
typedef composed< boost::mpl::set<    I       >::type > cIvm;
typedef composed< boost::mpl::set< C, I       >::type > CIvm;
typedef composed< boost::mpl::set<       V    >::type > ciVm;
typedef composed< boost::mpl::set< C,    V    >::type > CiVm;
typedef composed< boost::mpl::set<    I, V    >::type > cIVm;
typedef composed< boost::mpl::set< C, I, V    >::type > CIVm;
typedef composed< boost::mpl::set<          M >::type > civM;
typedef composed< boost::mpl::set< C,       M >::type > CivM;
typedef composed< boost::mpl::set<    I,    M >::type > cIvM;
typedef composed< boost::mpl::set< C, I,    M >::type > CIvM;
typedef composed< boost::mpl::set<       V, M >::type > ciVM;
typedef composed< boost::mpl::set< C,    V, M >::type > CiVM;
typedef composed< boost::mpl::set<    I, V, M >::type > cIVM;
typedef composed< boost::mpl::set< C, I, V, M >::type > CIVM;

/// Helping template which contains all items from A set which didn't find in B set.
template < typename A, typename B >
struct a_not_in_b:boost::mpl::copy_if< A,
                                       boost::mpl::not_< boost::mpl::contains< B, boost::mpl::_1 > >,
                                       boost::mpl::inserter< boost::mpl::set< >,
                                                             boost::mpl::insert< boost::mpl::_1,
                                                                                 boost::mpl::_2 >
                                                           >
                                     >
{ };

template < typename STATES, bool NO_STATE = boost::mpl::empty< STATES >::value >
struct state_manipulation
{
    typedef Fred::StatusList state_container;
    typedef state_container::key_type state_item;

    static void add_state_into(state_container &_status_list)
    {
        typedef typename boost::mpl::front< STATES >::type            front;
        typedef typename boost::mpl::erase_key< STATES, front >::type tail;
        _status_list.insert(Fred::Object::State(front::state).into< state_item >());
        state_manipulation< tail >::add_state_into(_status_list);
    }

    static void states_into(state_container &_status_list)
    {
        _status_list.clear();
        add_state_into(_status_list);
#if 1
        for (state_container::const_iterator s_ptr = _status_list.begin(); s_ptr != _status_list.end(); ++s_ptr) {
            if (s_ptr != _status_list.begin()) {
                std::cout << ", ";
            }
            std::cout << (*s_ptr);
        }
        if (!_status_list.empty()) {
            std::cout << std::endl;
        }
#endif
    }

    template < typename EVENT >
    static void set(const EVENT &event)
    {
        Fred::StatusList to_set;
        states_into(to_set);
        Fred::CreateObjectStateRequestId(event.get_object_id(), to_set).exec(event.get_operation_context());
    }

    template < typename EVENT >
    static void reset(const EVENT &event)
    {
        Fred::StatusList to_reset;
        states_into(to_reset);
        Fred::CancelObjectStateRequestId(event.get_object_id(), to_reset).exec(event.get_operation_context());
    }
};

template < typename STATES >
struct state_manipulation< STATES, true >
{
    typedef Fred::StatusList state_container;

    static void add_state_into(const state_container&) { }

    template < typename EVENT >
    static void set(const EVENT&) { }

    template < typename EVENT >
    static void reset(const EVENT&) { }
};

/**
 * Sets and resets particular states so the composed state transits from START into NEXT.
 * @tparam START beginning composed state
 * @tparam NEXT terminal composed state
 * @tparam EVENT what happened
 * @param event object necessary for transition completion
 */
template < typename START, typename NEXT, typename EVENT >
void set(const EVENT &event)
{
    typedef typename a_not_in_b< START, NEXT >::type to_reset;
    typedef typename a_not_in_b< NEXT, START >::type to_set;
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    state_manipulation< to_reset >::reset(event);
    state_manipulation< to_set >::set(event);
}

/// State machine implementation (transitions between states).
namespace Machine {

/// Exception thrown when corresponding transition isn't specified in a transition table.
class transition_not_allowed:public std::runtime_error
{
public:
    transition_not_allowed(const std::string &_msg):std::runtime_error(_msg) { }
};

/// Exception thrown when corresponding reaction on given EVENT isn't specified in a transition table.
template < typename EVENT >
class transition_not_allowed_on:public transition_not_allowed
{
public:
    transition_not_allowed_on(const std::string &_msg):transition_not_allowed(_msg) { }
};

/// Events that can occur.
namespace Event {

struct into_mojeid_request
{
};

struct enter_pin12
{
};

}//namespace Registry::MojeID::VrfState::Machine::Event

/// Actions called when event occur.
namespace Action {

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

struct send_pin12
{
    template < typename STATE_PRESENT >
    void operator()(const Event::into_mojeid_request&, const STATE_PRESENT&)const
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

}//namespace Registry::MojeID::VrfState::Machine::Action

/**
 * Guards called before transition.
 */
namespace Guard {

/// Default guard, does nothing.
struct no_guard
{
    template < typename EVENT, typename STATE_PRESENT >
    void operator()(const EVENT&, const STATE_PRESENT&)const { }
};

}//namespace Registry::MojeID::VrfState::Machine::Guard

/**
 * Specifies what to do when some event occurs and some states reign.
 * @tparam CURRENT actual state
 * @tparam EVENT what happened
 * @tparam ACTION what to do
 * @tparam GUARD check called before action; it has to throw exception when something wrong happened
 * @tparam NEXT at the end transits into this state
 */
template < typename CURRENT,
           typename EVENT,
           typename ACTION,
           typename GUARD = Guard::no_guard,
           typename NEXT = CURRENT >
struct a_row
{
    typedef CURRENT current;
    typedef EVENT   event;
    typedef ACTION  action;
    typedef GUARD   guard;
    typedef NEXT    next;
};

/// Helping template for searching row in transition table. Compares CURRENT and EVENT with given A_ROW.
template < typename CURRENT, typename EVENT,
           typename A_ROW >
struct hit:boost::mpl::and_< boost::is_same< CURRENT, typename A_ROW::current >,
                             boost::is_same< EVENT,   typename A_ROW::event   > >
{ };

/// Helping template for searching row in transition table. Searched row present or doesn't present.
template < typename TRANSITION_TABLE, typename CURRENT, typename EVENT >
struct event_traits
{
    typedef boost::mpl::count_if< TRANSITION_TABLE, hit< CURRENT, EVENT, boost::mpl::_1 > > count;
    static const bool transition_found = 0 < count::value;
    BOOST_STATIC_ASSERT_MSG((count::value == 0) || (count::value == 1), "too many rows with the same key");
};

/// Helping template returning searched row when its present.
template < typename TRANSITION_TABLE, typename CURRENT, typename EVENT,
           bool TRANSITION_FOUND = event_traits< TRANSITION_TABLE, CURRENT, EVENT >::transition_found >
struct get
{
    typedef boost::mpl::find_if< TRANSITION_TABLE, hit< CURRENT, EVENT, boost::mpl::_1 > > find_transition;
    typedef typename boost::mpl::deref< typename find_transition::type >::type transition;
};

/// Helping template returning default row when searching isn't successful.
template < typename TRANSITION_TABLE, typename CURRENT, typename EVENT >
struct get< TRANSITION_TABLE, CURRENT, EVENT, false >
{
    typedef a_row< CURRENT, EVENT, Action::transition_disabled< CURRENT, EVENT > > transition;
};

/// Helping template for calling actions which transits from initial state into final state. When initial
/// and final state is the same then does nothing.
template < typename CURRENT >
struct from
{
    typedef CURRENT start_state;
    typedef typename start_state::states from_states;
    template < typename NEXT, typename STATE_CHANGED = typename boost::is_same< CURRENT, NEXT >::type >
    struct into
    {
        typedef NEXT finish_state;
        typedef typename finish_state::states to_states;
        template < typename EVENT >
        static void transit(const EVENT &event)
        {
            start_state::on_exit(event);
            VrfState::set< from_states, to_states >(event);
            finish_state::on_entry(event);
        }
    };
    template < typename NEXT >
    struct into< NEXT, boost::true_type >
    {
        typedef start_state finish_state;
        template < typename EVENT >
        static void transit(const EVENT&)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
    };
};

/**
 * Looks into TRANSITION_TABLE and does what is specified.
 * @tparam TRANSITION_TABLE crucial component of state machine, specifies reactions on events in a given state
 * @tparam CURRENT_STATE type representing current state
 * @tparam EVENT type of occured event
 * @tparam STATE_PRESENT type with actual informations about present of all relevant states
 *
 * Sequence of actions:
 * @code{.cpp}
 *     GUARD()(event, states);
 *     ACTION()(event, states);
 *     CURRENT::on_exit(event, states);
 *     VrfState::set< CURRENT, NEXT >(event);
 *     NEXT::on_entry(event, states);
 * @endcode
 */
template < typename TRANSITION_TABLE, typename CURRENT_STATE, typename EVENT, typename STATE_PRESENT >
void on_event(const EVENT &event, const STATE_PRESENT &states)
{
    typedef TRANSITION_TABLE transition_table;
    typedef CURRENT_STATE current_state;
    typedef typename get< transition_table, current_state, EVENT >::transition transition;
    typedef typename transition::guard  guard;
    typedef typename transition::action action;
    typedef typename transition::next   next_state;

    guard guard_invoke;
    action action_invoke;
    guard_invoke(event, states);
    action_invoke(event, states);

    typedef typename from< current_state >::template into< next_state > from_into;
    from_into::transit(event);
}

/**
 * It converts states into a number.
 * @tparam STATES type with actual informations about present of all relevant states
 * @param states actual present of all relevant states
 * 
 */
template < typename STATES >
unsigned to_number(const STATES &states)
{
    unsigned retval = 0;
    if (states.STATES::template get< C::state >()) {
        retval |= C::value;
    }
    if (states.STATES::template get< I::state >()) {
        retval |= I::value;
    }
    if (states.STATES::template get< V::state >()) {
        retval |= V::value;
    }
    if (states.STATES::template get< M::state >()) {
        retval |= M::value;
    }
    std::cout << "to_number() = " << retval << std::endl;
    return retval;
}

/**
 * The main function called when an event occurs.
 * @tparam TRANSITION_TABLE crucial component of state machine, specifies reactions on events in a given state
 * @tparam EVENT type of occured event
 * @tparam STATE_PRESENT type with actual informations about present of all relevant states
 * @param event object with information about occured event
 * @param states actual present of all relevant states
 * 
 * It converts states into a number and in dependence on this number calls corresponding event handler.
 */
template < typename TRANSITION_TABLE, typename EVENT, typename STATE_PRESENT >
void process(const EVENT &event, const STATE_PRESENT &states)
{
    switch (to_number(states)) {
    case VrfState::civm::value:
        on_event< TRANSITION_TABLE, VrfState::civm >(event, states);
        return;
    case VrfState::Civm::value:
        on_event< TRANSITION_TABLE, VrfState::Civm >(event, states);
        return;
    case VrfState::cIvm::value:
        on_event< TRANSITION_TABLE, VrfState::cIvm >(event, states);
        return;
    case VrfState::CIvm::value:
        on_event< TRANSITION_TABLE, VrfState::CIvm >(event, states);
        return;
    case VrfState::ciVm::value:
        on_event< TRANSITION_TABLE, VrfState::ciVm >(event, states);
        return;
    case VrfState::CiVm::value:
        on_event< TRANSITION_TABLE, VrfState::CiVm >(event, states);
        return;
    case VrfState::cIVm::value:
        on_event< TRANSITION_TABLE, VrfState::cIVm >(event, states);
        return;
    case VrfState::CIVm::value:
        on_event< TRANSITION_TABLE, VrfState::CIVm >(event, states);
        return;
    case VrfState::civM::value:
        on_event< TRANSITION_TABLE, VrfState::civM >(event, states);
        return;
    case VrfState::CivM::value:
        on_event< TRANSITION_TABLE, VrfState::CivM >(event, states);
        return;
    case VrfState::cIvM::value:
        on_event< TRANSITION_TABLE, VrfState::cIvM >(event, states);
        return;
    case VrfState::CIvM::value:
        on_event< TRANSITION_TABLE, VrfState::CIvM >(event, states);
        return;
    case VrfState::ciVM::value:
        on_event< TRANSITION_TABLE, VrfState::ciVM >(event, states);
        return;
    case VrfState::CiVM::value:
        on_event< TRANSITION_TABLE, VrfState::CiVM >(event, states);
        return;
    case VrfState::cIVM::value:
        on_event< TRANSITION_TABLE, VrfState::cIVM >(event, states);
        return;
    case VrfState::CIVM::value:
        on_event< TRANSITION_TABLE, VrfState::CIVM >(event, states);
        return;
    }
    throw std::runtime_error("unexpected state reigns");
}

}//namespace Registry::MojeID::VrfState::Machine
}//namespace Registry::MojeID::VrfState
}//namespace Registry::MojeID
}//namespace Registry

#endif//STATE_MACHINE_H_6F28C6F85A47DBFD2DEEDA5E508AB751
