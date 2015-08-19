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

#include "src/fredlib/object/object_state.h"

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

#include <iostream>
#include <stdexcept>

namespace Registry {
namespace MojeID {
/**
 * MojeID contact verification states.
 */
namespace VrfState {

template < Fred::Object::State::Value FRED_STATE >
struct single
{
    static const Fred::Object::State::Value state = FRED_STATE;
    static const unsigned value = 0x01 << FRED_STATE;
    BOOST_STATIC_ASSERT_MSG(0 < value, "FRED_STATE too big");
};

/**
 * conditionallyIdentifiedContact
 */
struct C:single< Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT > { };

/**
 * identifiedContact
 */
struct I:single< Fred::Object::State::IDENTIFIED_CONTACT > { };

/**
 * validatedContact
 */
struct V:single< Fred::Object::State::VALIDATED_CONTACT > { };

/**
 * mojeidContact
 */
struct M:single< Fred::Object::State::MOJEID_CONTACT > { };

template < typename STATES >
struct composed_id
{
    typedef typename boost::mpl::front< STATES >::type            first;
    typedef typename boost::mpl::erase_key< STATES, first >::type tail;
    static const unsigned value = first::value + composed_id< tail >::value;
};

template < >
struct composed_id< boost::mpl::set0< > >
{
    static const unsigned value = 0;
};

template < typename STATES >
struct composed:composed_id< STATES >
{
    typedef STATES states;
    template < typename EVENT >
    static void on_entry(const EVENT &event);
    template < typename EVENT >
    static void on_exit(const EVENT &event);
};

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

template < typename A, typename B >
struct a_not_in_b:boost::mpl::copy_if< A,
                                       boost::mpl::not_< boost::mpl::contains< B, boost::mpl::_1 > >,
                                       boost::mpl::inserter< boost::mpl::set< >,
                                                             boost::mpl::insert< boost::mpl::_1,
                                                                                 boost::mpl::_2 >
                                                           >
                                     >
{ };

template < typename START, typename NEXT, typename EVENT >
static void set(const EVENT &event)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    typedef typename a_not_in_b< START, NEXT >::type to_reset;
    typedef typename a_not_in_b< NEXT, START >::type to_set;
}

/**
 * State machine implementation (transitions between states).
 */
namespace Machine {

/**
 * Events that can occur.
 */
namespace Event {

struct into_mojeid_request
{
};

struct enter_pin12
{
};

}//namespace Registry::MojeID::VrfState::Machine::Event

/**
 * Actions called when event occur.
 */
namespace Action {

template < typename CURRENT, typename EVENT >
struct transition_disabled
{
    template < typename STATE_PRESENT >
    void operator()(const EVENT&, const STATE_PRESENT&)const
    {
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }
};

struct send_pin12
{
    void operator()(const Event::into_mojeid_request&, int)const { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

}//namespace Registry::MojeID::VrfState::Machine::Action

/**
 * Guards called before transition.
 */
namespace Guard {

struct no_guard
{
    template < typename EVENT, typename STATE_PRESENT >
    void operator()(const EVENT&, const STATE_PRESENT&)const { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

}//namespace Registry::MojeID::VrfState::Machine::Guard

/**
 * process(event, states) will be called on event EVENT for object in CURRENT state:
 *     GUARD()(event, states);
 *     ACTION()(event, states);
 *     CURRENT::on_exit(event, states);
 *     VrfState::set< CURRENT, NEXT >(event);
 *     NEXT::on_entry(event, states);
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

template < typename CURRENT, typename EVENT,
           typename A_ROW >
struct hit:boost::mpl::and_< boost::is_same< CURRENT, typename A_ROW::current >,
                             boost::is_same< EVENT,   typename A_ROW::event   > >
{ };

typedef boost::mpl::set<
    a_row< civm, Event::into_mojeid_request, Action::send_pin12, Guard::no_guard, civm >
> transition_table;

template < typename CURRENT, typename EVENT >
struct event_traits
{
    typedef boost::mpl::count_if< transition_table, hit< CURRENT, EVENT, boost::mpl::_1 > > count;
    static const bool transition_found = 0 < count::value;
    BOOST_STATIC_ASSERT_MSG((count::value == 0) || (count::value == 1), "too many rows with the same key");
};

template < typename CURRENT, typename EVENT,
           bool TRANSITION_FOUND = event_traits< CURRENT, EVENT >::transition_found >
struct get
{
    typedef boost::mpl::find_if< transition_table, hit< CURRENT, EVENT, boost::mpl::_1 > > find_transition;
    typedef typename boost::mpl::deref< typename find_transition::type >::type transition;
};

template < typename CURRENT, typename EVENT >
struct get< CURRENT, EVENT, false >
{
    typedef a_row< CURRENT, EVENT, Action::transition_disabled< CURRENT, EVENT > > transition;
};

template < typename CURRENT >
struct from
{
    typedef CURRENT start_state;
    template < typename NEXT, typename STATE_CHANGED = typename boost::is_same< CURRENT, NEXT >::type >
    struct into
    {
        typedef NEXT finish_state;
        template < typename EVENT >
        static void transit(const EVENT &event)
        {
            start_state::on_exit(event);
            VrfState::set< start_state, finish_state >(event);
            finish_state::on_entry(event);
        }
    };
    template < typename NEXT >
    struct into< NEXT, boost::true_type >
    {
        typedef start_state finish_state;
        template < typename EVENT >
        static void transit(const EVENT&) { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    };
};

template < typename CURRENT_STATE, typename EVENT, typename STATE_PRESENT >
void on_event(const EVENT &event, const STATE_PRESENT &states)
{
    typedef CURRENT_STATE current_state;
    typedef typename get< current_state, EVENT >::transition transition;
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

template < typename STATES >
unsigned to_number(const STATES &states)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return civm::value;
}

template < typename EVENT, typename STATE_PRESENT >
void process(const EVENT &event, const STATE_PRESENT &states)
{
    switch (to_number(states)) {
    case VrfState::civm::value:
        on_event< VrfState::civm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::Civm::value:
        on_event< VrfState::Civm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::cIvm::value:
        on_event< VrfState::cIvm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CIvm::value:
        on_event< VrfState::CIvm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::ciVm::value:
        on_event< VrfState::ciVm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CiVm::value:
        on_event< VrfState::CiVm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::cIVm::value:
        on_event< VrfState::cIVm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CIVm::value:
        on_event< VrfState::CIVm, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::civM::value:
        on_event< VrfState::civM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CivM::value:
        on_event< VrfState::CivM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::cIvM::value:
        on_event< VrfState::cIvM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CIvM::value:
        on_event< VrfState::CIvM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::ciVM::value:
        on_event< VrfState::ciVM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CiVM::value:
        on_event< VrfState::CiVM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::cIVM::value:
        on_event< VrfState::cIVM, EVENT, STATE_PRESENT >(event, states);
        return;
    case VrfState::CIVM::value:
        on_event< VrfState::CIVM, EVENT, STATE_PRESENT >(event, states);
        return;
    }
    throw std::runtime_error("unexpected state reigns");
}

}//namespace Registry::MojeID::VrfState::Machine
}//namespace Registry::MojeID::VrfState
}//namespace Registry::MojeID
}//namespace Registry

#endif//STATE_MACHINE_H_6F28C6F85A47DBFD2DEEDA5E508AB751
