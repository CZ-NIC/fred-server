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

#include <memory>

#include "commonclient.h"
#include "keysetclient.h"

namespace Admin {

void
KeysetClient::runMethod()
{
    if (keyset_list) {
        list();
    } else if (keyset_list_plain) {
        list_plain();
    } else if (keyset_check.is_value_set()) {
        check();
    } else if (keyset_info2.is_value_set()) {
        info2();
    } else if (keyset_info.is_value_set()) {
        info();
    } else if (keyset_show_opts) {
        //show_opts();
    }
}

void
KeysetClient::list()
{
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(m_db, true));
    std::auto_ptr<Fred::KeySet::List> keyList(
            keyMan->createList());

    Database::Filters::KeySet *keyFilter;
    keyFilter = new Database::Filters::KeySetHistoryImpl();

    if (m_list_args.keyset_id.is_value_set())
        keyFilter->addId().setValue(Database::ID(m_list_args.keyset_id.get_value()));
    if (m_list_args.keyset_handle.is_value_set())
        keyFilter->addHandle().setValue(m_list_args.keyset_handle.get_value());
    if (m_list_args.admin_id.is_value_set())
        keyFilter->addTechContact().addId().setValue(
                Database::ID(m_list_args.admin_id.get_value()));
    if (m_list_args.admin_handle.is_value_set())
        keyFilter->addTechContact().addHandle().setValue(
                m_list_args.admin_handle.get_value());
    if (m_list_args.admin_name.is_value_set())
        keyFilter->addTechContact().addName().setValue(
                m_list_args.admin_name.get_value());
    if (m_list_args.registrar_id.is_value_set())
        keyFilter->addRegistrar().addId().setValue(Database::ID(m_list_args.registrar_id.get_value()));
    if (m_list_args.registrar_handle.is_value_set())
        keyFilter->addRegistrar().addHandle().setValue(m_list_args.registrar_handle.get_value());
    if (m_list_args.registrar_name.is_value_set())
        keyFilter->addRegistrar().addName().setValue(m_list_args.registrar_name.get_value());
    if (m_list_args.crdate.is_value_set())
    {
        keyFilter->addCreateTime().setValue(
                *parseDateTime(m_list_args.crdate.get_value()));
    }
    if (m_list_args.deldate.is_value_set())
    {
            keyFilter->addDeleteTime().setValue(
                    *parseDateTime(m_list_args.deldate.get_value()));
    }
    if (m_list_args.update.is_value_set())
    {
        keyFilter->addUpdateTime().setValue(
                *parseDateTime(m_list_args.update.get_value()));
    }
    if (m_list_args.transdate.is_value_set())
    {
        keyFilter->addTransferTime().setValue(
                *parseDateTime(m_list_args.transdate.get_value()));
    }
    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    
    unionFilter->addFilter(keyFilter);
    //apply_LIMIT(keyList);
    keyList->setLimit(m_list_args.limit.get_value());

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
        if (m_list_args.full_list) {
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
    std::string xml;
    std::string cltrid;

    xml = "<handle>" + m_list_args.keyset_handle.get_value() + "</handle>";
    cltrid = "list_keysets";
    ccReg::Lists *k;

    epp_client_login_return epp_login = epp_client_login(
            m_list_args.login_registrar.get_value());

    epp_login.r = epp_login.epp->KeySetList(k, epp_login.clientId, cltrid.c_str(), xml.c_str());

    if (epp_login.r->code != 1000) {
        std::cerr << "An error has occured: " << epp_login.r->code;
        return;
    }
    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",
                "<system_delete_logout/>");
    return;
}

void
KeysetClient::check()
{

    std::string xml;
    std::string cltrid;

    ccReg::CheckResp *a;
    
    std::string name = keyset_check.get_value();

    cltrid = "keyset_check";
    xml = "<handle>" + name + "</handle>";

    ccReg::Check names;
    names.length(1);
    names[0] = CORBA::string_dup(name.c_str());

    epp_client_login_return epp_login = epp_client_login(
            m_list_args.login_registrar.get_value());


    epp_login.r = epp_login.epp->KeySetCheck(names, a, epp_login.clientId, cltrid.c_str(), xml.c_str());

    std::cout << (*a)[0].avail << std::endl;
    std::cout << (*a)[0].reason << std::endl;

    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",
                    "<system_delete_logout/>");
    return;
}

void
KeysetClient::info()
{
    std::string name = keyset_info.get_value();
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "info_keyset";

    epp_client_login_return epp_login = epp_client_login(
            m_list_args.login_registrar.get_value());

    ccReg::KeySet* k = new ccReg::KeySet;
    epp_login.epp->KeySetInfo(name.c_str(), k, epp_login.clientId, cltrid.c_str(), xml.c_str());

    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",
                    "<system_delete_logout/>");

    std::cout << k->handle << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    // std::cout << k->dsrec[0].digest << std::endl;
    std::cout << k->tech[0] << std::endl;

    return;
}

void
KeysetClient::info2()
{
    std::string key_handle = keyset_info2.get_value();
    std::string cltrid;
    std::string xml;

    CORBA::Long count;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "info_keyset_2";
    ccReg::InfoType type = ccReg::IT_LIST_KEYSETS;

    epp_client_login_return epp_login = epp_client_login(
            m_list_args.login_registrar.get_value());

    epp_login.r = epp_login.epp->info(
            type,
            key_handle.c_str(),
            count,
            epp_login.clientId,
            cltrid.c_str(),
            xml.c_str());

    std::cout << "return code: " << epp_login.r->code << std::endl;


    epp_login.epp->ClientLogout(epp_login.clientId,"system_delete_logout",
                    "<system_delete_logout/>");

    return;
}

} //namespace Admin;
