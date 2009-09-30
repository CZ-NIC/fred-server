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

#include "simple.h"
#include "commonclient.h"
#include "publicreqclient.h"
#include "register/public_request.h"

namespace Admin {

const struct options *
PublicRequestClient::getOpts()
{
    return m_opts;
}

void
PublicRequestClient::runMethod()
{
    if (m_conf.hasOpt(PUBLICREQ_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(PUBLICREQ_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
PublicRequestClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Public request", getOpts(), getOptsCount());
}

void
PublicRequestClient::list()
{
    callHelp(m_conf, list_help);
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
            Register::Zone::Manager::create());
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

    if (m_conf.hasOpt(PUBLICREQ_REGISTRAR_ID_NAME))
	    prFilter->addRegistrar().addId().setValue(Database::ID(m_conf.get<unsigned int>(PUBLICREQ_REGISTRAR_ID_NAME)));

    if (m_conf.hasOpt(PUBLICREQ_REGISTRAR_HANDLE_NAME))
	    prFilter->addRegistrar().addHandle().setValue(m_conf.get<std::string>(PUBLICREQ_REGISTRAR_HANDLE_NAME));

    if (m_conf.hasOpt(PUBLICREQ_REGISTRAR_NAME_NAME))
	    prFilter->addRegistrar().addName().setValue(m_conf.get<std::string>(PUBLICREQ_REGISTRAR_NAME_NAME));




    apply_CRDATE(prFilter);
    apply_DATETIME(prFilter, PUBLICREQ_RESDATE_NAME, Resolve);

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
            << "\t\t<registrar_id>" << pr->getRegistrarId() << "</registrar_id>\n"
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
        << "** list authorizatin info **\n\n"
        << "  $ " << g_prog_name << " --" << PUBLICREQ_LIST_NAME << " \\\n"
        << "    [--" << PUBLICREQ_TYPE_NAME << "=<public_request_type>] \\\n"
        << "    [--" << PUBLICREQ_STATUS_NAME << "=<public_request_status>] \\\n"
        << "    [--" << PUBLICREQ_ANSWER_EMAIL_NAME << "=<answer_email>] \\\n"
        << "    [--" << PUBLICREQ_ANSWER_EMAIL_ID_NAME << "=<answer_email_id>] \\\n"
        << "    [--" << PUBLICREQ_REASON_NAME << "=<public_request_reason>] \\\n"
        << "    [--" << PUBLICREQ_REGISTRAR_ID_NAME << "=<registrar_id>] \\\n"
        << "    [--" << PUBLICREQ_REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        << "    [--" << PUBLICREQ_REGISTRAR_NAME_NAME << "=<registrar_name>] \\\n"
        << "    [--" << CRDATE_NAME << "=<create_date>] \\\n"
        << "    [--" << PUBLICREQ_RESDATE_NAME << "=<resolve_date>] \n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_PUBLICREQUEST, name, name##_DESC, type, callable, visible}

const struct options
PublicRequestClient::m_opts[] = {
    ADDOPT(PUBLICREQ_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(PUBLICREQ_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_NAME,
    ADDOPT(PUBLICREQ_TYPE_NAME, TYPE_INT, false, false),
    ADDOPT(PUBLICREQ_STATUS_NAME, TYPE_UINT, false, false),
    ADDOPT(PUBLICREQ_ANSWER_EMAIL_NAME, TYPE_STRING, false, false),
    ADDOPT(PUBLICREQ_ANSWER_EMAIL_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(PUBLICREQ_REGISTRAR_ID_NAME, TYPE_UINT, false, false),
    ADDOPT(PUBLICREQ_REGISTRAR_HANDLE_NAME, TYPE_STRING, false, false),
    ADDOPT(PUBLICREQ_REGISTRAR_NAME_NAME, TYPE_STRING, false, false),
    add_CRDATE,
    ADDOPT(PUBLICREQ_RESDATE_NAME, TYPE_STRING, false, false),
};

#undef ADDOPT

int
PublicRequestClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

