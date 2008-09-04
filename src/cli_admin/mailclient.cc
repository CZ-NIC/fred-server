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

#include "commonclient.h"
#include "mailclient.h"

namespace Admin {

MailClient::MailClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Mail related options");
    m_options->add_options()
        add_opt(MAIL_LIST_NAME)
        add_opt(MAIL_LIST_HELP_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Mail related invisible options");
    m_optionsInvis->add_options()
        add_opt_type(ID_NAME, unsigned int)
        add_opt_type(HANDLE_NAME, std::string)
        add_opt_type(TYPE_NAME, int)
        add_opt_type(MAIL_STATUS_NAME, int)
        add_opt_type(MAIL_ATTEMPT_NAME, int)
        add_opt_type(MAIL_MESSAGE_NAME, std::string);

}
MailClient::MailClient(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
    m_options = NULL;
    m_optionsInvis = NULL;
}

MailClient::~MailClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
MailClient::init(
        std::string connstring,
        std::string nsAddr,
        boost::program_options::variables_map varMap)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_varMap = varMap;
}

boost::program_options::options_description *
MailClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
MailClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
MailClient::list()
{
    std::auto_ptr<Register::Mail::Manager> mailMan(
            Register::Mail::Manager::create(m_dbman));
    std::auto_ptr<Register::Mail::List> mailList(
            mailMan->createList());

    Database::Filters::Mail *mailFilter;
    mailFilter = new Database::Filters::MailImpl();

    if (m_varMap.count(ID_NAME))
        mailFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(HANDLE_NAME))
        mailFilter->addHandle().setValue(
                m_varMap[HANDLE_NAME].as<std::string>());
    if (m_varMap.count(TYPE_NAME))
        mailFilter->addType().setValue(
                m_varMap[TYPE_NAME].as<int>());
    if (m_varMap.count(MAIL_STATUS_NAME))
        mailFilter->addStatus().setValue(
                m_varMap[MAIL_STATUS_NAME].as<int>());
    if (m_varMap.count(MAIL_ATTEMPT_NAME))
        mailFilter->addAttempt().setValue(
                m_varMap[MAIL_ATTEMPT_NAME].as<int>());
    if (m_varMap.count(MAIL_MESSAGE_NAME))
        mailFilter->addMessage().setValue(
                m_varMap[MAIL_MESSAGE_NAME].as<std::string>());
    if (m_varMap.count(MAIL_ATTACHMENT_ID_NAME))
        mailFilter->addAttachment().addId().setValue(
                Database::ID(m_varMap[MAIL_ATTACHMENT_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(MAIL_ATTACHMENT_NAME_NAME))
        mailFilter->addAttachment().addName().setValue(
                m_varMap[MAIL_ATTACHMENT_NAME_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(mailFilter);
    mailList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    mailList->reload(*unionFilter);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < mailList->getCount(); i++) {
        Register::Mail::Mail *mail = mailList->get(i);
        std::cout
            << "\t<mail>\n"
            << "\t\t<id>" << mail->getId() << "</id>\n"
            << "\t\t<crtime>" << mail->getCreateTime() << "</crtime>\n"
            << "\t\t<modtime>" << mail->getModTime() << "</modtime>\n"
            << "\t\t<type>" << mail->getType() << "</type>\n"
            << "\t\t<type_desc>" << mail->getTypeDesc() << "</type_desc>\n"
            << "\t\t<status>" << mail->getStatus() << "</status>\n";
        if (m_varMap.count(FULL_LIST_NAME))
            std::cout 
                << "\t\t<content>" << mail->getContent() << "</content>\n";
        for (unsigned int j = 0; j < mail->getHandleSize(); j++) {
            std::cout
                << "\t\t<handle>" << mail->getHandle(j) << "</handle>\n";
        }
        for (unsigned int j = 0; j < mail->getAttachmentSize(); j++) {
            std::cout
                << "\t\t<attachment>" << mail->getAttachment(j) << "</attachment>\n";
        }
        std::cout
            << "\t</mail>\n";
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    // XXX this delete cause segfault :(
    // delete mailFilter;
    delete unionFilter;
}

void
MailClient::list_help()
{
    std::cout << "mail client list help" << std::endl;
}

} // namespace Admin;


