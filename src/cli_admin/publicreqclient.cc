/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#include "simple.h"
#include "commonclient.h"
#include "publicreqclient.h"
#include "register/public_request.h"

namespace Admin {

PublicRequestClient::PublicRequestClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Public request related options");
    m_options->add_options()
        addOpt(PUBLICREQ_LIST_NAME)
        addOpt(PUBLICREQ_LIST_HELP_NAME)
        addOpt(PUBLICREQ_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Public request related sub options");
    m_optionsInvis->add_options()
        add_ID()
        add_NAME()
        addOptInt(PUBLICREQ_TYPE_NAME)
        addOptUInt(PUBLICREQ_STATUS_NAME)
        addOptStr(PUBLICREQ_ANSWER_EMAIL_NAME)
        addOptUInt(PUBLICREQ_ANSWER_EMAIL_ID_NAME)
        addOptUInt(PUBLICREQ_EPP_ID_NAME)
        addOptStr(PUBLICREQ_EPP_CLTRID_NAME)
        addOptStr(PUBLICREQ_EPP_SVTRID_NAME)
        addOptUInt(PUBLICREQ_EPP_CODE_RESPONSE_NAME)
        addOptUInt(PUBLICREQ_EPP_RESPONSE_NAME)
        addOptUInt(PUBLICREQ_EPP_TYPE_NAME)
        add_CRDATE()
        addOptStr(PUBLICREQ_RESDATE_FROM_NAME)
        addOptStr(PUBLICREQ_RESDATE_TO_NAME);
}
PublicRequestClient::PublicRequestClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

PublicRequestClient::~PublicRequestClient()
{
}

void
PublicRequestClient::init(
        std::string connstring,
        std::string nsAddr,
        Config::Conf &conf)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
}

boost::program_options::options_description *
PublicRequestClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
PublicRequestClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
PublicRequestClient::show_opts() const 
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

void
PublicRequestClient::list()
{
    Database::Filters::PublicRequest *prFilter;
    prFilter = new Database::Filters::PublicRequestImpl();

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());

    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));

    std::auto_ptr<Register::PublicRequest::Manager> prMan(
            Register::PublicRequest::Manager::create(
                m_dbman,
                domMan.get(),
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                &mailMan,
                docMan.get())
            );
    std::auto_ptr<Register::PublicRequest::List> prList(
            prMan->createList());

    apply_ID(prFilter);
    if (m_conf.hasOpt(PUBLICREQ_TYPE_NAME))
        prFilter->addType().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_TYPE_NAME));
    
    if (m_conf.hasOpt(PUBLICREQ_STATUS_NAME))
        prFilter->addType().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_STATUS_NAME));
    if (m_conf.hasOpt(PUBLICREQ_STATUS_NAME))
        prFilter->addStatus().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_STATUS_NAME));
    if (m_conf.hasOpt(PUBLICREQ_ANSWER_EMAIL_NAME))
        prFilter->addEmailToAnswer().setValue(
                m_conf.get<std::string>(PUBLICREQ_ANSWER_EMAIL_NAME));
    if (m_conf.hasOpt(PUBLICREQ_ANSWER_EMAIL_ID_NAME))
        prFilter->addAnswerEmailId().setValue(
                Database::ID(m_conf.get<unsigned int>(PUBLICREQ_ANSWER_EMAIL_ID_NAME)));
    if (m_conf.hasOpt(PUBLICREQ_REASON_NAME))
        prFilter->addReason().setValue(
                m_conf.get<std::string>(PUBLICREQ_REASON_NAME));
    if (m_conf.hasOpt(PUBLICREQ_EPP_ID_NAME))
        prFilter->addEppAction().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(PUBLICREQ_EPP_ID_NAME)));
    if (m_conf.hasOpt(PUBLICREQ_EPP_CLTRID_NAME))
        prFilter->addEppAction().addClTRID().setValue(
                m_conf.get<std::string>(PUBLICREQ_EPP_CLTRID_NAME));
    if (m_conf.hasOpt(PUBLICREQ_EPP_SVTRID_NAME))
        prFilter->addEppAction().addSvTRID().setValue(
                m_conf.get<std::string>(PUBLICREQ_EPP_SVTRID_NAME));
    if (m_conf.hasOpt(PUBLICREQ_EPP_CODE_RESPONSE_NAME))
        prFilter->addEppAction().addEppCodeResponse().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_EPP_CODE_RESPONSE_NAME));
    if (m_conf.hasOpt(PUBLICREQ_EPP_RESPONSE_NAME))
        prFilter->addEppAction().addResponse().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_EPP_RESPONSE_NAME));
    if (m_conf.hasOpt(PUBLICREQ_EPP_TYPE_NAME))
        prFilter->addEppAction().addType().setValue(
                m_conf.get<unsigned int>(PUBLICREQ_EPP_TYPE_NAME));

    apply_CRDATE(prFilter);
    if (m_conf.hasOpt(PUBLICREQ_RESDATE_FROM_NAME) || m_conf.hasOpt(PUBLICREQ_RESDATE_TO_NAME)) {
        Database::DateTime resDateFrom("1901-01-01 00:00:00");
        Database::DateTime resDateTo("2101-01-01 00:00:00");
        if (m_conf.hasOpt(PUBLICREQ_RESDATE_FROM_NAME))
            resDateFrom.from_string(
                    m_conf.get<std::string>(PUBLICREQ_RESDATE_FROM_NAME));
        if (m_conf.hasOpt(PUBLICREQ_RESDATE_TO_NAME))
            resDateTo.from_string(
                    m_conf.get<std::string>(PUBLICREQ_RESDATE_TO_NAME));
        prFilter->addResolveTime().setValue(
                Database::DateTimeInterval(resDateFrom, resDateTo));
    }

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(prFilter);
    prList->reload(*unionFilter);

    std::cout << "<object>\n";
    for (unsigned int j = 0; j < prList->getCount(); j++) {
        Register::PublicRequest::PublicRequest *pr =
            (Register::PublicRequest::PublicRequest *)prList->get(j);
        std::cout
            << "\t<public_request>\n"
            << "\t\t<id>" << pr->getId() << "</id>\n"
            << "\t\t<type>" << Register::PublicRequest::Type2Str(pr->getType()) << "</type>\n"
            << "\t\t<status>" << Register::PublicRequest::Status2Str(pr->getStatus()) << "</status>\n"
            << "\t\t<crtime>" << pr->getCreateTime() << "</crtime>\n"
            << "\t\t<resolve_time>" << pr->getResolveTime() << "</resolve_time>\n"
            << "\t\t<reason>" << pr->getReason() << "</reason>\n"
            << "\t\t<answer_email>" << pr->getEmailToAnswer() << "</answer_email>\n"
            << "\t\t<answer_email_id>" << pr->getAnswerEmailId() << "</answer_email_id>\n"
            << "\t\t<epp_action_id>" << pr->getEppActionId() << "</epp_action_id>\n"
            << "\t\t<check>" << pr->check() << "</check>\n";
        for (unsigned int j = 0; j < pr->getObjectSize(); j++) {
            std::cout
                << "\t\t<object_info>\n"
                << "\t\t\t<id>" << pr->getObject(j).id << "</id>\n"
                << "\t\t\t<handle>" << pr->getObject(j).handle << "</handle>\n"
                << "\t\t\t<type>" << Register::PublicRequest::ObjectType2Str(pr->getObject(j).type) << "</type>\n"
                << "\t\t</object_info>\n";
        }
        std::cout
            << "\t\t<svtrid>" << pr->getSvTRID() << "</svtrid>\n"
            << "\t\t<registrar>\n"
            << "\t\t\t<handle>" << pr->getRegistrarHandle() << "</handle>\n"
            << "\t\t\t<name>" << pr->getRegistrarName() << "</name>\n"
            << "\t\t\t<url>" << pr->getRegistrarUrl() << "</url>\n"
            << "\t\t</registrar>\n"
            << "\t\t<template>" << pr->getTemplateName() << "</template>\n"
            << "\t\t<emails>" << pr->getEmails() << "</emails>\n"
            << "\t</public_request>\n";
    }
    std::cout << "</object>" << std::endl;
    unionFilter->clear();
    delete unionFilter;
}

void
PublicRequestClient::list_help()
{
    std::cout
        << "** list authorizatin info **\n"
        << std::endl;
}

} // namespace Admin;


