/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include <memory>

#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/keysetclient.hh"

namespace Admin {

void
KeysetClient::runMethod()
{
    if (keyset_list)
    {
        list();
    }
}

void
KeysetClient::list()
{
    std::unique_ptr<LibFred::Keyset::Manager> keyMan(
            LibFred::Keyset::Manager::create(m_db, true));
    std::unique_ptr<LibFred::Keyset::List> keyList(
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
        LibFred::Keyset::Keyset *keyset = keyList->getKeyset(i);
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
            LibFred::Keyset::DSRecord *dsrec = (LibFred::Keyset::DSRecord *)keyset->getDSRecordByIdx(j);
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
            LibFred::Keyset::DNSKey *dnskey = (LibFred::Keyset::DNSKey *)keyset->getDNSKeyByIdx(j);
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
                LibFred::Status *status = (LibFred::Status *)keyset->getStatusByIdx(j);
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

} // namespace Admin;
