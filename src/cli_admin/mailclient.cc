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
#include "mailclient.h"

namespace Admin {

const struct options *
MailClient::getOpts()
{
    return m_opts;
}

void
MailClient::runMethod()
{
    if (m_conf.hasOpt(MAIL_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(MAIL_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
MailClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("Mail", getOpts(), getOptsCount());
}

void
MailClient::list()
{
    callHelp(m_conf, list_help);
    std::auto_ptr<Register::Mail::Manager> mailMan(
            Register::Mail::Manager::create());
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

    apply_DATETIME(mailFilter, MAIL_MODDATE_NAME, Modify);

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
    delete unionFilter;
}

void
MailClient::list_help()
{
    std::cout << "mail client list help" << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_MAIL, name, name##_DESC, type, callable, visible}

const struct options
MailClient::m_opts[] = {
    ADDOPT(MAIL_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(MAIL_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    ADDOPT(MAIL_TYPE_NAME, TYPE_INT, false, false),
    ADDOPT(MAIL_STATUS_NAME, TYPE_INT, false, false),
    ADDOPT(MAIL_ATTEMPT_NAME, TYPE_INT, false, false),
    ADDOPT(MAIL_MESSAGE_NAME, TYPE_STRING, false, false),
    add_CRDATE,
    ADDOPT(MAIL_MODDATE_NAME, TYPE_STRING, false, false),
};

#undef ADDOPT

int 
MailClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;


