/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  contact update
 */

#ifndef UPDATE_CONTACT_H_
#define UPDATE_CONTACT_H_

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/contact_enum.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/contact/place_address.h"

#include "src/admin/contact/verification/contact_states/delete_all.h"

#include <string>
#include <vector>
#include <set>

namespace Fred
{

    /**
     * Update of contact exception tag.
     */
    class UpdateContactET;

    /**
     * Exception type for @ref UpdateContact.
     */
    template <>
    struct ExceptionTraits<UpdateContactET>
    {
        DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);/**< exception members for unknown contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_ssntype, std::string);/**< exception members for unknown type of identification of the contact generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);/**< exception members for unknown registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_country, std::string);/**< exception members for unknown country generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);/**< exception members for unknown sponsoring registrar generated by macro @ref DECLARE_EXCEPTION_DATA*/

        template <class DERIVED_EXCEPTION> struct ExceptionTemplate
            : ExceptionData_unknown_registrar_handle<DERIVED_EXCEPTION>
            , ExceptionData_unknown_sponsoring_registrar_handle<DERIVED_EXCEPTION>
            , ExceptionData_unknown_ssntype<DERIVED_EXCEPTION>
            , ExceptionData_unknown_country<DERIVED_EXCEPTION>
        {};

        /**
         * Exception for accumulation of non-fatal good path errors, fatal good path errors shall be reported immediately
         */
        struct Exception
            : virtual Fred::OperationException
            , ExceptionData_unknown_contact_handle<Exception>
            , ExceptionTemplate<Exception>
        {};

    protected:
        /**
         * Empty destructor meant to be called by derived class.
         */
        ~ExceptionTraits(){}
    };


    /**
     * Container of additional contact addresses to update.
     */
    class ContactAddressToUpdate
    {
    public:
        /**
         * Default constructor, nothing to update, nothing to remove.
         */
        ContactAddressToUpdate() { }
        /**
         * Copy constructor.
         * @param _src refers to instance which is copied
         */
        ContactAddressToUpdate(const ContactAddressToUpdate &_src);
        /**
         * Destructor, nothing to do.
         */
        ~ContactAddressToUpdate() { }
        /**
         * Sets address given purpose to be updated.
         * @tparam type purpose of address
         * @param _address new content of contact address given purpose
         * @return self instance reference to allow method chaining
         */
        template < ContactAddressType::Value type >
        ContactAddressToUpdate& update(const struct ContactAddress &_address);
        /**
         * Contact address given purpose to be removed.
         * @tparam type purpose of address
         * @return self instance reference to allow method chaining
         */
        template < ContactAddressType::Value type >
        ContactAddressToUpdate& remove();
        /**
         * Container of addresses given purpose.
         */
        typedef ContactAddressList ToUpdate;
        /**
         * Container of purposes.
         */
        typedef std::set< ContactAddressType > ToRemove;
        /**
         * Get addresses to update.
         * @return addresses to update
         */
        const ToUpdate& to_update()const { return to_update_; }
        /**
         * Get addresses to remove.
         * @return addresses to remove
         */
        const ToRemove& to_remove()const { return to_remove_; }
        /**
        * Dumps content of the instance into the string
        * @return string with description of the instance content
        */
        std::string to_string()const;
    private:
        ToUpdate to_update_;
        ToRemove to_remove_;
    };

    /**
    * Update of contact, implementation template.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the update.
    * Update is executed by @ref exec  method on contact instance identified by @ref InfoContactOutput parameter and using database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, i.e. good path error, an instance of @ref UpdateContact::ExceptionType is set or set and thrown depending on the fatality of the error.
    * In case of other insuperable failures and inconsistencies, i.e. bad path error, an instance of @ref InternalError or other exception is thrown.
    */
    template <class DERIVED>
    class UpdateContact
        : public virtual Util::Printable
        , public ExceptionTraits<UpdateContactET>
    {
        const std::string registrar_;/**< handle of registrar performing the update */
        Optional<std::string> sponsoring_registrar_;/**< handle of registrar administering the object */
        Optional<std::string> authinfo_;/**< transfer password */
        Optional<std::string> name_ ;/**< name of contact person */
        Optional<std::string> organization_;/**< full trade name of organization */
        Optional< Fred::Contact::PlaceAddress > place_;/**< place address of contact */
        Optional<std::string> telephone_;/**<  telephone number */
        Optional<std::string> fax_;/**< fax number */
        Optional<std::string> email_;/**< e-mail address */
        Optional<std::string> notifyemail_;/**< to this e-mail address will be send message in case of any change in domain or nsset affecting contact */
        Optional<std::string> vat_;/**< taxpayer identification number */
        Optional<std::string> ssntype_;/**< type of identification from enum_ssntype table */
        Optional<std::string> ssn_;/**< unambiguous identification number e.g. social security number, identity card number, date of birth */
        ContactAddressToUpdate addresses_;/**< contact addresses to update or remove */
        Optional<bool> disclosename_;/**< whether to reveal contact name */
        Optional<bool> discloseorganization_;/**< whether to reveal organization */
        Optional<bool> discloseaddress_;/**< whether to reveal address */
        Optional<bool> disclosetelephone_;/**< whether to reveal phone number */
        Optional<bool> disclosefax_;/**< whether to reveal fax number */
        Optional<bool> discloseemail_;/**< whether to reveal email address */
        Optional<bool> disclosevat_;/**< whether to reveal taxpayer identification number */
        Optional<bool> discloseident_;/**< whether to reveal unambiguous identification number */
        Optional<bool> disclosenotifyemail_;/**< whether to reveal notify email */
        Nullable<unsigned long long> logd_request_id_; /**< id of the new entry in log_entry database table, id is used in other calls to logging within current request */

    protected:
        /**
         * Empty destructor meant to be called by derived class.
         */
        ~UpdateContact(){}
    public:
        /**
        * Update contact constructor with mandatory parameter.
        * @param registrar sets registrar handle into @ref registrar_ attribute
        */
        UpdateContact(const std::string& registrar)
        : registrar_(registrar)
        {}

        /**
        * Update contact constructor with all parameters.
        * @param registrar sets registrar handle into @ref registrar_ attribute
        * @param sponsoring_registrar sets sponsoring registrar handle into @ref sponsoring_registrar_ attribute
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @param name sets name of contact person into @ref name_ attribute
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @param place sets place address of contact into @ref place_ attribute
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @param fax sets fax number into @ref fax_ attribute
        * @param email sets e-mail address into @ref email_ attribute
        * @param notifyemail sets e-mail address for notifications into @ref notifyemail_ attribute
        * @param vat sets taxpayer identification number into @ref vat_ attribute
        * @param ssntype sets type of identification into @ref ssntype_ attribute
        * @param ssn sets unambiguous identification number into @ref ssn_ attribute
        * @param addresses sets contact addresses into @ref addresses_ attribute
        * @param disclosename sets whether to reveal contact name into @ref disclosename_ attribute
        * @param discloseorganization sets whether to reveal organization name into @ref discloseorganization_ attribute
        * @param discloseaddress sets whether to reveal contact address into @ref discloseaddress_ attribute
        * @param disclosetelephone sets whether to reveal telephone number into @ref disclosetelephone_ attribute
        * @param disclosefax sets whether to reveal fax number into @ref disclosefax_ attribute
        * @param discloseemail sets whether to reveal e-mail address into @ref discloseemail_ attribute
        * @param disclosevat sets whether to reveal taxpayer identification number into @ref disclosevat_ attribute
        * @param discloseident sets whether to reveal unambiguous identification number into @ref discloseident_ attribute
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into @ref disclosenotifyemail_ attribute
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        */
        UpdateContact(const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional< Fred::Contact::PlaceAddress > &place
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
            , const ContactAddressToUpdate &addresses
            , const Optional<bool>& disclosename
            , const Optional<bool>& discloseorganization
            , const Optional<bool>& discloseaddress
            , const Optional<bool>& disclosetelephone
            , const Optional<bool>& disclosefax
            , const Optional<bool>& discloseemail
            , const Optional<bool>& disclosevat
            , const Optional<bool>& discloseident
            , const Optional<bool>& disclosenotifyemail
            , const Optional<unsigned long long>& logd_request_id
            );

        /**
        * Sets contact sponsoring registrar.
        * @param sponsoring_registrar sets sponsoring registrar handle into @ref sponsoring_registrar_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_sponsoring_registrar(const std::string& sponsoring_registrar)
        {
            sponsoring_registrar_ = sponsoring_registrar;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact transfer password.
        * @param authinfo sets transfer password into @ref authinfo_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_authinfo(const std::string& authinfo)
        {
            authinfo_ = authinfo;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact name.
        * @param name sets name of contact person into @ref name_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_name(const std::string& name)
        {
            name_ = name;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact organization name.
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_organization(const std::string& organization)
        {
            organization_ = organization;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets place address of contact.
        * @param place sets place address of contact into @ref place_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_place(const Fred::Contact::PlaceAddress &place)
        {
            place_ = place;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact telephone number.
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_telephone(const std::string& telephone)
        {
            telephone_ = telephone;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact fax number.
        * @param fax sets fax number into @ref fax_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_fax(const std::string& fax)
        {
            fax_ = fax;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact e-mail address.
        * @param email sets e-mail address into @ref email_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_email(const std::string& email)
        {
            email_ = email;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact e-mail address for notifications.
        * @param notifyemail sets e-mail address for notifications into @ref notifyemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_notifyemail(const std::string& notifyemail)
        {
            notifyemail_ = notifyemail;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact taxpayer identification number.
        * @param vat sets taxpayer identification number into @ref vat_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_vat(const std::string& vat)
        {
            vat_ = vat;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact type of identification.
        * @param ssntype sets type of identification into @ref ssntype_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_ssntype(const std::string& ssntype)
        {
            ssntype_ = ssntype;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets contact type of identification.
        * @param ssn sets unambiguous identification number into @ref ssn_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_ssn(const std::string& ssn)
        {
            ssn_ = ssn;
            return static_cast<DERIVED&>(*this);
        }

        /**
         * Sets address given purpose to be updated.
         * @tparam type purpose of address
         * @param _address new content of contact address given purpose
         * @return self instance reference to allow method chaining
         */
        template < ContactAddressType::Value type >
        DERIVED& set_address(const struct ContactAddress &_address)
        {
            addresses_.update< type >(_address);
            return static_cast<DERIVED&>(*this);
        }

        /**
         * Contact address given purpose to be removed.
         * @tparam type purpose of address
         * @return self instance reference to allow method chaining
         */
        template < ContactAddressType::Value type >
        DERIVED& reset_address()
        {
            addresses_.remove< type >();
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal contact name.
        * @param disclosename sets whether to reveal contact name into @ref disclosename_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_disclosename(const bool disclosename)
        {
            disclosename_ = disclosename;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal organization name.
        * @param discloseorganization sets whether to reveal organization name into @ref discloseorganization_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_discloseorganization(const bool discloseorganization)
        {
            discloseorganization_ = discloseorganization;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal address.
        * @param discloseaddress sets whether to reveal contact address into @ref discloseaddress_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_discloseaddress(const bool discloseaddress)
        {
            discloseaddress_ = discloseaddress;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal telephone number.
        * @param disclosetelephone sets whether to reveal telephone number into @ref disclosetelephone_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_disclosetelephone(const bool disclosetelephone)
        {
            disclosetelephone_ = disclosetelephone;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal fax number.
        * @param disclosefax sets whether to reveal fax number into @ref disclosefax_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_disclosefax(const bool disclosefax)
        {
            disclosefax_ = disclosefax;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal e-mail address.
        * @param discloseemail sets whether to reveal e-mail address into @ref discloseemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_discloseemail(const bool discloseemail)
        {
            discloseemail_ = discloseemail;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal taxpayer identification number.
        * @param disclosevat sets whether to reveal taxpayer identification number into @ref disclosevat_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_disclosevat(const bool disclosevat)
        {
            disclosevat_ = disclosevat;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal unambiguous identification number.
        * @param discloseident sets whether to reveal unambiguous identification number into @ref discloseident_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_discloseident(const bool discloseident)
        {
            discloseident_ = discloseident;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets whether to reveal e-mail address for notifications.
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into @ref disclosenotifyemail_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_disclosenotifyemail(const bool disclosenotifyemail)
        {
            disclosenotifyemail_ = disclosenotifyemail;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Sets logger request id
        * @param logd_request_id sets logger request id into @ref logd_request_id_ attribute
        * @return operation instance reference to allow method chaining
        */
        DERIVED& set_logd_request_id(unsigned long long logd_request_id)
        {
            logd_request_id_ = logd_request_id;
            return static_cast<DERIVED&>(*this);
        }

        /**
        * Executes update
        * @param ctx contains reference to database and logging interface
        * @param contact designated for the update
        * @return new history_id
        */
        unsigned long long exec(OperationContext& ctx, const InfoContactOutput& contact);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//UpdateContact

    /**
     * @class UpdateContactById
     * Update of contact identified by database id.
     * Forward declaration of derived composite class.
     */
    class UpdateContactById;

    /**
     * Exception type for @ref UpdateContactById.
     */
    template <>
    struct ExceptionTraits<UpdateContactById>
    {
        DECLARE_EXCEPTION_DATA(unknown_contact_id, unsigned long long);/**< exception members for unknown contact database id generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
            : virtual Fred::OperationException
            , ExceptionData_unknown_contact_id<Exception>
            , ExceptionTraits<UpdateContactET>::ExceptionTemplate<Exception>
        {};
    protected:
        /**
         * Empty destructor meant to be called by derived class.
         */
        ~ExceptionTraits(){}
    };


    class UpdateContactById  : public virtual Util::Printable,
        public ExceptionTraits<UpdateContactById>,
        public UpdateContact<UpdateContactById>
    {
        unsigned long long id_;
        Fred::InfoContactById select_contact_by_id_;
    public:

        typedef Fred::ExceptionTraits<UpdateContactById>::Exception ExceptionType;/**< Exception type inherited via @ref ExceptionTraits */

        /**
        * Update contact by id constructor with mandatory parameters.
        * @param id sets registrar handle into id_ and InfoContactById members
        * @param registrar sets registrar handle into UpdateContact base
        */
        UpdateContactById(unsigned long long id, const std::string& registrar);

        /**
        * Update contact by id constructor with all parameters.
        * @param id sets database id into id_ and InfoContactById members
        * @param registrar sets registrar handle into UpdateContact base
        * @param sponsoring_registrar sets sponsoring registrar handle into UpdateContact base
        * @param authinfo sets transfer password into UpdateContact base
        * @param name sets name of contact person into UpdateContact base
        * @param organization sets full trade name of organization into UpdateContact base
        * @param place sets place address of contact into UpdateContact base
        * @param telephone sets telephone number into UpdateContact base
        * @param fax sets fax number into UpdateContact base
        * @param email sets e-mail address into UpdateContact base
        * @param notifyemail sets e-mail address for notifications into UpdateContact base
        * @param vat sets taxpayer identification number into UpdateContact base
        * @param ssntype sets type of identification into UpdateContact base
        * @param ssn sets unambiguous identification number into UpdateContact base
        * @param addresses sets contact addresses into UpdateContact base
        * @param disclosename sets whether to reveal contact name into UpdateContact base
        * @param discloseorganization sets whether to reveal organization name into UpdateContact base
        * @param discloseaddress sets whether to reveal contact address into UpdateContact base
        * @param disclosetelephone sets whether to reveal telephone number into UpdateContact base
        * @param disclosefax sets whether to reveal fax number into UpdateContact base
        * @param discloseemail sets whether to reveal e-mail address into UpdateContact base
        * @param disclosevat sets whether to reveal taxpayer identification number into UpdateContact base
        * @param discloseident sets whether to reveal unambiguous identification number into UpdateContact base
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into UpdateContact base
        * @param logd_request_id sets logger request id into UpdateContact base
        */
        UpdateContactById(unsigned long long id
                , const std::string& registrar
                , const Optional<std::string>& sponsoring_registrar
                , const Optional<std::string>& authinfo
                , const Optional<std::string>& name
                , const Optional<std::string>& organization
                , const Optional< Fred::Contact::PlaceAddress > &place
                , const Optional<std::string>& telephone
                , const Optional<std::string>& fax
                , const Optional<std::string>& email
                , const Optional<std::string>& notifyemail
                , const Optional<std::string>& vat
                , const Optional<std::string>& ssntype
                , const Optional<std::string>& ssn
                , const ContactAddressToUpdate &addresses
                , const Optional<bool>& disclosename
                , const Optional<bool>& discloseorganization
                , const Optional<bool>& discloseaddress
                , const Optional<bool>& disclosetelephone
                , const Optional<bool>& disclosefax
                , const Optional<bool>& discloseemail
                , const Optional<bool>& disclosevat
                , const Optional<bool>& discloseident
                , const Optional<bool>& disclosenotifyemail
                , const Optional<unsigned long long>& logd_request_id
                );

        /**
        * Executes update
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(Fred::OperationContext& ctx);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

    /**
     * @class UpdateContactByHandle
     * Update of contact identified by handle.
     * Forward declaration of derived composite class.
     */
    class UpdateContactByHandle;

    /**
     * Exception type for @ref UpdateContactByHandle.
     */
    template <>
    struct ExceptionTraits<UpdateContactByHandle>
    {
        DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);/**< exception members for unknown contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
          , ExceptionData_unknown_contact_handle<Exception>
          , ExceptionTraits<UpdateContactET>::ExceptionTemplate<Exception>
        {};
    protected:
        /**
         * Empty destructor meant to be called by derived class.
         */
        ~ExceptionTraits(){}
    };


    class UpdateContactByHandle  : public virtual Util::Printable,
        public ExceptionTraits<UpdateContactByHandle>,
        public UpdateContact<UpdateContactByHandle>
    {
        const std::string& handle_;
        Fred::InfoContactByHandle select_contact_by_handle_;
    public:

        typedef Fred::ExceptionTraits<UpdateContactByHandle>::Exception ExceptionType;/**< Exception type inherited via @ref ExceptionTraits */

        /**
        * Update contact by handle constructor with mandatory parameters.
        * @param handle sets contact handle into handle_ and InfoContactByHandle members
        * @param registrar sets registrar handle into UpdateContact base
        */
        UpdateContactByHandle(const std::string& handle, const std::string& registrar);

        /**
        * Update contact by handle constructor with mandatory parameters.
        * @param handle sets contact handle into handle_ and InfoContactByHandle members
        * @param registrar sets registrar handle into UpdateContact base
        * @param sponsoring_registrar sets sponsoring registrar handle into UpdateContact base
        * @param authinfo sets transfer password into UpdateContact base
        * @param name sets name of contact person into UpdateContact base
        * @param organization sets full trade name of organization into UpdateContact base
        * @param place sets place address of contact into UpdateContact base
        * @param telephone sets telephone number into UpdateContact base
        * @param fax sets fax number into UpdateContact base
        * @param email sets e-mail address into UpdateContact base
        * @param notifyemail sets e-mail address for notifications into UpdateContact base
        * @param vat sets taxpayer identification number into UpdateContact base
        * @param ssntype sets type of identification into UpdateContact base
        * @param ssn sets unambiguous identification number into UpdateContact base
        * @param addresses sets contact addresses into UpdateContact base
        * @param disclosename sets whether to reveal contact name into UpdateContact base
        * @param discloseorganization sets whether to reveal organization name into UpdateContact base
        * @param discloseaddress sets whether to reveal contact address into UpdateContact base
        * @param disclosetelephone sets whether to reveal telephone number into UpdateContact base
        * @param disclosefax sets whether to reveal fax number into UpdateContact base
        * @param discloseemail sets whether to reveal e-mail address into UpdateContact base
        * @param disclosevat sets whether to reveal taxpayer identification number into UpdateContact base
        * @param discloseident sets whether to reveal unambiguous identification number into UpdateContact base
        * @param disclosenotifyemail sets whether to reveal e-mail address for notifications into UpdateContact base
        * @param logd_request_id sets logger request id into UpdateContact base
        */
        UpdateContactByHandle(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& sponsoring_registrar
                , const Optional<std::string>& authinfo
                , const Optional<std::string>& name
                , const Optional<std::string>& organization
                , const Optional< Fred::Contact::PlaceAddress > &place
                , const Optional<std::string>& telephone
                , const Optional<std::string>& fax
                , const Optional<std::string>& email
                , const Optional<std::string>& notifyemail
                , const Optional<std::string>& vat
                , const Optional<std::string>& ssntype
                , const Optional<std::string>& ssn
                , const ContactAddressToUpdate &addresses
                , const Optional<bool>& disclosename
                , const Optional<bool>& discloseorganization
                , const Optional<bool>& discloseaddress
                , const Optional<bool>& disclosetelephone
                , const Optional<bool>& disclosefax
                , const Optional<bool>& discloseemail
                , const Optional<bool>& disclosevat
                , const Optional<bool>& discloseident
                , const Optional<bool>& disclosenotifyemail
                , const Optional<unsigned long long> logd_request_id
                );

        /**
        * Executes update
        * @param ctx contains reference to database and logging interface
        * @return new history_id
        */
        unsigned long long exec(Fred::OperationContext& ctx);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };


}//namespace Fred

#endif//UPDATE_CONTACT_H_
