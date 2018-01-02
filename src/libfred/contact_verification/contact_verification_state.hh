/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  contact verification state
 */

#ifndef CONTACT_VERIFICATION_STATE_HH_EC25B8CDFCDB4BE0AF554F99145BFF11
#define CONTACT_VERIFICATION_STATE_HH_EC25B8CDFCDB4BE0AF554F99145BFF11

#include <stdint.h>
#include <string>
#include <stdexcept>

/** Fred */
namespace LibFred {
/** Contact */
namespace Contact {
/** Verification */
namespace Verification {

/**
 * Contact verification state consists of four states
 * conditionallyIdentifiedContact,
 * identifiedContact,
 * validatedContact and
 * mojeidContact.
 */
class State
{
public:
    /**
     * All possible combinations of states. Lower case character means
     * state doesn't present, upper case character means state presents.
     */
    enum Value
    {
        c = 0,     ///< conditionallyIdentifiedContact doesn't present
        C = 1 << 0,///< conditionallyIdentifiedContact presents
        i = 0,     ///< identifiedContact doesn't present
        I = 1 << 1,///< identifiedContact presents
        v = 0,     ///< validatedContact doesn't present
        V = 1 << 2,///< validatedContact presents
        m = 0,     ///< mojeidContact doesn't present
        M = 1 << 3,///< mojeidContact presents
        civm = c | i | v | m,///< civm
        civM = c | i | v | M,///< civM
        ciVm = c | i | V | m,///< ciVm
        ciVM = c | i | V | M,///< ciVM
        cIvm = c | I | v | m,///< cIvm
        cIvM = c | I | v | M,///< cIvM
        cIVm = c | I | V | m,///< cIVm
        cIVM = c | I | V | M,///< cIVM
        Civm = C | i | v | m,///< Civm
        CivM = C | i | v | M,///< CivM
        CiVm = C | i | V | m,///< CiVm
        CiVM = C | i | V | M,///< CiVM
        CIvm = C | I | v | m,///< CIvm
        CIvM = C | I | v | M,///< CIvM
        CIVm = C | I | V | m,///< CIVm
        CIVM = C | I | V | M,///< CIVM
    };
    State(enum Value _value = civm):value_(_value) { }
    /**
     * @throw ConversionFailure if _state doesn't represent any
     */
    State(const std::string &_state):value_(str2value(_state)) { }
    State(const State &_src):value_(_src.value_) { }
    State& operator=(enum Value _value) { value_ = _value; return *this; }
    /**
     * @throw ConversionFailure if _state doesn't represent any
     */
    State& operator=(const std::string &_state) { return *this = str2value(_state); }
    State& operator=(const State &_src) { value_ = _src.value_; return *this; }
    /**
     * @throw ConversionFailure if _state doesn't represent any
     */
    State& set(const std::string &_state) { return *this = _state; }
    /**
     * @throw ConversionFailure if _state doesn't represent any
     */
    State& add(const std::string &_state);
    enum Value get()const { return value_; }
    /**
     * Exception class.
     */
    class ResultUndefined:public std::runtime_error
    {
    public:
        ResultUndefined(const std::string &_msg):std::runtime_error(_msg) { }
    };
    /**
     * @throw ResultUndefined if _mask is empty
     */
    bool has_all(enum Value _mask)const
    {
        if (_mask != civm) {
            return (value_ & _mask) == _mask;
        }
        throw ResultUndefined("empty mask");
    }
    /**
     * @throw ResultUndefined if _mask is empty
     */
    bool has_any(enum Value _mask)const
    {
        if (_mask != civm) {
            return (value_ & _mask) != civm;
        }
        throw ResultUndefined("empty mask");
    }
    bool operator==(enum Value _value)const { return value_ == _value; }
    /**
     * Exception class.
     */
    class ConversionFailure:public std::runtime_error
    {
    public:
        ConversionFailure(const std::string &_msg):std::runtime_error(_msg) { }
    };
    /**
     * @throw ConversionFailure if _state doesn't represent any
     */
    static enum Value str2value(const std::string &_state);
    static const std::string STR_C;///< conditionallyIdentifiedContact
    static const std::string STR_I;///< identifiedContact
    static const std::string STR_V;///< validatedContact
    static const std::string STR_M;///< mojeidContact
private:
    enum Value value_;
};

/**
 * Lock four verification states of contact.
 * @param _contact_id contact id
 */
void lock_contact_verification_states(::uint64_t _contact_id);

/**
 * Exception class.
 */
class ContactNotFound:public std::runtime_error
{
public:
    ContactNotFound(const std::string &_msg):std::runtime_error(_msg) { }
};
/**
 * Lock four verification states of contact.
 * @param _contact_handle contact handle
 * @return contact id
 * @throw ContactNotFound if contact handle doesn't exist
 */
::uint64_t lock_contact_verification_states(const std::string &_contact_handle);

/**
 * Get four verification states of contact.
 * @param _contact_id contact id
 * @return four verification states of contact
 * @throw ContactNotFound if contact od doesn't exist
 * @note contact states must be locked
 */
State get_contact_verification_state(::uint64_t _contact_id);

/**
 * Get four verification states of contact.
 * @param _contact_handle contact handle
 * @return four verification states of contact
 * @throw ContactNotFound if contact handle doesn't exist
 * @note contact states must be locked
 */
State get_contact_verification_state(const std::string &_contact_handle);

} // namespace Verification
} // namespace Contact
} // namespace LibFred

/**
 * Bitwise OR.
 * @param _a first operand
 * @param _b second operand
 * @return _a OR _b
 */
inline enum LibFred::Contact::Verification::State::Value operator|(
    enum LibFred::Contact::Verification::State::Value _a,
    enum LibFred::Contact::Verification::State::Value _b)
{
    return LibFred::Contact::Verification::State::Value(long(_a) | long(_b));
}

/**
 * Bitwise AND.
 * @param _a first operand
 * @param _b second operand
 * @return _a AND _b
 */
inline enum LibFred::Contact::Verification::State::Value operator&(
    enum LibFred::Contact::Verification::State::Value _a,
    enum LibFred::Contact::Verification::State::Value _b)
{
    return LibFred::Contact::Verification::State::Value(long(_a) & long(_b));
}

/**
 * Operator @a unary @a not means equal to zero.
 * @param _a operand
 * @return true if _a == 0
 */
inline bool operator!(enum LibFred::Contact::Verification::State::Value _a)
{
    return _a == LibFred::Contact::Verification::State::Value(0);
}

#endif
