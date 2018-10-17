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


#include <string>
#include <vector>
#include <sstream>

#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrable_object/contact/contact_enum.hh"

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"
#include "src/util/util.hh"

namespace LibFred
{

    CreateRegistrar::CreateRegistrar(const std::string& handle)
    : handle_(handle)
    {}

    CreateRegistrar::CreateRegistrar(const std::string& handle,
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
        const Optional<bool>& vat_payer)
    : handle_(handle)
    , name_(name)
    , organization_(organization)
    , street1_(street1)
    , street2_(street2)
    , street3_(street3)
    , city_(city)
    , stateorprovince_(stateorprovince)
    , postalcode_(postalcode)
    , country_(country)
    , telephone_(telephone)
    , fax_(fax)
    , email_(email)
    , url_(url)
    , system_(system)
    , ico_(ico)
    , dic_(dic)
    , variable_symbol_(variable_symbol)
    , payment_memo_regex_(payment_memo_regex)
    , vat_payer_(vat_payer)
    {}

    CreateRegistrar& CreateRegistrar::set_name(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_organization(const std::string& organization)
    {
        organization_ = organization;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_street1(const std::string& street1)
    {
        street1_ = street1;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_street2(const std::string& street2)
    {
        street2_ = street2;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_street3(const std::string& street3)
    {
        street3_ = street3;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_city(const std::string& city)
    {
        city_ = city;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_stateorprovince(const std::string& stateorprovince)
    {
        stateorprovince_ = stateorprovince;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_postalcode(const std::string& postalcode)
    {
        postalcode_ = postalcode;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_country(const std::string& country)
    {
        country_ = country;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_telephone(const std::string& telephone)
    {
        telephone_ = telephone;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_fax(const std::string& fax)
    {
        fax_ = fax;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_email(const std::string& email)
    {
        email_ = email;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_url(const std::string& url)
    {
        url_ = url;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_system(bool system)
    {
        system_ = system;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_ico(const std::string& ico)
    {
        ico_ = ico;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_dic(const std::string& dic)
    {
        dic_ = dic;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_variable_symbol(const std::string& variable_symbol)
    {
        variable_symbol_ = variable_symbol;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_payment_memo_regex(const std::string& payment_memo_regex)
    {
        payment_memo_regex_ = payment_memo_regex;
        return *this;
    }

    CreateRegistrar& CreateRegistrar::set_vat_payer(bool vat_payer)
    {
        vat_payer_ = vat_payer;
        return *this;
    }

    unsigned long long CreateRegistrar::exec(OperationContext& ctx)
    {
        try
        {
            //create registrar
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO registrar (";
                val_sql << " VALUES (";

                params.push_back(handle_);
                col_sql << col_separator.get() << "handle";
                val_sql << val_separator.get() << "UPPER($" << params.size() <<"::text)";

                if (name_.isset())
                {
                    params.push_back(name_.get_value());
                    col_sql << col_separator.get() << "name";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (organization_.isset())
                {
                    params.push_back(organization_.get_value());
                    col_sql << col_separator.get() << "organization";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (street1_.isset())
                {
                    params.push_back(street1_.get_value());
                    col_sql << col_separator.get() << "street1";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (street2_.isset())
                {
                    params.push_back(street2_.get_value());
                    col_sql << col_separator.get() << "street2";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (street3_.isset())
                {
                    params.push_back(street3_.get_value());
                    col_sql << col_separator.get() << "street3";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (city_.isset())
                {
                    params.push_back(city_.get_value());
                    col_sql << col_separator.get() << "city";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (stateorprovince_.isset())
                {
                    params.push_back(stateorprovince_.get_value());
                    col_sql << col_separator.get() << "stateorprovince";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (postalcode_.isset())
                {
                    params.push_back(postalcode_.get_value());
                    col_sql << col_separator.get() << "postalcode";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (country_.isset())
                {
                    params.push_back(LibFred::Contact::get_country_code(country_, ctx,
                            static_cast<Exception*>(0), &Exception::set_unknown_country));//throw if country unknown
                    col_sql << col_separator.get() << "country";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (telephone_.isset())
                {
                    params.push_back(telephone_.get_value());
                    col_sql << col_separator.get() << "telephone";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (fax_.isset())
                {
                    params.push_back(fax_.get_value());
                    col_sql << col_separator.get() << "fax";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (email_.isset())
                {
                    params.push_back(email_.get_value());
                    col_sql << col_separator.get() << "email";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (url_.isset())
                {
                    params.push_back(url_.get_value());
                    col_sql << col_separator.get() << "url";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (system_.isset())
                {
                    params.push_back(system_.get_value());
                    col_sql << col_separator.get() << "system";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                if (ico_.isset())
                {
                    params.push_back(ico_.get_value());
                    col_sql << col_separator.get() << "ico";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (dic_.isset())
                {
                    params.push_back(dic_.get_value());
                    col_sql << col_separator.get() << "dic";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (variable_symbol_.isset())
                {
                    params.push_back(variable_symbol_.get_value());
                    col_sql << col_separator.get() << "varsymb";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (payment_memo_regex_.isset())
                {
                    params.push_back(payment_memo_regex_.get_value());
                    col_sql << col_separator.get() << "regex";
                    val_sql << val_separator.get() << "$" << params.size() <<"::text";
                }

                if (vat_payer_.isset())
                {
                    params.push_back(vat_payer_.get_value());
                    col_sql << col_separator.get() << "vat";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }

                col_sql <<")";
                val_sql << ") RETURNING id";

                //insert into registrar
                try
                {
                    Database::Result registrar_result = ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);

                    if (registrar_result.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(LibFred::InternalError("registrar creation failed"));
                    }
                    const auto id = static_cast<unsigned long long>(registrar_result[0][0]);
                    return id;
                }
                catch(const std::exception& ex)
                {
                    std::string what_string(ex.what());
                    if(what_string.find("registrar_handle_key") != std::string::npos)
                    {
                        BOOST_THROW_EXCEPTION(Exception().set_invalid_registrar_handle(handle_));
                    }
                    else
                        throw;
                }
            }//create registrar

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string CreateRegistrar::to_string() const
    {
        return Util::format_operation_state("CreateRegistrar",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("name",name_.print_quoted()))
        (std::make_pair("organization",organization_.print_quoted()))
        (std::make_pair("street1",street1_.print_quoted()))
        (std::make_pair("street2",street2_.print_quoted()))
        (std::make_pair("street3",street3_.print_quoted()))
        (std::make_pair("city",city_.print_quoted()))
        (std::make_pair("stateorprovince",stateorprovince_.print_quoted()))
        (std::make_pair("postalcode",postalcode_.print_quoted()))
        (std::make_pair("country",country_.print_quoted()))
        (std::make_pair("telephone",telephone_.print_quoted()))
        (std::make_pair("fax",fax_.print_quoted()))
        (std::make_pair("email",email_.print_quoted()))
        (std::make_pair("url",url_.print_quoted()))
        (std::make_pair("system",system_.print_quoted()))
        (std::make_pair("ico",ico_.print_quoted()))
        (std::make_pair("dic",dic_.print_quoted()))
        (std::make_pair("variable_symbol",variable_symbol_.print_quoted()))
        (std::make_pair("payment_memo_regex",payment_memo_regex_.print_quoted()))
        (std::make_pair("vat_payer",vat_payer_.print_quoted()))
        );
    }

} // namespace LibFred

