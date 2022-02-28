/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#include "src/bin/corba/admin/public_request_mojeid.hh"

#include "src/deprecated/libfred/contact_verification/contact_verification_state.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "src/deprecated/libfred/public_request/public_request_impl.hh"
#include "src/util/types/birthdate.hh"

#include "libfred/mailer.hh"


namespace CorbaConversion {
namespace Admin {

namespace {

class MojeIdPublicRequestBase : public LibFred::PublicRequest::PublicRequestAuthImpl
{
public:
    void sendPasswords() override
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " should never be called");
    }

    std::string generatePasswords() override
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " should never be called");
    }
};

class MojeIdContactValidation : public LibFred::PublicRequest::PublicRequestImpl
{
public:
    bool check() const override
    {
        return true;
    }

    void invalidateAction() override
    {
        LOGGER.debug(boost::format("invalidation request id=%1%") % this->getId());
        /* just send email - note that difference between succesfully
         * processed email and invalidated email is done
         * by setting status_ = PRS_INVALIDATED which is passed to email in
         * fillTemplateParams(...) method -
         * (params["status"] = getStatus() == PRS_RESOLVED ? "1" : "2";)
         */
        this->get_answer_email_id() = this->sendEmail();
    }

    void processAction(bool _check [[gnu::unused]]) override
    {
        LOGGER.debug(boost::format("processing validation request id=%1%") % this->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        const unsigned long long oid = this->getObject(0).id;

        const LibFred::Contact::Verification::State contact_state =
                LibFred::Contact::Verification::get_contact_verification_state(oid);
        if (contact_state.has_all(LibFred::Contact::Verification::State::ciVm) ||
                !contact_state.has_all(LibFred::Contact::Verification::State::CivM))
        {
            throw LibFred::PublicRequest::NotApplicable("pre_insert_checks: failed!");
        }

        /* set new state */
        LibFred::PublicRequest::insertNewStateRequest(this->getId(), oid, "validatedContact");
        LibFred::update_object_states(oid);

        tx.commit();
    }

    void fillTemplateParams(::LibFred::Mailer::Parameters& params) const override
    {
        params["reqdate"] = stringify(this->getCreateTime().date());
        params["reqid"] = stringify(this->getId());

        if (this->getObjectSize())
        {
            params["type"] = stringify(this->getObject(0).type);
            params["handle"] = this->getObject(0).handle;
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                // clang-format off
                "SELECT c.name,c.organization,c.ssn,"
                       "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                       "CONCAT_WS(', ',NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),"
                                      "NULLIF(BTRIM(c.street3),''),NULLIF(BTRIM(c.postalcode),''),"
                                      "NULLIF(BTRIM(c.city),''),c.country) "
                "FROM public_request pr "
                "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
                "JOIN contact c ON c.id=prom.object_id "
                "WHERE pr.id=$1::INTEGER", Database::query_param_list(this->getId()));
                // clang-format on
        if (res.size() == 1)
        {
            const std::string ssn = res[0][2].isnull() ? std::string()
                                                       : static_cast<std::string>(res[0][2]);
            const std::string ssn_type = res[0][3].isnull() ? std::string()
                                                            : static_cast<std::string>(res[0][3]);
            params["name"] = res[0][0].isnull() ? std::string() : static_cast<std::string>(res[0][0]);
            params["org"] = res[0][1].isnull() ? std::string() : static_cast<std::string>(res[0][1]);
            params["ic"] = ssn_type == "ICO" ? ssn : std::string();
            params["birthdate"] = (ssn_type == "BIRTHDAY") && !ssn.empty()
                                          ? stringify(birthdate_from_string_to_date(ssn))
                                          : std::string();
            params["address"] = res[0][4].isnull() ? std::string() : static_cast<std::string>(res[0][4]);
            params["status"] = this->getStatus() == LibFred::PublicRequest::PRS_RESOLVED ? "1" : "2";
        }
    }

    std::string getTemplateName() const override
    {
        return "mojeid_validation";
    }

    void save() override
    {
        if (this->getId() == 0)
        {
            throw std::runtime_error("insert new request disabled");
        }
        PublicRequestImpl::save();
    }
};

} // namespace CorbaConversion::Admin::{anonymous}

} // namespace CorbaConversion::Admin
} // namespace CorbaConversion

using namespace CorbaConversion::Admin;

LibFred::PublicRequest::Factory&
CorbaConversion::Admin::add_producers(LibFred::PublicRequest::Factory& factory)
{
    return factory
            .add_producer({PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_CONTACT_IDENTIFICATION, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_CONTACT_VALIDATION, std::make_unique<MojeIdContactValidation>()})
            .add_producer({PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_CONTACT_REIDENTIFICATION, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER, std::make_unique<MojeIdPublicRequestBase>()})
            .add_producer({PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER, std::make_unique<MojeIdPublicRequestBase>()});
}
