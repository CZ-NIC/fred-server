/**
 *  @file
 *  test fixture utils
 */

#ifndef TESTS_SETUP_COMMON_FIXTURES_41215653023
#define TESTS_SETUP_COMMON_FIXTURES_41215653023

#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/random_data_generator.h"

#include <fredlib/registrar.h>
#include <fredlib/contact.h>
#include <fredlib/domain.h>
#include <fredlib/nsset.h>
#include <fredlib/keyset.h>

#include <vector>
#include <utility>
#include <boost/static_assert.hpp>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

    namespace util {
        // type relations - ugly XXX - may in fact hint at possible refactoring in fredlib

        template<typename TCreateOperOp> struct InfoXData_type;
        template<> struct InfoXData_type<Fred::CreateRegistrar> {
            typedef Fred::InfoRegistrarData type;
        };
        template<> struct InfoXData_type<Fred::CreateContact> {
            typedef Fred::InfoContactData type;
        };
        template<> struct InfoXData_type<Fred::CreateDomain> {
            typedef Fred::InfoDomainData type;
        };
        template<> struct InfoXData_type<Fred::CreateNsset> {
            typedef Fred::InfoNssetData type;
        };
        template<> struct InfoXData_type<Fred::CreateKeyset> {
            typedef Fred::InfoKeysetData type;
        };

        template<typename TCreateOperOp> struct InfoXByHandle_type;
        template<> struct InfoXByHandle_type<Fred::CreateRegistrar> {
            typedef Fred::InfoRegistrarByHandle type;
        };
        template<> struct InfoXByHandle_type<Fred::CreateContact> {
            typedef Fred::InfoContactByHandle type;
        };
        template<> struct InfoXByHandle_type<Fred::CreateDomain> {
            typedef Fred::InfoDomainByHandle type;
        };
        template<> struct InfoXByHandle_type<Fred::CreateNsset> {
            typedef Fred::InfoNssetByHandle type;
        };
        template<> struct InfoXByHandle_type<Fred::CreateKeyset> {
            typedef Fred::InfoKeysetByHandle type;
        };

        template<typename TInfoOutput, typename TInfoData> void copy_InfoXOutput_to_InfoXData(const TInfoOutput& in, TInfoData& out);
        template<> inline void copy_InfoXOutput_to_InfoXData<>(const Fred::InfoRegistrarOutput& in, Fred::InfoRegistrarData& out) {
            out = in.info_registrar_data;
        }
        template<> inline void copy_InfoXOutput_to_InfoXData<>(const Fred::InfoContactOutput& in, Fred::InfoContactData& out) {
            out = in.info_contact_data;
        }
        template<> inline void copy_InfoXOutput_to_InfoXData<>(const Fred::InfoDomainOutput& in, Fred::InfoDomainData& out) {
            out = in.info_domain_data;
        }
        template<> inline void copy_InfoXOutput_to_InfoXData<>(const Fred::InfoNssetOutput& in, Fred::InfoNssetData& out) {
            out = in.info_nsset_data;
        }
        template<> inline void copy_InfoXOutput_to_InfoXData<>(const Fred::InfoKeysetOutput& in, Fred::InfoKeysetData& out) {
            out = in.info_keyset_data;
        }

        // one of the least intrusive ways to get to member data is make those protected and derive...
        template<typename TCreateOper> struct get_handle_from_CreateX : public TCreateOper {
            // copy c'tor from base class
            explicit get_handle_from_CreateX(const TCreateOper& init)
            : TCreateOper(init)
            { }

            std::string operator()() {
                return this->handle_;
            }
        };
        template<> struct get_handle_from_CreateX<Fred::CreateDomain> : public Fred::CreateDomain {
            // copy c'tor from base class
            explicit get_handle_from_CreateX(const Fred::CreateDomain& init)
            : Fred::CreateDomain(init)
            { }

            std::string operator()() {
                return this->fqdn_;
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
    };



    template<typename TCreateOper> struct CreateX_factory;
    template<> struct CreateX_factory<Fred::CreateRegistrar> {
        typedef Fred::CreateRegistrar create_type;

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
    template<> struct CreateX_factory<Fred::CreateContact> {
        typedef Fred::CreateContact create_type;

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
    template<> struct CreateX_factory<Fred::CreateDomain> {
        typedef Fred::CreateDomain create_type;

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
    template<> struct CreateX_factory<Fred::CreateNsset> {
        typedef Fred::CreateNsset create_type;

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
    template<> struct CreateX_factory<Fred::CreateKeyset> {
        typedef Fred::CreateKeyset create_type;

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
    template<> inline Fred::CreateRegistrar generate_test_data<>(Fred::CreateRegistrar obj) {
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
    template<> inline Fred::CreateContact generate_test_data<>(Fred::CreateContact obj) {
        RandomDataGenerator rnd;

        obj.set_authinfo(rnd.xnstring(10));
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
    template<> inline Fred::CreateDomain generate_test_data<>(Fred::CreateDomain obj) {
        RandomDataGenerator rnd;

        obj.set_authinfo(rnd.xnstring(15));

        return obj;
    }
    template<> inline Fred::CreateNsset generate_test_data<>(Fred::CreateNsset obj) {
        RandomDataGenerator rnd;

        obj.set_authinfo(rnd.xnstring(15));
        obj.set_dns_hosts(
            boost::assign::list_of(
                Fred::DnsHost(
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
    template<> inline Fred::CreateKeyset generate_test_data<>(Fred::CreateKeyset obj) {
        RandomDataGenerator rnd;

        obj.set_authinfo(rnd.xnstring(15));
        obj.set_dns_keys(
            boost::assign::list_of(Fred::DnsKey(1, 1, 1, "abcde"))
        );

        return obj;
    }

    unsigned long long  get_nonexistent_object_id(Fred::OperationContext& ctx);
    unsigned long long  get_nonexistent_object_historyid(Fred::OperationContext& ctx);
    std::string         get_nonexistent_object_handle(Fred::OperationContext& ctx);
    unsigned long long  get_nonexistent_registrar_id(Fred::OperationContext& ctx);

    // for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
    template<typename TCreateOper> typename util::InfoXData_type<TCreateOper>::type exec(TCreateOper create, Fred::OperationContext& ctx) {
        create.exec(ctx);

        typename util::InfoXData_type<TCreateOper>::type temp;
        util::copy_InfoXOutput_to_InfoXData(
            typename util::InfoXByHandle_type<TCreateOper>::type(
                util::get_handle_from_CreateX<TCreateOper>(create)()
            ).exec(ctx),
            temp
        );

        return temp;
    }
    // for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
    template<typename TCreateOper> std::vector<typename util::InfoXData_type<TCreateOper>::type> exec(boost::ptr_vector<TCreateOper> objects, Fred::OperationContext& ctx) {
        std::vector<typename util::InfoXData_type<TCreateOper>::type> result;
        BOOST_FOREACH(const TCreateOper& obj, objects) {
            result.push_back(exec(obj, ctx));
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
            exec(Fred::OperationContext& ctx);
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
            exec(Fred::OperationContext& ctx);
    };



    struct registrar {
        Fred::InfoRegistrarData info_data;

        static Fred::InfoRegistrarData make(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>() ) {
            return exec(
                CreateX_factory<Fred::CreateRegistrar>().make(_handle),
                _ctx
            );
        }

        registrar(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>()) {
            info_data = make(_ctx, _handle);
        }

        registrar(Optional<std::string> _handle = Optional<std::string>()) {
            Fred::OperationContext ctx;
            info_data = make(ctx, _handle);
            ctx.commit_transaction();
        }
    };

    struct contact {
        Fred::InfoContactData info_data;

        static Fred::InfoContactData make(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            if(_registrar_handle.isset()) {
                return exec(
                    CreateX_factory<Fred::CreateContact>().make(
                        _registrar_handle.get_value_or_default(),
                        _handle
                    ),
                    _ctx
                );
            } else {
                return exec(
                    CreateX_factory<Fred::CreateContact>().make(
                        registrar(_ctx).info_data.handle,
                        _handle
                    ),
                    _ctx
                );
            }
        }

        contact(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            info_data = make(_ctx, _handle, _registrar_handle);
        }

        contact(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            Fred::OperationContext ctx;
            info_data = make(ctx, _handle, _registrar_handle);
            ctx.commit_transaction();
        }
    };

    struct domain {
        Fred::InfoDomainData info_data;

        static Fred::InfoDomainData make(Fred::OperationContext& _ctx) {
            return exec(
                CreateX_factory<Fred::CreateDomain>().make(
                    registrar(_ctx).info_data.handle,
                    contact(_ctx).info_data.handle
                ),
                _ctx
            );
        }

        domain(Fred::OperationContext& _ctx) {
            info_data = make(_ctx);
        }

        domain() {
            Fred::OperationContext ctx;
            info_data = make(ctx);
            ctx.commit_transaction();
        }
    };

    struct nsset {
        Fred::InfoNssetData info_data;

        static Fred::InfoNssetData make(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            if(_registrar_handle.isset()) {
                return exec(
                    CreateX_factory<Fred::CreateNsset>().make(
                        _registrar_handle.get_value_or_default(),
                        _handle
                    ),
                    _ctx
                );
            } else {
                return exec(
                    CreateX_factory<Fred::CreateNsset>().make(
                        registrar(_ctx).info_data.handle,
                        _handle
                    ),
                    _ctx
                );
            }
        }

        nsset(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            info_data = make(_ctx, _handle, _registrar_handle);
        }

        nsset(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            Fred::OperationContext ctx;
            info_data = make(ctx, _handle, _registrar_handle);
            ctx.commit_transaction();
        }
    };

    struct keyset {
        Fred::InfoKeysetData info_data;

        static Fred::InfoKeysetData make(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            if(_registrar_handle.isset()) {
                return exec(
                    CreateX_factory<Fred::CreateKeyset>().make(
                        _registrar_handle.get_value_or_default(),
                        _handle
                    ),
                    _ctx
                );
            } else {
                return exec(
                    CreateX_factory<Fred::CreateKeyset>().make(
                        registrar(_ctx).info_data.handle,
                        _handle
                    ),
                    _ctx
                );
            }
        }

        keyset(Fred::OperationContext& _ctx, Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            info_data = make(_ctx, _handle, _registrar_handle);
        }

        keyset(Optional<std::string> _handle = Optional<std::string>(), Optional<std::string> _registrar_handle = Optional<std::string>()) {
            Fred::OperationContext ctx;
            info_data = make(ctx, _handle, _registrar_handle);
            ctx.commit_transaction();
        }
    };
};

#endif // #include guard end
