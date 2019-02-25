/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
 *  test fixture utils
 */

#ifndef FIXTURES_UTILS_HH_AFF603B566B44E78A5A215F9112A2040
#define FIXTURES_UTILS_HH_AFF603B566B44E78A5A215F9112A2040

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrable_object/keyset/check_keyset.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/delete_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/keyset/update_keyset.hh"
#include "libfred/registrable_object/nsset/check_nsset.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/delete_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/nsset/update_nsset.hh"
#include "libfred/registrar/check_registrar.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"
#include "util/random_data_generator.hh"
#include "src/util/tz/europe/prague.hh"
#include "src/util/tz/get_psql_handle_of.hh"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/static_assert.hpp>

#include <utility>
#include <vector>

namespace Test {

namespace util {
    // type relations - ugly XXX - may in fact hint at possible refactoring in fredlib

    template<typename TCreateOperOp> struct InfoXData_type;
    template<> struct InfoXData_type<::LibFred::CreateRegistrar> {
        typedef ::LibFred::InfoRegistrarData type;
    };
    template<> struct InfoXData_type<::LibFred::CreateContact> {
        typedef ::LibFred::InfoContactData type;
    };
    template<> struct InfoXData_type<::LibFred::CreateDomain> {
        typedef ::LibFred::InfoDomainData type;
    };
    template<> struct InfoXData_type<::LibFred::CreateNsset> {
        typedef ::LibFred::InfoNssetData type;
    };
    template<> struct InfoXData_type<::LibFred::CreateKeyset> {
        typedef ::LibFred::InfoKeysetData type;
    };

    template<typename TCreateOperOp> struct InfoXByHandle_type;
    template<> struct InfoXByHandle_type<::LibFred::CreateRegistrar> {
        typedef ::LibFred::InfoRegistrarByHandle type;
    };
    template<> struct InfoXByHandle_type<::LibFred::CreateContact> {
        typedef ::LibFred::InfoContactByHandle type;
    };
    template<> struct InfoXByHandle_type<::LibFred::CreateDomain> {
        typedef ::LibFred::InfoDomainByFqdn type;
    };
    template<> struct InfoXByHandle_type<::LibFred::CreateNsset> {
        typedef ::LibFred::InfoNssetByHandle type;
    };
    template<> struct InfoXByHandle_type<::LibFred::CreateKeyset> {
        typedef ::LibFred::InfoKeysetByHandle type;
    };

    template<typename TInfoOutput, typename TInfoData> void copy_InfoXOutput_to_InfoXData(const TInfoOutput& in, TInfoData& out);
    template<> inline void copy_InfoXOutput_to_InfoXData<>(const ::LibFred::InfoRegistrarOutput& in, ::LibFred::InfoRegistrarData& out) {
        out = in.info_registrar_data;
    }
    template<> inline void copy_InfoXOutput_to_InfoXData<>(const ::LibFred::InfoContactOutput& in, ::LibFred::InfoContactData& out) {
        out = in.info_contact_data;
    }
    template<> inline void copy_InfoXOutput_to_InfoXData<>(const ::LibFred::InfoDomainOutput& in, ::LibFred::InfoDomainData& out) {
        out = in.info_domain_data;
    }
    template<> inline void copy_InfoXOutput_to_InfoXData<>(const ::LibFred::InfoNssetOutput& in, ::LibFred::InfoNssetData& out) {
        out = in.info_nsset_data;
    }
    template<> inline void copy_InfoXOutput_to_InfoXData<>(const ::LibFred::InfoKeysetOutput& in, ::LibFred::InfoKeysetData& out) {
        out = in.info_keyset_data;
    }

    // one of the least intrusive ways to get to member data is make those protected and derive...
    template <typename TCreateOper>
    struct get_handle_from_CreateX : public TCreateOper
    {
        // copy c'tor from base class
        explicit get_handle_from_CreateX(const TCreateOper& init)
        : TCreateOper(init)
        { }

        const std::string& operator()()
        {
            return this->TCreateOper::get_handle();
        }
    };
    template <>
    struct get_handle_from_CreateX<::LibFred::CreateDomain> : public ::LibFred::CreateDomain
    {
        // copy c'tor from base class
        explicit get_handle_from_CreateX(const ::LibFred::CreateDomain& init)
        : ::LibFred::CreateDomain(init)
        { }

        const std::string& operator()()
        {
            return this->::LibFred::CreateDomain::get_fqdn();
        }
    };

    // base class with aggregating methods
    template<typename Tdata, typename Tderived> class aggregator {
        protected:
            std::vector<Tdata> values_;

        public:

            aggregator() { }

            aggregator(const Tdata& _val) {
                values_.push_back(_val);
            }

            virtual ~aggregator() { }

            Tderived& add(const Tdata& _val) {
                values_.push_back(_val);

                return static_cast<Tderived&>(*this);
            }
            Tderived& add(const std::vector<Tdata>& _values) {
                values_.insert(values_.end(), _values.begin(), _values.end());

                return static_cast<Tderived&>(*this);
            }
    };
}//namespace Test::util

template<typename TCreateOper> struct CreateX_factory;
template<> struct CreateX_factory<::LibFred::CreateRegistrar> {
    typedef ::LibFred::CreateRegistrar create_type;

    create_type make(const Optional<std::string>& _handle = Optional<std::string>()) {
        return create_type(
            _handle.isset() ? _handle.get_value() : "REGISTRAR_" + RandomDataGenerator().xnumstring(20)
        );
    }

    // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
    boost::ptr_vector<create_type> make_vector(
        const unsigned n
    ) {
        boost::ptr_vector<create_type> result;
        for(unsigned i=0; i<n; ++i) {
            result.push_back(new create_type(make()));
        }

        return result;
    }
};
template<> struct CreateX_factory<::LibFred::CreateContact> {
    typedef ::LibFred::CreateContact create_type;

    create_type make(
        const std::string& _registrar_handle,
        const Optional<std::string>& _handle = Optional<std::string>()
    ) {
        return create_type(
            _handle.isset() ? _handle.get_value() : "CONTACT_" + RandomDataGenerator().xnumstring(20),
            _registrar_handle
        );
    }

    // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
    boost::ptr_vector<create_type> make_vector(
        const unsigned n,
        const std::string& _registrar_handle
    ) {
        boost::ptr_vector<create_type> result;
        for(unsigned i=0; i<n; ++i) {
            result.push_back(new create_type(make(_registrar_handle)));
        }

        return result;
    }
};
template<> struct CreateX_factory<::LibFred::CreateDomain> {
    typedef ::LibFred::CreateDomain create_type;

    create_type make(
        const std::string& _registrar,
        const std::string& _registrant,
        const Optional<std::string>& _fqdn = Optional<std::string>()
    ) {
        return create_type(
            _fqdn.isset() ? _fqdn.get_value() : RandomDataGenerator().xnumstring(20) + ".cz",  // TODO zavisle na existenci .cz zony, coz bychom casem mohli odstranit
            _registrar,
            _registrant
        );
    }

    // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
    boost::ptr_vector<create_type> make_vector(
        const unsigned n,
        const std::string& _registrar,
        const std::string& _registrant
    ) {
        boost::ptr_vector<create_type> result;
        for(unsigned i=0; i<n; ++i) {
            result.push_back(new create_type(make(_registrar, _registrant)));
        }

        return result;
    }
};
template<> struct CreateX_factory<::LibFred::CreateNsset> {
    typedef ::LibFred::CreateNsset create_type;

    create_type make(
        const std::string& _registrar,
        const Optional<std::string>& _handle = Optional<std::string>()
    ) {
        return create_type(
            _handle.isset() ? _handle.get_value() : "NSSET_" + RandomDataGenerator().xnumstring(20),
            _registrar
        );
    }

    // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
    boost::ptr_vector<create_type> make_vector(
        const unsigned n,
        const std::string& _registrar
    ) {
        boost::ptr_vector<create_type> result;
        for(unsigned i=0; i<n; ++i) {
            result.push_back(new create_type(make(_registrar)));
        }

        return result;
    }
};
template<> struct CreateX_factory<::LibFred::CreateKeyset> {
    typedef ::LibFred::CreateKeyset create_type;

    create_type make(
        const std::string& _registrar,
        const Optional<std::string>& _handle = Optional<std::string>()
    ) {
        return create_type(
            _handle.isset() ? _handle.get_value() : "KEYSET_" + RandomDataGenerator().xnumstring(20),
            _registrar
        );
    }

    // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
    boost::ptr_vector<create_type> make_vector(
        const unsigned n,
        const std::string& _registrar
    ) {
        boost::ptr_vector<create_type> result;
        for(unsigned i=0; i<n; ++i) {
            result.push_back(new create_type(make(_registrar)));
        }

        return result;
    }
};


// for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
template<typename Tobject> Tobject generate_test_data(Tobject obj);
template<typename Tobject> boost::ptr_vector<Tobject>& generate_test_data(boost::ptr_vector<Tobject>& obj_vec) {
    BOOST_FOREACH(Tobject& obj, obj_vec) {
        generate_test_data(obj);
    }

    return obj_vec;
}
template<> inline ::LibFred::CreateRegistrar generate_test_data<>(::LibFred::CreateRegistrar obj) {
    RandomDataGenerator rnd;

    obj.set_name("Jan " + rnd.xnstring(7) + " Novak");
    obj.set_organization("Zakoupil a Zboril a " + rnd.xnstring(7) + " s. r. o.");
    obj.set_street1("Na rynku " + rnd.xnumstring(3) + "/" + rnd.xnumstring(2));
    obj.set_street2(rnd.xnumstring(1) + ". patro");
    obj.set_street3(rnd.xnumstring(1) + ". dvere vlevo");
    obj.set_city("Praha " + rnd.xnumstring(1));
    obj.set_stateorprovince("Kraj c." + rnd.xnumstring(2));
    obj.set_postalcode(rnd.xnumstring(3) + " " + rnd.xnumstring(2));
    static const std::string country("CZ");
    obj.set_country(country);
    obj.set_telephone("+" + rnd.xnumstring(3) + " " + rnd.xnumstring(9));
    obj.set_fax("+" + rnd.xnumstring(3) + " " + rnd.xnumstring(9));
    obj.set_email(rnd.xnstring(7) + "@" + rnd.xnstring(7) + "." + country);
    obj.set_url("www." + rnd.xnstring(20) + "." + country);
    obj.set_system(true);
    const std::string ico(rnd.xnumstring(8));
    obj.set_ico(ico);
    obj.set_dic(country + ico);
    obj.set_variable_symbol(rnd.xnumstring(6));
    obj.set_payment_memo_regex(".*");   // TODO - casem mozno doplnit realnejsi exemplar
    obj.set_vat_payer(true);

    return obj;
}
template<> inline ::LibFred::CreateContact generate_test_data<>(::LibFred::CreateContact obj) {
    RandomDataGenerator rnd;

    ::LibFred::Contact::PlaceAddress place;
    place.street1 = std::string("Na rynku ") + rnd.xnumstring(3) + "/" + rnd.xnumstring(2);
    place.street2 = rnd.xnumstring(1) + ". patro";
    place.street3 = rnd.xnumstring(1) + ". dvere vlevo";
    place.city = std::string("Praha ") + rnd.xnumstring(1);
    place.stateorprovince = std::string("Kraj c.") + rnd.xnumstring(2);
    place.postalcode = rnd.xnumstring(3) + " " + rnd.xnumstring(2);
    static const std::string country("CZ");
    place.country = country;

    obj.set_authinfo(rnd.xnstring(10));
    obj.set_name("Jan " + rnd.xnstring(7) + " Novak");
    obj.set_organization("Zakoupil a Zboril a " + rnd.xnstring(7) + " s. r. o.");
    obj.set_place(place);
    obj.set_telephone("+" + rnd.xnumstring(3) + " " + rnd.xnumstring(9));
    obj.set_fax("+" + rnd.xnumstring(3) + " " + rnd.xnumstring(9));
    const std::string email(rnd.xnstring(7) + "@" + rnd.xnstring(7) + "." + country);
    obj.set_email(email);
    obj.set_notifyemail(rnd.xnumstring(1) + email);
    obj.set_vat(country + rnd.xnumstring(8));
    obj.set_ssntype("BIRTHDAY");
    obj.set_ssn(rnd.xnumstring(4) + "-" + rnd.xnumstring(2) + "-" + rnd.xnumstring(2));
    obj.set_disclosename(true);
    obj.set_discloseorganization(true);
    obj.set_discloseaddress(true);
    obj.set_disclosetelephone(true);
    obj.set_disclosefax(true);
    obj.set_discloseemail(true);
    obj.set_disclosevat(true);
    obj.set_discloseident(true);
    obj.set_disclosenotifyemail(true);
    obj.set_logd_request_id(rnd.xuint());

    return obj;
}
template<> inline ::LibFred::CreateDomain generate_test_data<>(::LibFred::CreateDomain obj) {
    RandomDataGenerator rnd;

    obj.set_authinfo(rnd.xnstring(15));

    return obj;
}
template<> inline ::LibFred::CreateNsset generate_test_data<>(::LibFred::CreateNsset obj) {
    RandomDataGenerator rnd;

    obj.set_authinfo(rnd.xnstring(15));
    obj.set_dns_hosts(
        boost::assign::list_of(
            ::LibFred::DnsHost(
                "a.b.c.de",
                boost::assign::list_of(
                    boost::asio::ip::address::from_string("1.2.3.4")
                )
            )
        )
    );
    obj.set_tech_check_level(1);

    return obj;
}
template<> inline ::LibFred::CreateKeyset generate_test_data<>(::LibFred::CreateKeyset obj) {
    RandomDataGenerator rnd;

    obj.set_authinfo(rnd.xnstring(15));
    obj.set_dns_keys(
        boost::assign::list_of(::LibFred::DnsKey(1, 1, 3, "abcde"))
    );

    return obj;
}

unsigned long long generate_random_bigserial();
std::string generate_random_handle();

/** @returns a value not used in object_registry.id. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
unsigned long long  get_nonexistent_object_id(::LibFred::OperationContext& ctx);
/** @returns a value not used in object_history.historyid. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
unsigned long long  get_nonexistent_object_historyid(::LibFred::OperationContext& ctx);
/** @returns a value not used in message.id. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
unsigned long long get_nonexistent_message_id(::LibFred::OperationContext& ctx);
/** @returns a value not used in object_registry.handle. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
std::string         get_nonexistent_object_handle(::LibFred::OperationContext& ctx);
/** @returns a value not used in registrar.id. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
unsigned long long  get_nonexistent_registrar_id(::LibFred::OperationContext& ctx);
/** @returns a value not used in zone.id. No other qualities (e.g. being constant, non-repeating, ...) are guaranteed. */
unsigned long long  get_nonexistent_zone_id(::LibFred::OperationContext& ctx);

template<typename T> T get_nonexistent_value(
    ::LibFred::OperationContext& ctx,
    const std::string& table,
    const std::string& column,
    const std::string& postgres_type,
    T (*random_generator) ()
) {
    T result;

    Database::Result check;
    // guarantee non-existence
    do {
        ::LibFred::OperationContextCreator ctx;
        result = random_generator();
        check = ctx.get_conn().exec_params(
            "SELECT "+ column +" "
                "FROM "+ table +" "
                "WHERE "+ column +"=$1::"+postgres_type,
                Database::query_param_list(result) );
    } while(check.size() != 0);

    return result;
}

// TODO XXX - hopefully one day we break the dependency on cz zone creation before running tests
unsigned long long  get_cz_zone_id(::LibFred::OperationContext& ctx);

// for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
template<typename TCreateOper> typename util::InfoXData_type<TCreateOper>::type exec(TCreateOper create, ::LibFred::OperationContext& ctx, const std::string& timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
    create.exec(ctx);

    typename util::InfoXData_type<TCreateOper>::type temp;
    util::copy_InfoXOutput_to_InfoXData(
        typename util::InfoXByHandle_type<TCreateOper>::type(
            util::get_handle_from_CreateX<TCreateOper>(create)()
            ).exec(ctx, timezone),
        temp
    );

    return temp;
}
// for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
template<typename TCreateOper> std::vector<typename util::InfoXData_type<TCreateOper>::type> exec(boost::ptr_vector<TCreateOper> objects, ::LibFred::OperationContext& ctx, const std::string& timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
    std::vector<typename util::InfoXData_type<TCreateOper>::type> result;
    BOOST_FOREACH(const TCreateOper& obj, objects) {
        result.push_back(exec(obj, ctx, timezone));
    }

    return result;
}



class add_admin_contacts_to_domain
    : public util::aggregator<std::string, add_admin_contacts_to_domain>
{
    private:
        std::string domain_handle_;
        std::string registrar_handle_;

    public:
        add_admin_contacts_to_domain(
            const std::string& _domain_handle,
            const std::string& _registrar_handle
        );

        std::pair<
            std::string,
            std::vector<std::string>
        >
        exec(::LibFred::OperationContext& ctx);
};

class add_admin_contact_to_domains
    : public util::aggregator<std::string, add_admin_contact_to_domains>
{
    private:
        std::string contact_handle_;
        std::string registrar_handle_;

    public:
        add_admin_contact_to_domains(
            const std::string& _contact_handle,
            const std::string& _registrar_handle
        );

        std::pair<
            std::string,
            std::vector<std::string>
        >
        exec(::LibFred::OperationContext& ctx);
};



struct registrar {
    ::LibFred::InfoRegistrarData info_data;

    static ::LibFred::InfoRegistrarData make(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        return exec(
            CreateX_factory<::LibFred::CreateRegistrar>().make(_handle),
            _ctx,
            _timezone
        );
    }

    registrar(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        info_data = make(_ctx, _handle, _timezone);
    }

    registrar(Optional<std::string> _handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        ::LibFred::OperationContextCreator ctx;
        info_data = make(ctx, _handle, _timezone);
        ctx.commit_transaction();
    }
};

struct contact {
    ::LibFred::InfoContactData info_data;

    static ::LibFred::InfoContactData make(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        if(_registrar_handle.isset()) {
            return exec(
                CreateX_factory<::LibFred::CreateContact>().make(
                    _registrar_handle.get_value_or_default(),
                    _handle
                ),
                _ctx,
                _timezone
            );
        } else {
            return exec(
                CreateX_factory<::LibFred::CreateContact>().make(
                    registrar(_ctx).info_data.handle,
                    _handle
                ),
                _ctx,
                _timezone
            );
        }
    }

    contact(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        info_data = make(_ctx, _handle, _registrar_handle, _timezone);
    }

    contact(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        ::LibFred::OperationContextCreator ctx;
        info_data = make(ctx, _handle, _registrar_handle, _timezone);
        ctx.commit_transaction();
    }
};

struct domain {
    ::LibFred::InfoDomainData info_data;

    static ::LibFred::InfoDomainData make(::LibFred::OperationContext& _ctx, const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        return exec(
            CreateX_factory<::LibFred::CreateDomain>().make(
                registrar(_ctx).info_data.handle,
                contact(_ctx).info_data.handle
            ),
            _ctx,
            _timezone
        );
    }

    domain(::LibFred::OperationContext& _ctx, const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        info_data = make(_ctx, _timezone);
    }

    domain(const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        ::LibFred::OperationContextCreator ctx;
        info_data = make(ctx, _timezone);
        ctx.commit_transaction();
    }
};

struct nsset {
    ::LibFred::InfoNssetData info_data;

    static ::LibFred::InfoNssetData make(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        if(_registrar_handle.isset()) {
            return exec(
                CreateX_factory<::LibFred::CreateNsset>().make(
                    _registrar_handle.get_value_or_default(),
                    _handle
                ),
                _ctx,
                _timezone
            );
        } else {
            return exec(
                CreateX_factory<::LibFred::CreateNsset>().make(
                    registrar(_ctx).info_data.handle,
                    _handle
                ),
                _ctx,
                _timezone
            );
        }
    }

    nsset(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        info_data = make(_ctx, _handle, _registrar_handle, _timezone);
    }

    nsset(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        ::LibFred::OperationContextCreator ctx;
        info_data = make(ctx, _handle, _registrar_handle, _timezone);
        ctx.commit_transaction();
    }
};

struct keyset {
    ::LibFred::InfoKeysetData info_data;

    static ::LibFred::InfoKeysetData make(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        if(_registrar_handle.isset()) {
            return exec(
                CreateX_factory<::LibFred::CreateKeyset>().make(
                    _registrar_handle.get_value_or_default(),
                    _handle
                ),
                _ctx,
                _timezone
            );
        } else {
            return exec(
                CreateX_factory<::LibFred::CreateKeyset>().make(
                    registrar(_ctx).info_data.handle,
                    _handle
                ),
                _ctx,
                _timezone
            );
        }
    }

    keyset(::LibFred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        info_data = make(_ctx, _handle, _registrar_handle, _timezone);
    }

    keyset(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>(), const std::string& _timezone = Tz::get_psql_handle_of<Tz::Europe::Prague>()) {
        ::LibFred::OperationContextCreator ctx;
        info_data = make(ctx, _handle, _registrar_handle, _timezone);
        ctx.commit_transaction();
    }
};

}//namespace Test

#endif
