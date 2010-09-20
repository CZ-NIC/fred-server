#include "mojeid_impl.h"
#include "corba_wrap.h"

#include "log/logger.h"
#include "log/context.h"

#include "register/db_settings.h"
#include "register/contact.h"
#include "corba/connection_releaser.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



boost::mt19937 gen;

const std::string create_ctx_name(const std::string &_name)
{
    boost::uniform_int<unsigned int> ruint(std::numeric_limits<unsigned int>::min(),
                                           std::numeric_limits<unsigned int>::max());
    boost::variate_generator<boost::mt19937&, boost::uniform_int<unsigned int> > gen_uint(gen, ruint);

    return str(boost::format("<%2%> %1%") % _name % gen_uint());
}


namespace Registry {


MojeIDImpl::MojeIDImpl(const std::string &_server_name)
    : server_name_(_server_name)
{
}


MojeIDImpl::~MojeIDImpl()
{
}


CORBA::ULongLong MojeIDImpl::contactCreate(const Contact &_contact)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("contact-create"));
    ConnectionReleaser releaser;

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        Register::Contact::ManagerPtr contact_mgr(Register::Contact::Manager::create(0, false));

    }
    catch (std::exception &_ex) {
    }
    catch (...) {
    }

    return 0;
}


void MojeIDImpl::contactUpdatePrepare(const Contact &_contact,
                                                  const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("contact-update"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
}


Contact* MojeIDImpl::contactInfo(const char* _handle)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("contact-info"));
    ConnectionReleaser releaser;

    try {
        Database::Connection conn = Database::Manager::acquire();

        std::string qinfo = "SELECT oreg.id, oreg.name,"
            " SPLIT_PART(c.name, ' ', 1) as first_name,"
            " SPLIT_PART(c.name, ' ', 2) as last_name,"
            " c.organization, c.vat, c.ssntype, c.ssn,"
            " c.disclosename, c.discloseorganization,"
            " c.disclosevat, c.discloseident,"
            " c.discloseemail, c.disclosenotifyemail,"
            " c.discloseaddress, c.disclosetelephone,"
            " c.disclosefax,"
            " c.street1, c.street2, c.street3,"
            " c.city, c.stateorprovince, c.postalcode, c.country,"
            " c.email, c.notifyemail, c.telephone, c.fax"
            " FROM object_registry oreg JOIN contact c ON c.id = oreg.id"
            " WHERE oreg.name = UPPER($1::text) AND oreg.erdate IS NULL";
        Database::QueryParams pinfo = Database::query_param_list(_handle);

        Database::Result rinfo = conn.exec_params(qinfo, pinfo);
        if (!rinfo.size()) {
            throw std::runtime_error("not found");
        }

        Contact *data = new Contact();
        data->username     = corba_wrap_string(rinfo[0][1]);
        data->first_name   = corba_wrap_string(rinfo[0][2]);
        data->last_name    = corba_wrap_string(rinfo[0][3]);
        data->organization = corba_wrap_nullable_string(rinfo[0][4]);
        data->vat_reg_num  = corba_wrap_nullable_string(rinfo[0][5]);
        data->ssn_type     = corba_wrap_nullable_string(rinfo[0][6]);

        int ident_type = static_cast<int>(rinfo[0][6]);
        data->id_card_num  = ident_type == 2 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->passport_num = ident_type == 3 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->vat_id_num   = ident_type == 4 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->ssn_id_num   = ident_type == 5 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->birth_date   = 0;
        if (ident_type == 6) {
            boost::gregorian::date tmp = from_string(rinfo[0][7]);
            if (!tmp.is_special()) {
                Date birth;
                birth.day   = static_cast<int>(tmp.day());
                birth.month = static_cast<int>(tmp.month());
                birth.year  = static_cast<int>(tmp.year());

                data->birth_date = new NullableDate(birth);
            }
        }

        data->disclose_name         = corba_wrap_nullable_boolean(rinfo[0][8]);
        data->disclose_organization = corba_wrap_nullable_boolean(rinfo[0][9]);
        data->disclose_vat          = corba_wrap_nullable_boolean(rinfo[0][10]);
        data->disclose_ident        = corba_wrap_nullable_boolean(rinfo[0][11]);
        data->disclose_email        = corba_wrap_nullable_boolean(rinfo[0][12]);
        data->disclose_notify_email = corba_wrap_nullable_boolean(rinfo[0][13]);
        data->disclose_address      = corba_wrap_nullable_boolean(rinfo[0][14]);
        data->disclose_phone        = corba_wrap_nullable_boolean(rinfo[0][15]);
        data->disclose_fax          = corba_wrap_nullable_boolean(rinfo[0][16]);

        data->addresses.length(1);
        data->addresses[0].type         = "DEFAULT";
        data->addresses[0].street1      = corba_wrap_string(rinfo[0][17]);
        data->addresses[0].street2      = corba_wrap_nullable_string(rinfo[0][18]);
        data->addresses[0].street3      = corba_wrap_nullable_string(rinfo[0][19]);
        data->addresses[0].city         = corba_wrap_string(rinfo[0][20]);
        data->addresses[0].state        = corba_wrap_nullable_string(rinfo[0][21]);
        data->addresses[0].postal_code  = corba_wrap_string(rinfo[0][22]);
        data->addresses[0].country_code = corba_wrap_string(rinfo[0][23]);

        data->emails.length(1);
        data->emails[0].type = "DEFAULT";
        data->emails[0].email_address = corba_wrap_string(rinfo[0][24]);
        std::string notify_email = static_cast<std::string>(rinfo[0][25]);
        if (notify_email.size()) {
            data->emails.length(2);
            data->emails[1].type = "NOTIFY";
            data->emails[2].email_address = corba_wrap_string(notify_email);
        }

        std::string telephone = static_cast<std::string>(rinfo[0][26]);
        std::string fax = static_cast<std::string>(rinfo[0][27]);
        if (telephone.size()) {
            data->phones.length(1);
            data->phones[0].type = "DEFAULT";
            data->phones[0].number = corba_wrap_string(telephone);
        }
        if (fax.size()) {
            unsigned int s = data->phones.length();
            data->phones.length(s + 1);
            data->phones[s].type = "FAX";
            data->phones[s].number = corba_wrap_string(fax);
        }

        return data;
    }
    catch (std::exception &_ex) {
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        throw Registry::MojeID::ErrorReport("unknown exception");
    }
}


void MojeIDImpl::commitPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("commit-prepared"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
}


void MojeIDImpl::rollbackPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("rollback-prepared"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
}


}

