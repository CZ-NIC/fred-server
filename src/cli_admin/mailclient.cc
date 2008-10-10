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
#include "mailclient.h"

namespace Admin {

MailClient::MailClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Mail related options");
    m_options->add_options()
        addOpt(MAIL_LIST_NAME)
        addOpt(MAIL_LIST_HELP_NAME)
        addOpt(MAIL_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Mail related sub options");
    m_optionsInvis->add_options()
        add_ID()
        add_HANDLE()
        addOptInt(MAIL_TYPE_NAME)
        addOptInt(MAIL_STATUS_NAME)
        addOptInt(MAIL_ATTEMPT_NAME)
        addOptStr(MAIL_MESSAGE_NAME)
        add_CRDATE()
        addOptStr(MAIL_MODDATE_FROM_NAME)
        addOptStr(MAIL_MODDATE_TO_NAME);
}

MailClient::MailClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
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
        Config::Conf &conf)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
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
MailClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
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

    apply_ID(mailFilter);
    apply_HANDLE(mailFilter);
    apply_CRDATE(mailFilter);

    if (m_conf.hasOpt(MAIL_TYPE_NAME))
        mailFilter->addType().setValue(
                m_conf.get<int>(MAIL_TYPE_NAME));
    if (m_conf.hasOpt(MAIL_STATUS_NAME))
        mailFilter->addStatus().setValue(
                m_conf.get<int>(MAIL_STATUS_NAME));
    if (m_conf.hasOpt(MAIL_ATTEMPT_NAME))
        mailFilter->addAttempt().setValue(
                m_conf.get<int>(MAIL_ATTEMPT_NAME));
    if (m_conf.hasOpt(MAIL_MESSAGE_NAME))
        mailFilter->addMessage().setValue(
                m_conf.get<std::string>(MAIL_MESSAGE_NAME));
    if (m_conf.hasOpt(MAIL_ATTACHMENT_ID_NAME))
        mailFilter->addAttachment().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(MAIL_ATTACHMENT_ID_NAME)));
    if (m_conf.hasOpt(MAIL_ATTACHMENT_NAME_NAME))
        mailFilter->addAttachment().addName().setValue(
                m_conf.get<std::string>(MAIL_ATTACHMENT_NAME_NAME));

    if (m_conf.hasOpt(MAIL_MODDATE_FROM_NAME) || m_conf.hasOpt(MAIL_MODDATE_TO_NAME)) {
        Database::Date modDateFrom("1901-01-01 00:00:00");
        Database::Date modDateTo("2101-01-01 00:00:00");
        if (m_conf.hasOpt(MAIL_MODDATE_FROM_NAME))
            modDateFrom.from_string(
                    m_conf.get<std::string>(MAIL_MODDATE_FROM_NAME));
        if (m_conf.hasOpt(MAIL_MODDATE_TO_NAME))
            modDateTo.from_string(
                    m_conf.get<std::string>(MAIL_MODDATE_TO_NAME));
        mailFilter->addModifyTime().setValue(
                Database::DateTimeInterval(modDateFrom, modDateTo));
    }

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(mailFilter);
    mailList->setLimit(m_conf.get<unsigned int>(LIMIT_NAME));

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
        if (m_conf.hasOpt(FULL_LIST_NAME))
            std::cout 
                << "\t\t<content>" << mail->getContent() << "</content>\n";
        for (unsigned int j = 0; j < mail->getHandleSize(); j++) {
            std::cout
                << "\t\t<handle>" << mail->getHandle(j) << "</handle>\n";
        }
        for (unsigned int j = 0; j < mail->getAttachmentSize(); j++) {
            std::cout
                << "\t\t<attachment>" << mail->getAttachment(j).id << "</attachment>\n";
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


