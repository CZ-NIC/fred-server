/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <utility>

#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/domainclient.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registry.hh"
#include "src/libfred/registrable_object/domain/create_domain.hh"
#include "src/libfred/registrable_object/domain/delete_domain.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

namespace Admin {

void
DomainClient::runMethod()
{
    if (domain_list_)
    {
        domain_list();
    }
}


void
DomainClient::domain_list()
{
    std::unique_ptr<LibFred::Zone::Manager> zoneMan(
            LibFred::Zone::Manager::create());

    std::unique_ptr<LibFred::Domain::Manager> domMan(
            LibFred::Domain::Manager::create(m_db, zoneMan.get()));
    std::unique_ptr<LibFred::Domain::List> domList(
            domMan->createList());

    Database::Filters::Domain *domFilter;
    domFilter = new Database::Filters::DomainHistoryImpl();

    if (domain_id_.is_value_set())
        domFilter->addId().setValue(Database::ID(domain_id_.get_value()));
    if (fqdn_.is_value_set())
        domFilter->addFQDN().setValue(fqdn_.get_value());
    if (domain_handle_.is_value_set())
        domFilter->addHandle().setValue(domain_handle_.get_value());
    if (nsset_id_.is_value_set())
        domFilter->addNSSet().addId().setValue(Database::ID(nsset_id_.get_value()));
    if (nsset_handle_.is_value_set())
        domFilter->addNSSet().addHandle().setValue(nsset_handle_.get_value());
    if (any_nsset_) domFilter->addNSSet();
    if (keyset_id_.is_value_set())
        domFilter->addKeySet().addId().setValue(Database::ID(keyset_id_.get_value()));
    if (keyset_handle_.is_value_set())
        domFilter->addKeySet().addHandle().setValue(keyset_handle_.get_value());
    if (any_keyset_) domFilter->addKeySet();
    if (zone_id_.is_value_set())
        domFilter->addZoneId().setValue(Database::ID(zone_id_.get_value()));
    if (registrant_id_.is_value_set())
        domFilter->addRegistrant().addId().setValue(Database::ID(registrant_id_.get_value()));
    if (registrant_handle_.is_value_set())
        domFilter->addRegistrant().addHandle().setValue(registrant_handle_.get_value());
    if (registrant_name_.is_value_set())
        domFilter->addRegistrant().addName().setValue(registrant_name_.get_value());
    if (crdate_.is_value_set())
        domFilter->addCreateTime().setValue(*parseDateTime(crdate_.get_value()));
    if (deldate_.is_value_set())
        domFilter->addDeleteTime().setValue(*parseDateTime(deldate_.get_value()));
    if (transdate_.is_value_set())
        domFilter->addTransferTime().setValue(*parseDateTime(transdate_.get_value()));
    if (update_.is_value_set())
        domFilter->addUpdateTime().setValue(*parseDateTime(update_.get_value()));
    if (admin_id_.is_value_set())
        domFilter->addAdminContact().addId().setValue(Database::ID(admin_id_.get_value()));
    if (admin_handle_.is_value_set())
        domFilter->addAdminContact().addHandle().setValue(admin_handle_.get_value());
    if (admin_name_.is_value_set())
        domFilter->addAdminContact().addName().setValue(admin_name_.get_value());
    if (out_zone_date_.is_value_set())
        domFilter->addOutZoneDate().setValue(*parseDate(out_zone_date_.get_value()));
    if (exp_date_.is_value_set())
        domFilter->addExpirationDate().setValue(*parseDate(exp_date_.get_value()));
    if (cancel_date_.is_value_set())
        domFilter->addCancelDate().setValue(*parseDate(cancel_date_.get_value()));

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    unionFilter->addFilter(domFilter);

    if(limit_.is_value_set())
        domList->setLimit(limit_.get_value());

    domList->reload(*unionFilter);

    std::cout << "<objects>\n";
    for (unsigned int i = 0; i < domList->getCount(); i++) {
        LibFred::Domain::Domain *domain = domList->getDomain(i);
        std::cout
            << "\t<domain>\n"
            << "\t\t<id>" << domain->getId() << "</id>\n"
            << "\t\t<fqdn>" << domain->getFQDN() << "</fqdn>\n"
            << "\t\t<fqdn_idn>" << domain->getFQDNIDN() << "</fqdn_idn>\n"
            << "\t\t<zone>" << domain->getZoneId() << "</zone>\n"
            << "\t\t<nsset>\n"
            << "\t\t\t<id>" << domain->getNssetId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getNssetHandle() << "</handle>\n"
            << "\t\t</nsset>\n"
            << "\t\t<keyset>\n"
            << "\t\t\t<id>" << domain->getKeysetId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getKeysetHandle() << "</handle>\n"
            << "\t\t</keyset>\n"
            << "\t\t<registrant>\n"
            << "\t\t\t<id>" << domain->getRegistrantId() << "</id>\n"
            << "\t\t\t<handle>" << domain->getRegistrantHandle() << "</handle>\n"
            << "\t\t\t<name>" << domain->getRegistrantName() << "</name>\n"
            << "\t\t</registrant>\n";
        for (unsigned int j = 0; j < domain->getAdminCount(); j++)
            std::cout
                << "\t\t<admin>\n"
                << "\t\t\t<id>" << domain->getAdminIdByIdx(j) << "</id>\n"
                << "\t\t\t<handle>" << domain->getAdminHandleByIdx(j) << "</handle>\n"
                << "\t\t</admin>\n";
        if (full_list_) {
            std::cout
                << "\t\t<exp_date>" << domain->getExpirationDate() << "</exp_date>\n"
                << "\t\t<val_ex_date>" << domain->getValExDate() << "</val_ex_date>\n"
                << "\t\t<zone_stat_time>" << domain->getZoneStatusTime() << "</zone_stat_time>\n"
                << "\t\t<out_zone_date>" << domain->getOutZoneDate() << "</out_zone_date>\n"
                << "\t\t<cancel_date>" << domain->getCancelDate() << "</cancel_date>\n"
                << "\t\t<create_date>" << domain->getCreateDate() << "</create_date>\n"
                << "\t\t<transfer_date>" << domain->getTransferDate() << "</transfer_date>\n"
                << "\t\t<update_date>" << domain->getUpdateDate() << "</update_date>\n"
                << "\t\t<delete_date>" << domain->getDeleteDate() << "</delete_date>\n"
                << "\t\t<registrar>\n"
                << "\t\t\t<id>" << domain->getRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getRegistrarHandle() << "</handle>\n"
                << "\t\t</registrar>\n"
                << "\t\t<create_registrar>\n"
                << "\t\t\t<id>" << domain->getCreateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getCreateRegistrarHandle() << "</handle>\n"
                << "\t\t</create_registrar>\n"
                << "\t\t<update_registrar>\n"
                << "\t\t\t<id>" << domain->getUpdateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << domain->getUpdateRegistrarHandle() << "</handle>\n"
                << "\t\t</update_registrar>\n"
                << "\t\t<auth_password>" << domain->getAuthPw() << "</auth_password>\n"
                << "\t\t<ROID>" << domain->getROID() << "</ROID>\n";
            for (unsigned int j = 0; j < domain->getStatusCount(); j++) {
                LibFred::Status *status = (LibFred::Status *)domain->getStatusByIdx(j);
                std::cout
                    << "\t\t<status>\n"
                    << "\t\t\t<id>" << status->getStatusId() << "</id>\n"
                    << "\t\t\t<from>" << status->getFrom() << "</from>\n"
                    << "\t\t\t<to>" << status->getTo() << "</to>\n"
                    << "\t\t</status>\n";
            }
        }
        std::cout
            << "\t</domain>\n";
    }
    std::cout << "</objects>" << std::endl;

    unionFilter->clear();
    delete unionFilter;
}

void
create_expired_domain(LibFred::Logger::LoggerClient& _logger_client, const CreateExpiredDomainArgs& _params)
{

    LibFred::OperationContextCreator ctx;
    unsigned long long deleted_domain_id = 0;
    unsigned long long new_domain_id = 0;

    unsigned long long req_id = _logger_client.createRequest("", "Admin", "",
            boost::assign::list_of
                (LibFred::Logger::RequestProperty("command", "create_expired_domain", false))
                (LibFred::Logger::RequestProperty("handle", _params.fqdn, true))
                (LibFred::Logger::RequestProperty("registrant", _params.registrant, true))
                (LibFred::Logger::RequestProperty("cltrid", _params.cltrid, true)),
            LibFred::Logger::ObjectReferences(),
            "CreateExpiredDomain", 0);
    if (req_id == 0) {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error("unable to log create expired domain request");
    }

    Database::Result registrar_res = ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system = True");
    if (registrar_res.size() != 1)
    {
        boost::format msg("System registrar is not found in database.");
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error(msg.str());
    }
    std::string registrar = static_cast<std::string>(registrar_res[0][0]);

    Database::Result registrant_res = ctx.get_conn().exec_params(
            "SELECT id FROM object_registry WHERE type = get_object_type_id('contact') AND name = $1::text AND erdate is NULL",
            Database::query_param_list(_params.registrant));
    if (registrant_res.size() != 1)
    {
        boost::format msg("Contact with handle %1% not found in database.");
        msg % _params.registrant;
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error(msg.str());
    }

    Database::Result fqdn_res = ctx.get_conn().exec_params(
            "SELECT id FROM object_registry WHERE type = get_object_type_id('domain') AND name = $1::text AND erdate is NULL",
            Database::query_param_list(_params.fqdn));

    if (fqdn_res.size() == 1)
    {
        if (_params.delete_existing)
        {
            LibFred::DeleteDomainByFqdn(_params.fqdn).exec(ctx);
            deleted_domain_id = static_cast<unsigned long long>(fqdn_res[0][0]);
        }
        else
        {
            boost::format msg("Domain with fqdn %1% already exists in database.");
            msg % _params.fqdn;
            logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
            throw std::runtime_error(msg.str());
        }
    }

    boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
    LibFred::CreateDomain::Result result;
    try
    {
        result = LibFred::CreateDomain(
            _params.fqdn,
            registrar,
            _params.registrant).set_expiration_date(current_date).exec(ctx);
    }
    catch (std::exception&)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error("domain create failed");
    }
    ctx.commit_transaction();
    logger_create_expired_domain_close(_logger_client, "Success", req_id, deleted_domain_id, result.create_object_result.object_id);
}

void
logger_create_expired_domain_close(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _result,
        const unsigned long long _req_id,
        const unsigned long long _deleted_domain_id,
        const unsigned long long _new_domain_id)
{
    LibFred::Logger::RequestProperties properties;
    LibFred::Logger::ObjectReferences references;
    properties.push_back(LibFred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
    if (_deleted_domain_id > 0)
    {
        references.push_back(LibFred::Logger::ObjectReference("domain", _deleted_domain_id));
    }
    if (_new_domain_id > 0)
    {
        references.push_back(LibFred::Logger::ObjectReference("domain", _new_domain_id));
    }
    _logger_client.closeRequest(_req_id, "Admin", "",
            properties,
            references,
            _result, 0);
}

} // namespace Admin;
