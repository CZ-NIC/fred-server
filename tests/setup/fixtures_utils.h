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

#include <vector>
#include <utility>
#include <boost/static_assert.hpp>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

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

        Fred::CreateRegistrar make(const Optional<std::string>& _handle = Optional<std::string>()) {
            return Fred::CreateRegistrar(
                _handle.isset() ? _handle.get_value() : "REGISTRAR_" + RandomDataGenerator().xnumstring(20)
            );
        }
    };
    template<> struct CreateX_factory<Fred::CreateContact> {

        Fred::CreateContact make(
            const std::string& _registrar_handle,
            const Optional<std::string>& _handle = Optional<std::string>()
        ) {
            return Fred::CreateContact(
                _handle.isset() ? _handle.get_value() : "CONTACT_" + RandomDataGenerator().xnumstring(20),
                _registrar_handle
            );
        }

        // CreateX operation class is in fact non-assignable (due to const members). Ouch :-/ Welcome boost::ptr_vector.
        boost::ptr_vector<Fred::CreateContact> make_vector(
            const unsigned n,
            const std::string& _registrar_handle
        ) {
            boost::ptr_vector<Fred::CreateContact> result;
            for(unsigned i=0; i<n; ++i) {
                result.push_back(new Fred::CreateContact(make(_registrar_handle)));
            }

            return result;
        }
    };
    template<> struct CreateX_factory<Fred::CreateDomain> {

        Fred::CreateDomain make(
            const std::string& _registrar,
            const std::string& _registrant,
            const Optional<std::string>& _fqdn = Optional<std::string>()
        ) {
            return Fred::CreateDomain(
                _fqdn.isset() ? _fqdn.get_value() : RandomDataGenerator().xnumstring(20) + ".cz",  // TODO zone
                _registrar,
                _registrant
            );
        }
    };



    // for use with temporary object - copying arguments - suboptimal but hopefully adequate enough
    template<typename Tobject> Tobject fill_optional_data(Tobject obj);
    template<typename Tobject> boost::ptr_vector<Tobject>& fill_optional_data(boost::ptr_vector<Tobject>& obj_vec) {
        BOOST_FOREACH(Tobject& obj, obj_vec) {
            fill_optional_data(obj);
        }

        return obj_vec;
    }
    template<> inline Fred::CreateRegistrar fill_optional_data<>(Fred::CreateRegistrar obj) {
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
        obj.set_payment_memo_regex(".*");   // TODO - nejaky priklad
        obj.set_vat_payer(true);

        return obj;
    }
    template<> inline Fred::CreateContact fill_optional_data<>(Fred::CreateContact obj) {
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
        // TODO setup na ssntypes
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
    template<> inline Fred::CreateDomain fill_optional_data<>(Fred::CreateDomain obj) {
        RandomDataGenerator rnd;

        obj.set_authinfo(rnd.xnstring(15));

        return obj;
    }

    unsigned long long  get_nonexistent_contact_id(Fred::OperationContext& ctx);
    std::string         get_nonexistent_contact_handle(Fred::OperationContext& ctx);

    template<typename Tinfo> std::vector<std::string> get_handles_from_CreateXs(const std::vector<Tinfo>& objects) {
        std::vector<std::string> result;
        BOOST_FOREACH(const Tinfo& obj, objects) {
            result.push_back(obj.handle);
        }

        return result;
    }
    template<> inline std::vector<std::string> get_handles_from_CreateXs(const std::vector<Fred::InfoDomainData>& objects) {
        std::vector<std::string> result;
        BOOST_FOREACH(const Fred::InfoDomainData& obj, objects) {
            result.push_back(obj.fqdn);
        }

        return result;
    }


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

};

#endif // #include guard end
