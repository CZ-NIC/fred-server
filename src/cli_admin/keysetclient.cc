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
#include "keysetclient.h"

namespace Admin {

const struct options *
KeysetClient::getOpts()
{
    return m_opts;
}

void
KeysetClient::runMethod()
{
    if (m_conf.hasOpt(KEYSET_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(KEYSET_CHECK_NAME)) {
        check();
    } else if (m_conf.hasOpt(KEYSET_INFO2_NAME)) {
        info2();
    } else if (m_conf.hasOpt(KEYSET_INFO_NAME)) {
        info();
    } else if (m_conf.hasOpt(KEYSET_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
KeysetClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Keyset", getOpts(), getOptsCount());
}

void
KeysetClient::list()
{
    callHelp(m_conf, list_help);
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(&m_db, true));
    std::auto_ptr<Fred::KeySet::List> keyList(
            keyMan->createList());

    Database::Filters::KeySet *keyFilter;
    keyFilter = new Database::Filters::KeySetHistoryImpl();

    apply_ID(keyFilter);
    apply_HANDLE(keyFilter);

    if (m_conf.hasOpt(ADMIN_ID_NAME))
        keyFilter->addTechContact().addId().setValue(
                Database::ID(m_conf.get<unsigned int>(ADMIN_ID_NAME)));
    if (m_conf.hasOpt(ADMIN_HANDLE_NAME))
        keyFilter->addTechContact().addHandle().setValue(
                m_conf.get<std::string>(ADMIN_HANDLE_NAME));
    if (m_conf.hasOpt(ADMIN_NAME_NAME))
        keyFilter->addTechContact().addName().setValue(
                m_conf.get<std::string>(ADMIN_NAME_NAME));

    apply_REGISTRAR_ID(keyFilter);
    apply_REGISTRAR_HANDLE(keyFilter);
    apply_REGISTRAR_NAME(keyFilter);

    apply_CRDATE(keyFilter);
    apply_DELDATE(keyFilter);
    apply_UPDATE(keyFilter);
    apply_TRANSDATE(keyFilter);

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    
    unionFilter->addFilter(keyFilter);
    apply_LIMIT(keyList);

    keyList->reload(*unionFilter);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < keyList->getCount(); i++) {
        Fred::KeySet::KeySet *keyset = keyList->getKeySet(i);
        std::cout
            << "\t<keyset>\n"
            << "\t\t<id>" << keyset->getId() << "</id>\n"
            << "\t\t<handle>" << keyset->getHandle() << "</handle>\n";
        for (unsigned int j = 0; j < keyset->getAdminCount(); j++) {
            std::cout
                << "\t\t<admin>\n"
                << "\t\t\t<id>" << keyset->getAdminIdByIdx(j) << "</id>\n"
                << "\t\t\t<handle>" << keyset->getAdminHandleByIdx(j) << "</handle>\n"
                << "\t\t</admin>\n";
        }
        for (unsigned int j = 0; j < keyset->getDSRecordCount(); j++) {
            Fred::KeySet::DSRecord *dsrec = (Fred::KeySet::DSRecord *)keyset->getDSRecordByIdx(j);
            std::cout
                << "\t\t<dsrecord>\n"
                << "\t\t\t<id>" << dsrec->getId() << "</id>\n"
                << "\t\t\t<keytag>" << dsrec->getKeyTag() << "</keytag>\n"
                << "\t\t\t<alg>" << dsrec->getAlg() << "</alg>\n"
                << "\t\t\t<digesttype>" << dsrec->getDigestType() << "</digesttype>\n"
                << "\t\t\t<digest>" << dsrec->getDigest() << "</digest>\n"
                << "\t\t\t<maxsiglife>" << dsrec->getMaxSigLife() << "</maxsiglife>\n"
                << "\t\t</dsrecord>\n";
        }
        for (unsigned int j = 0; j < keyset->getDNSKeyCount(); j++) {
            Fred::KeySet::DNSKey *dnskey = (Fred::KeySet::DNSKey *)keyset->getDNSKeyByIdx(j);
            std::cout
                << "\t\t<dnskey>\n"
                << "\t\t\t<id>" << dnskey->getId() << "</id>\n"
                << "\t\t\t<flags>" << dnskey->getFlags() << "</flags>\n"
                << "\t\t\t<protocol>" << dnskey->getProtocol() << "</protocol>\n"
                << "\t\t\t<alg>" << dnskey->getAlg() << "</alg>\n"
                << "\t\t\t<key>" << dnskey->getKey() << "</key>\n"
                << "\t\t</dnskey>\n";
        }
        if (m_conf.hasOpt(FULL_LIST_NAME)) {
            std::cout
                << "\t\t<create_date>" << keyset->getCreateDate() << "</create_date>\n"
                << "\t\t<transfer_date>" << keyset->getTransferDate() << "</transfer_date>\n"
                << "\t\t<update_date>" << keyset->getUpdateDate() << "</update_date>\n"
                << "\t\t<delete_date>" << keyset->getDeleteDate() << "</delete_date>\n"
                << "\t\t<registrar>\n"
                << "\t\t\t<id>" << keyset->getRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << keyset->getRegistrarHandle() << "</handle>\n"
                << "\t\t</registrar>\n"
                << "\t\t<create_registrar>\n"
                << "\t\t\t<id>" << keyset->getCreateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << keyset->getCreateRegistrarHandle() << "</handle>\n"
                << "\t\t</create_registrar>\n"
                << "\t\t<update_registrar>\n"
                << "\t\t\t<id>" << keyset->getUpdateRegistrarId() << "</id>\n"
                << "\t\t\t<handle>" << keyset->getUpdateRegistrarHandle() << "</handle>\n"
                << "\t\t</update_registrar>\n"
                << "\t\t<auth_password>" << keyset->getAuthPw() << "</auth_password>\n"
                << "\t\t<ROID>" << keyset->getROID() << "</ROID>\n";
            for (unsigned int j = 0; j < keyset->getStatusCount(); j++) {
                Fred::Status *status = (Fred::Status *)keyset->getStatusByIdx(j);
                std::cout
                    << "\t\t<status>\n"
                    << "\t\t\t<id>" << status->getStatusId() << "</id>\n"
                    << "\t\t\t<from>" << status->getFrom() << "</from>\n"
                    << "\t\t\t<to>" << status->getTo() << "</to>\n"
                    << "\t\t</status>\n";
            }
        }
        std::cout
            << "\t</keyset>\n";
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    delete unionFilter;
}

void
KeysetClient::list_plain()
{
    callHelp(m_conf, no_help);
    std::string xml;
    std::string cltrid;

    xml = "<handle>" + m_conf.get<std::string>(KEYSET_LIST_PLAIN_NAME) + "</handle>";
    cltrid = "list_keysets";
    ccReg::Lists *k;
    CLIENT_LOGIN;

    r = epp->KeySetList(k, clientId, cltrid.c_str(), xml.c_str());

    if (r->code != 1000) {
        std::cerr << "An error has occured: " << r->code;
        return;
    }
    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    CLIENT_LOGOUT;
    return;
}

void
KeysetClient::check()
{
    callHelp(m_conf, check_help);
    std::string xml;
    std::string cltrid;

    ccReg::CheckResp *a;
    
    std::string name = m_conf.get<std::string>(KEYSET_CHECK_NAME).c_str();

    cltrid = "keyset_check";
    xml = "<handle>" + name + "</handle>";

    ccReg::Check names;
    names.length(1);
    names[0] = CORBA::string_dup(name.c_str());

    CLIENT_LOGIN;

    r = epp->KeySetCheck(names, a, clientId, cltrid.c_str(), xml.c_str());

    std::cout << (*a)[0].avail << std::endl;
    std::cout << (*a)[0].reason << std::endl;

    CLIENT_LOGOUT;
    return;
}

void
KeysetClient::info()
{
    callHelp(m_conf, info_help);
    std::string name = m_conf.get<std::string>(KEYSET_INFO_NAME);
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "info_keyset";

    CLIENT_LOGIN;
    ccReg::KeySet *k = new ccReg::KeySet;
    epp->KeySetInfo(name.c_str(), k, clientId, cltrid.c_str(), xml.c_str());
    CLIENT_LOGOUT;

    std::cout << k->handle << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    // std::cout << k->dsrec[0].digest << std::endl;
    std::cout << k->tech[0] << std::endl;

    delete k;
    return;
}

void
KeysetClient::info2()
{
    callHelp(m_conf, no_help);
    std::string key_handle = m_conf.get<std::string>(KEYSET_INFO2_NAME);
    std::string cltrid;
    std::string xml;

    CORBA::Long count;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "info_keyset_2";
    ccReg::InfoType type = ccReg::IT_LIST_KEYSETS;

    CLIENT_LOGIN;

    r = epp->info(
            type,
            key_handle.c_str(),
            count,
            clientId,
            cltrid.c_str(),
            xml.c_str());

    std::cout << "return code: " << r->code << std::endl;


    CLIENT_LOGOUT;
    return;
}

void
KeysetClient::list_help()
{
    std::cout
        << "** KeySet list **\n\n"
        << "  $ " << g_prog_name << " --" << KEYSET_LIST_HELP_NAME << " \\\n"
        << "    [--" << ID_NAME << "=<id_nubmer>] \\\n"
        << "    [--" << HANDLE_NAME << "=<handle>] \\\n"
        << "    [--" << ADMIN_ID_NAME << "=<admin_id_number>] \\\n"
        << "    [--" << ADMIN_HANDLE_NAME << "=<admin_handle>] \\\n"
        << "    [--" << ADMIN_NAME_NAME << "=<admin_name>] \\\n"
        << "    [--" << REGISTRAR_ID_NAME << "=<registrar_id_number>] \\\n"
        << "    [--" << REGISTRAR_HANDLE_NAME << "=<registrar_handle>] \\\n"
        << "    [--" << REGISTRAR_NAME_NAME << "=<registrar_name>] \\\n"
        << "    [--" << CRDATE_NAME << "=<create_date>] \\\n"
        << "    [--" << DELDATE_NAME << "=<delete_date>] \\\n"
        << "    [--" << UPDATE_NAME << "=<update_date>] \\\n"
        << "    [--" << TRANSDATE_NAME << "<transfer_date>] \\\n"
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

void
KeysetClient::info_help()
{
    std::cout
        << "** Keyset info **\n\n"
        << "  " << g_prog_name << " --" << KEYSET_INFO_NAME << "=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --" << KEYSET_INFO_NAME << "=\"KEY::001\"\n"
        << "Command print on screen some informatins about keyset (identified by handle) such as handle, auth info password, roid,\n"
        << "tech contact handles, and dsrecord informations"
        << std::endl;
}
void
KeysetClient::check_help()
{
    std::cout
        << "** Keyset check **\n\n"
        << "  " << g_prog_name << " --" << KEYSET_CHECK_NAME << "=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --" << KEYSET_CHECK_NAME << "=\"K:02344\"\n"
        << "Command return 1 if keyset handle is free for registration and 0 if is already used in registry."
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_KEYSET, name, name##_DESC, type, callable, visible}

const struct options
KeysetClient::m_opts[] = {
    ADDOPT(KEYSET_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(KEYSET_LIST_PLAIN_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(KEYSET_INFO_NAME, TYPE_STRING, true, true),
    ADDOPT(KEYSET_INFO2_NAME, TYPE_STRING, true, true),
    ADDOPT(KEYSET_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_HANDLE,
    add_ADMIN_ID,
    add_ADMIN_HANDLE,
    add_ADMIN_NAME,
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    add_REGISTRAR_NAME,
    add_ADMIN_NAME,
    add_ADMIN_ADD,
    add_ADMIN_REM,
    add_CRDATE,
    add_UPDATE,
    add_TRANSDATE,
    add_DELDATE,
    add_AUTH_PW,
};

#undef ADDOPT

int 
KeysetClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}
} //namespace Admin;
