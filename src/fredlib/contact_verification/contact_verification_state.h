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
 *  contact validation state
 */

#ifndef CONTACT_VERIFICATION_STATE_H_85BD98645DD8A487ABA826A5D21C75CE//CONTACT_VERIFICATION_STATE_H_$(date "+%s"|md5sum|tr "[a-f]" "[A-F]")
#define CONTACT_VERIFICATION_STATE_H_85BD98645DD8A487ABA826A5D21C75CE

#include <stdint.h>
#include <string>

/** Fred */
namespace Fred {
/** Contact */
namespace Contact {

/**
 * Contact validation state consists of four states
 * conditionallyIdentifiedContact,
 * identifiedContact,
 * validatedContact and
 * mojeidContact.
 */
class ValidationState
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
    ValidationState(enum Value _value = civm):value_(_value) { }
    ValidationState(const std::string &_state):value_(str2value(_state)) { }
    ValidationState(const ValidationState &_src):value_(_src.value_) { }
    ValidationState& operator=(enum Value _value) { value_ = _value; return *this; }
    ValidationState& operator=(const std::string &_state) { return *this = str2value(_state); }
    ValidationState& operator=(const ValidationState &_src) { value_ = _src.value_; return *this; }
    ValidationState& set(const std::string &_state) { return *this = _state; }
    ValidationState& add(const std::string &_state);
    enum Value get()const { return value_; }
    static enum Value str2value(const std::string &_state);
    static const std::string STR_C;///< conditionallyIdentifiedContact
    static const std::string STR_I;///< identifiedContact
    static const std::string STR_V;///< validatedContact
    static const std::string STR_M;///< mojeidContact
private:
    enum Value value_;
};

/**
 * Lock four validation states of contact.
 * @param _contact_id contact id
 */
void lock_contact_validation_states(::uint64_t _contact_id);

/**
 * Lock four validation states of contact.
 * @param _contact_handle contact handle
 * @return contact id
 * @throw std::runtime_error if contact handle doesn't exist
 */
::uint64_t lock_contact_validation_states(const std::string &_contact_handle);

/**
 * Get four validation states of contact.
 * @param _contact_id contact id
 * @return four validation states of contact
 * @throw std::runtime_error if contact od doesn't exist
 * @note contact states must be locked
 */
ValidationState get_contact_validation_state(::uint64_t _contact_id);

/**
 * Get four validation states of contact.
 * @param _contact_handle contact handle
 * @return four validation states of contact
 * @throw std::runtime_error if contact handle doesn't exist
 * @note contact states must be locked
 */
ValidationState get_contact_validation_state(const std::string &_contact_handle);

} // namespace Contact
} // namespace Fred

/**
 * Logical OR.
 * @param _a first operand
 * @param _b second operand
 * @return _a OR _b
 */
enum Fred::Contact::ValidationState::Value operator|(
    enum Fred::Contact::ValidationState::Value _a,
    enum Fred::Contact::ValidationState::Value _b)
{
    return Fred::Contact::ValidationState::Value(_a | _b);
}

/**
 * Logical AND.
 * @param _a first operand
 * @param _b second operand
 * @return _a AND _b
 */
enum Fred::Contact::ValidationState::Value operator&(
    enum Fred::Contact::ValidationState::Value _a,
    enum Fred::Contact::ValidationState::Value _b)
{
    return Fred::Contact::ValidationState::Value(_a & _b);
}

/**
 * Operator @a unary @a not means equal to zero.
 * @param _a operand
 * @return true if _a == 0
 */
bool operator!(enum Fred::Contact::ValidationState::Value _a)
{
    return _a == Fred::Contact::ValidationState::Value(0);
}

#endif//CONTACT_VERIFICATION_STATE_H_85BD98645DD8A487ABA826A5D21C75CE
