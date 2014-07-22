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
 *  create registrar
 */

#ifndef CREATE_REGISTRAR_H_
#define CREATE_REGISTRAR_H_

#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/printable.h"

namespace Fred
{
    /**
    * Create of registrar.
    * Created instance is modifiable by chainable methods i.e. methods returning instance reference.
    * Data set into instance by constructor and methods serve as input data of the create.
    * Create is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered inconsistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref CreateRegistrar::Exception is thrown with appropriate attributes set.
    * In case of other insuperable failures and inconsistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class CreateRegistrar : public Util::Printable
    {
    public:
        DECLARE_EXCEPTION_DATA(invalid_registrar_handle, std::string);/**< exception members for invalid registrar handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(unknown_country, std::string);/**< exception members for unknown country generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
          , ExceptionData_invalid_registrar_handle<Exception>
          , ExceptionData_unknown_country<Exception>
        {};
    protected:
        const std::string handle_;/**< registrar identifier */
        Optional<std::string> name_;/**< name of the registrar */
        Optional<std::string> organization_;/**< full trade name of organization */
        Optional<std::string> street1_;/**< part of address */
        Optional<std::string> street2_;/**< part of address */
        Optional<std::string> street3_;/**< part of address*/
        Optional<std::string> city_;/**< part of address - city */
        Optional<std::string> stateorprovince_;/**< part of address - region */
        Optional<std::string> postalcode_;/**< part of address - postal code */
        Optional<std::string> country_;/**< two character country code or country name */
        Optional<std::string> telephone_;/**<  telephone number */
        Optional<std::string> fax_;/**< fax number */
        Optional<std::string> email_;/**< e-mail address */
        Optional<std::string> url_;/**< web page of the registrar */
        Optional<bool> system_;/**< system registrar flag */
        Optional<std::string> ico_;/**< company registration number */
        Optional<std::string> dic_;/**< taxpayer identification number */
        Optional<std::string> variable_symbol_;/**< registrar payments coupling tag, have to match with payment variable symbol to couple payment with registrar*/
        Optional<std::string> payment_memo_regex_;/**< registrar payments coupling alternative to variable symbol, if payment_memo_regex is set, payment_memo have to match case insesitive with payment_memo_regex to couple payment with registrar*/
        Optional<bool> vat_payer_;/**< VAT payer flag */
    public:
        /**
        * Create registrar constructor with mandatory parameters.
        * @param handle sets registrar identifier into @ref handle_ attribute
        */
        CreateRegistrar(const std::string& handle);

        /**
        * Create registrar constructor with all parameters.
        * @param handle sets registrar identifier into @ref handle_ attribute
        * @param name sets name of registrar person into @ref name_ attribute
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @param street1 sets part of address into @ref street1_ attribute
        * @param street2 sets part of address into @ref street2_ attribute
        * @param street3 sets part of address into @ref street3_ attribute
        * @param city sets part of address - city into @ref city_ attribute
        * @param stateorprovince sets part of address - region into @ref stateorprovince_ attribute
        * @param postalcode sets part of address - postal code into @ref postalcode_ attribute
        * @param country sets two character country code or country name  into @ref country_ attribute
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @param fax sets fax number into @ref fax_ attribute
        * @param email sets e-mail address into @ref email_ attribute
        * @param url sets web address into @ref url_ attribute
        * @param system sets system registrar flag into @ref system_ attribute
        * @param ico sets company registration number into @ref ico_ attribute
        * @param dic sets taxpayer identification number into @ref dic_ attribute
        * @param variable_symbol sets payments coupling tag into @ref variable_symbol_ attribute
        * @param payment_memo_regex sets alternative payment coupling expression into @ref payment_memo_regex_ attribute
        * @param vat_payer sets VAT payer flag into @ref vat_payer_ attribute
        */
        CreateRegistrar(const std::string& handle,
                const Optional<std::string>& name,
                const Optional<std::string>& organization,
                const Optional<std::string>& street1,
                const Optional<std::string>& street2,
                const Optional<std::string>& street3,
                const Optional<std::string>& city,
                const Optional<std::string>& stateorprovince,
                const Optional<std::string>& postalcode,
                const Optional<std::string>& country,
                const Optional<std::string>& telephone,
                const Optional<std::string>& fax,
                const Optional<std::string>& email,
                const Optional<std::string>& url,
                const Optional<bool>& system,
                const Optional<std::string>& ico,
                const Optional<std::string>& dic,
                const Optional<std::string>& variable_symbol,
                const Optional<std::string>& payment_memo_regex,
                const Optional<bool>& vat_payer);

        virtual ~CreateRegistrar() { }

        /**
        * Sets registrar name.
        * @param name sets name of registrar person into @ref name_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_name(const std::string& name);

        /**
        * Sets registrar organization name.
        * @param organization sets full trade name of organization into @ref organization_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_organization(const std::string& organization);

        /**
        * Sets registrar street1 part of address.
        * @param street1 sets part of address into @ref street1_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_street1(const std::string& street1);

        /**
        * Sets registrar street2 part of address.
        * @param street2 sets part of address into @ref street2_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_street2(const std::string& street2);

        /**
        * Sets registrar street3 part of address.
        * @param street3 sets part of address into @ref street3_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_street3(const std::string& street3);

        /**
        * Sets registrar city part of address.
        * @param city sets part of address - city into @ref city_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_city(const std::string& city);

        /**
        * Sets registrar region part of address.
        * @param stateorprovince sets part of address - region into @ref stateorprovince_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_stateorprovince(const std::string& stateorprovince);

        /**
        * Sets registrar postal code part of address.
        * @param postalcode sets part of address - postal code into @ref postalcode_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_postalcode(const std::string& postalcode);

        /**
        * Sets registrar country part of address.
        * @param country sets two character country code or country name into @ref country_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_country(const std::string& country);

        /**
        * Sets registrar telephone number.
        * @param telephone sets telephone number into @ref telephone_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_telephone(const std::string& telephone);

        /**
        * Sets registrar fax number.
        * @param fax sets fax number into @ref fax_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_fax(const std::string& fax);

        /**
        * Sets registrar e-mail address.
        * @param email sets e-mail address into @ref email_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_email(const std::string& email);

        /**
        * Sets registrar web address.
        * @param url sets web address into @ref url_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_url(const std::string& url);

        /**
        * Sets system registrar flag.
        * @param system sets system registrar flag into @ref system_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_system(bool system);

        /**
        * Sets registrar company registration number.
        * @param ico sets company registration number into @ref ico_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_ico(const std::string& ico);

        /**
        * Sets taxpayer identification number.
        * @param dic sets taxpayer identification number into @ref dic_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_dic(const std::string& dic);

        /**
        * Sets payments coupling tag.
        * @param variable_symbol sets payments coupling tag into @ref variable_symbol_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_variable_symbol(const std::string& variable_symbol);

        /**
        * Sets alternative payment coupling expression.
        * @param payment_memo_regex sets alternative payment coupling expression into @ref payment_memo_regex_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_payment_memo_regex(const std::string& payment_memo_regex);

        /**
        * Sets VAT payer flag.
        * @param vat_payer sets VAT payer flag into @ref vat_payer_ attribute
        * @return operation instance reference to allow method chaining
        */
        CreateRegistrar& set_vat_payer(bool vat_payer);

        /**
        * Executes create
        * @param ctx contains reference to database and logging interface
        */
        void exec(OperationContext& ctx);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

    };//CreateRegistrar
}
#endif // CREATE_REGISTRAR_H_
