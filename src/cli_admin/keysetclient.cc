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
#include "keysetclient.h"

namespace Admin {

KeysetClient::KeysetClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "KeySet related options");
    m_options->add_options()
        addOpt(KEYSET_LIST_NAME)
        addOpt(KEYSET_LIST_PLAIN_NAME)
        addOptStr(KEYSET_INFO_NAME)
        addOptStr(KEYSET_INFO2_NAME)
        addOptStr(KEYSET_SEND_AUTH_INFO_NAME)
        addOptStr(KEYSET_CREATE_NAME)
        addOptStr(KEYSET_DELETE_NAME)
        addOptStr(KEYSET_UPDATE_NAME)
        addOptStr(KEYSET_TRANSFER_NAME)
        addOpt(KEYSET_CREATE_HELP_NAME)
        addOpt(KEYSET_UPDATE_HELP_NAME)
        addOpt(KEYSET_DELETE_HELP_NAME)
        addOpt(KEYSET_CHECK_HELP_NAME)
        addOpt(KEYSET_INFO_HELP_NAME)
        addOpt(KEYSET_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "KeySet related sub options");
    m_optionsInvis->add_options()
        addOptStr(KEYSET_DSRECORDS_NAME)
        addOptStr(KEYSET_DSREC_ADD_NAME)
        addOptStr(KEYSET_DSREC_REM_NAME)
        addOptStr(KEYSET_DNSKEY_NAME)
        addOptStr(KEYSET_DNSKEY_ADD_NAME)
        addOptStr(KEYSET_DNSKEY_REM_NAME)
        add_ID()
        add_HANDLE()
        add_ADMIN_ID()
        add_ADMIN_HANDLE()
        add_ADMIN_NAME()
        add_REGISTRAR_ID()
        add_REGISTRAR_HANDLE()
        add_REGISTRAR_NAME()
        add_ADMIN_NAME()
        add_ADMIN_ADD()
        add_ADMIN_REM()
        add_CRDATE()
        add_UPDATE()
        add_TRANSDATE()
        add_DELDATE()
        add_AUTH_PW();
}

KeysetClient::KeysetClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

KeysetClient::~KeysetClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
KeysetClient::init(
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
KeysetClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
KeysetClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
KeysetClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
}

void
KeysetClient::list()
{
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(&m_db, true));
    std::auto_ptr<Register::KeySet::List> keyList(
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

    keyList->reload(*unionFilter, m_dbman);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < keyList->getCount(); i++) {
        Register::KeySet::KeySet *keyset = keyList->getKeySet(i);
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
            Register::KeySet::DSRecord *dsrec = (Register::KeySet::DSRecord *)keyset->getDSRecordByIdx(j);
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
            Register::KeySet::DNSKey *dnskey = (Register::KeySet::DNSKey *)keyset->getDNSKeyByIdx(j);
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
                Register::Status *status = (Register::Status *)keyset->getStatusByIdx(j);
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

int
KeysetClient::list_plain()
{
    std::string xml;
    std::string cltrid;

    xml = "<handle>" + m_conf.get<std::string>(KEYSET_LIST_PLAIN_NAME) + "</handle>";
    cltrid = "list_keysets";
    ccReg::Lists *k;
    CLIENT_LOGIN;

    r = epp->KeySetList(k, clientId, cltrid.c_str(), xml.c_str());

    if (r->code != 1000) {
        std::cerr << "An error has occured: " << r->code;
        return -1;
    }
    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::check()
{
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
    return 0;
}

int
KeysetClient::send_auth_info()
{
    std::string name = m_conf.get<std::string>(KEYSET_SEND_AUTH_INFO_NAME).c_str();
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "keyset_send_auth_info";

    CLIENT_LOGIN;
    r = epp->keysetSendAuthInfo(name.c_str(), clientId, cltrid.c_str(), xml.c_str());

    std::cout << r->code << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::transfer()
{
    std::string key_handle = m_conf.get<std::string>(KEYSET_TRANSFER_NAME).c_str();
    std::string authinfopw = m_conf.get<std::string>(AUTH_PW_NAME).c_str();

    std::string cltrid;
    std::string xml;

    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "transfer_keyset";

    CLIENT_LOGIN;

    r = epp->KeySetTransfer(
            key_handle.c_str(),
            authinfopw.c_str(),
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << r->code << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::update()
{
    std::string authinfopw("");
    std::string admins_add("");
    std::string admins_rem("");
    std::string dsrec_add("");
    std::string dsrec_rem("");
    std::string dnsk_add("");
    std::string dnsk_rem("");
    
    std::string key_handle = m_conf.get<std::string>(KEYSET_UPDATE_NAME).c_str();
    if (m_conf.hasOpt(AUTH_PW_NAME))
         authinfopw = m_conf.get<std::string>(AUTH_PW_NAME).c_str();
    if (m_conf.hasOpt(ADMIN_ADD_NAME))
        admins_add = m_conf.get<std::string>(ADMIN_ADD_NAME).c_str();
    if (m_conf.hasOpt(ADMIN_REM_NAME))
        admins_rem = m_conf.get<std::string>(ADMIN_REM_NAME).c_str();
    if (m_conf.hasOpt(KEYSET_DSREC_ADD_NAME))
        dsrec_add = m_conf.get<std::string>(KEYSET_DSREC_ADD_NAME).c_str();
    if (m_conf.hasOpt(KEYSET_DSREC_REM_NAME))
        dsrec_rem = m_conf.get<std::string>(KEYSET_DSREC_REM_NAME).c_str();
    if (m_conf.hasOpt(KEYSET_DNSKEY_ADD_NAME))
        dnsk_add = m_conf.get<std::string>(KEYSET_DNSKEY_ADD_NAME);
    if (m_conf.hasOpt(KEYSET_DNSKEY_REM_NAME))
        dnsk_rem = m_conf.get<std::string>(KEYSET_DNSKEY_REM_NAME);

    std::vector<std::string> admins_add_list = separate(admins_add);
    std::vector<std::string> admins_rem_list = separate(admins_rem);

    std::vector<std::string> dsrec_add_list = separate(dsrec_add);
    std::vector<std::string> dsrec_rem_list = separate(dsrec_rem);

    std::vector<std::string> dnskey_add_list = separate(dnsk_add);
    std::vector<std::string> dnskey_rem_list = separate(dnsk_rem);

    ccReg::TechContact admin_add;
    admin_add.length(admins_add_list.size());
    for (int i = 0; i < (int)admins_add_list.size(); i++) {
        admin_add[i] = CORBA::string_dup(admins_add_list[i].c_str());
    }
    ccReg::TechContact admin_rem;
    admin_rem.length(admins_rem_list.size());
    for (int i = 0; i < (int)admins_rem_list.size(); i++) {
        admin_rem[i] = CORBA::string_dup(admins_rem_list[i].c_str());
    }

    if ((dsrec_add_list.size() % 5) != 0) {
        std::cout << "Bad number of ``dsrec-add'' items." << std::endl;
        return 1;
    }
    if ((dsrec_rem_list.size() % 5) != 0) {
        std::cout << "Bad number of ``dsrec-rem'' items." << std::endl;
        return 1;
    }
    if ((dnskey_add_list.size() % 4) != 0) {
        std::cerr << "Bad number of ``" << KEYSET_DNSKEY_ADD_NAME << "'' items." << std::endl;
        return 1;
    }
    if ((dnskey_rem_list.size() % 4) != 0) {
        std::cerr << "Bad number of ``" << KEYSET_DNSKEY_REM_NAME << "'' items." << std::endl;
        return 1;
    }
    ccReg::DSRecord dsrecord_add;
    dsrecord_add.length(dsrec_add_list.size() / 5);

    ccReg::DSRecord dsrecord_rem;
    dsrecord_rem.length(dsrec_rem_list.size() / 5);

    ccReg::DNSKey dnskey_add;
    dnskey_add.length(dnskey_add_list.size() / 4);

    ccReg::DNSKey dnskey_rem;
    dnskey_rem.length(dnskey_rem_list.size() / 4);

    for (int i = 0; i < (int)(dsrec_add_list.size() / 5); i++) {
        dsrecord_add[i].keyTag = atol(dsrec_add_list[i * 5 + 0].c_str());
        dsrecord_add[i].alg = atol(dsrec_add_list[i * 5 + 1].c_str());
        dsrecord_add[i].digestType = atol(dsrec_add_list[i * 5 + 2].c_str());
        dsrecord_add[i].digest = CORBA::string_dup(dsrec_add_list[i * 5 + 3].c_str());
        if (dsrec_add_list[i * 5 + 4] == "NULL")
            dsrecord_add[i].maxSigLife = -1;
        else
            dsrecord_add[i].maxSigLife = atol(dsrec_add_list[i * 5 + 4].c_str());
    }
    for (int i = 0; i < (int)(dsrec_rem_list.size() / 5); i++) {
        dsrecord_rem[i].keyTag = atol(dsrec_rem_list[i * 5 + 0].c_str());
        dsrecord_rem[i].alg = atol(dsrec_rem_list[i * 5 + 1].c_str());
        dsrecord_rem[i].digestType = atol(dsrec_rem_list[i * 5 + 2].c_str());
        dsrecord_rem[i].digest = CORBA::string_dup(dsrec_rem_list[i * 5 + 3].c_str());
        if (dsrec_rem_list[i * 5 + 4] == "NULL")
            dsrecord_rem[i].maxSigLife = -1;
        else
            dsrecord_rem[i].maxSigLife = atol(dsrec_rem_list[i * 5 + 4].c_str());
    }
    for (int i = 0; i < (int)(dnskey_add_list.size() / 4); i++) {
        dnskey_add[i].flags     = atoi(dnskey_add_list[i * 4 + 0].c_str());
        dnskey_add[i].protocol  = atoi(dnskey_add_list[i * 4 + 1].c_str());
        dnskey_add[i].alg       = atoi(dnskey_add_list[i * 4 + 2].c_str());
        dnskey_add[i].key       = CORBA::string_dup(dnskey_add_list[i * 4 + 3].c_str());
    }
    for (int i = 0; i < (int)(dnskey_rem_list.size() / 4); i++) {
        dnskey_rem[i].flags     = atoi(dnskey_rem_list[i * 4 + 0].c_str());
        dnskey_rem[i].protocol  = atoi(dnskey_rem_list[i * 4 + 1].c_str());
        dnskey_rem[i].alg       = atoi(dnskey_rem_list[i * 4 + 2].c_str());
        dnskey_rem[i].key       = CORBA::string_dup(dnskey_rem_list[i * 4 + 3].c_str());
    }

#if 0
    for (int i = 0; i < (int)dnskey_add.length(); i++) {
        std::cout << "add {\n";
        std::cout << dnskey_add[i].flags << std::endl;
        std::cout << dnskey_add[i].protocol << std::endl;
        std::cout << dnskey_add[i].alg << std::endl;
        std::cout << dnskey_add[i].key << std::endl;
        std::cout << "}\n";
    }

    for (int i = 0; i < (int)dnskey_rem.length(); i++) {
        std::cout << "rem {\n";
        std::cout << dnskey_rem[i].flags << std::endl;
        std::cout << dnskey_rem[i].protocol << std::endl;
        std::cout << dnskey_rem[i].alg << std::endl;
        std::cout << dnskey_rem[i].key << std::endl;
        std::cout << "}\n";
    }
#endif

    std::string cltrid;
    std::string xml;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_update";
    CLIENT_LOGIN;

    r = epp->KeySetUpdate(
            key_handle.c_str(),
            authinfopw.c_str(),
            admin_add,
            admin_rem,
            dsrecord_add,
            dsrecord_rem,
            dnskey_add,
            dnskey_rem,
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << "return code: " << r->code << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::del()
{
    std::string key_handle = m_conf.get<std::string>(KEYSET_DELETE_NAME).c_str();

    std::cout << key_handle << std::endl;

    std::string cltrid;
    std::string xml;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_delete";

    CLIENT_LOGIN;

    r = epp->KeySetDelete(
            key_handle.c_str(),
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << "return code: " << r->code << std::endl;
    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::create()
{
    std::string key_handle = m_conf.get<std::string>(KEYSET_CREATE_NAME).c_str();
    std::string admins = m_conf.get<std::string>(ADMIN_NAME).c_str();
    std::string authinfopw("");
    if (m_conf.hasOpt(AUTH_PW_NAME))
        authinfopw = m_conf.get<std::string>(AUTH_PW_NAME).c_str();
    std::string dsrecords("");
    if (m_conf.hasOpt(KEYSET_DSRECORDS_NAME))
        dsrecords = m_conf.get<std::string>(KEYSET_DSRECORDS_NAME);
    std::string dnskeys("");
    if (m_conf.hasOpt(KEYSET_DNSKEY_NAME))
        dnskeys = m_conf.get<std::string>(KEYSET_DNSKEY_NAME);

    std::vector<std::string> admins_list = separateSpaces(admins.c_str());
    std::vector<std::string> dsrecords_list = separateSpaces(dsrecords.c_str());
    std::vector<std::string> dnskey_list = separateSpaces(dnskeys.c_str());

    ccReg::TechContact admin;
    admin.length(admins_list.size());
    for (int i = 0; i < (int)admins_list.size(); i++)
        admin[i] = CORBA::string_dup(admins_list[i].c_str());
    
    if ((dsrecords_list.size() % 5) != 0) {
        std::cerr << "Bad nubmer of ``dsrecords'' items." << std::endl;
        return 1;
    }
    ccReg::DSRecord dsrec;
    dsrec.length(dsrecords_list.size() / 5);

    for (int i = 0; i < (int)(dsrecords_list.size() / 5); i++) {
        dsrec[i].keyTag     = atol(dsrecords_list[i * 5 + 0].c_str());
        dsrec[i].alg        = atol(dsrecords_list[i * 5 + 1].c_str());
        dsrec[i].digestType = atol(dsrecords_list[i * 5 + 2].c_str());
        dsrec[i].digest     = CORBA::string_dup(dsrecords_list[i * 5 + 3].c_str());
        dsrec[i].maxSigLife = atol(dsrecords_list[i * 5 + 4].c_str());
    }
    if ((dnskey_list.size() % 4) != 0) {
        std::cerr << "Bad number of ``dnskey'' items." << std::endl;
        return 1;
    }
    ccReg::DNSKey dnskey;
    dnskey.length(dnskey_list.size() / 4);

    for (int i = 0; i < (int)(dnskey_list.size() / 4); i++) {
        dnskey[i].flags     = atoi(dnskey_list[i * 4 + 0].c_str());
        dnskey[i].protocol  = atoi(dnskey_list[i * 4 + 1].c_str());
        dnskey[i].alg       = atoi(dnskey_list[i * 4 + 2].c_str());
        dnskey[i].key       = CORBA::string_dup(dnskey_list[i * 4 + 3].c_str());
    }

    std::string cltrid;
    std::string xml;

    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_create";

    ccReg::timestamp crData;

    CLIENT_LOGIN;

    r = epp->KeySetCreate(
            key_handle.c_str(),
            authinfopw.c_str(),
            admin,
            dsrec,
            dnskey,
            crData,
            clientId,
            cltrid.c_str(), xml.c_str());

    std::cout << "return code: " << r->code << std::endl;

    CLIENT_LOGOUT;
    return 0;
}

int
KeysetClient::info()
{
    std::string name = m_conf.get<std::string>(KEYSET_INFO_NAME);
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "info_keyset";

    ccReg::KeySet *k = new ccReg::KeySet;

    CLIENT_LOGIN;
    epp->KeySetInfo(name.c_str(), k, clientId, cltrid.c_str(), xml.c_str());
    CLIENT_LOGOUT;

    std::cout << k->handle << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    std::cout << k->dsrec[0].digest << std::endl;
    std::cout << k->tech[0] << std::endl;

    delete k;
    return 0;
}

int
KeysetClient::info2()
{
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
    return 0;
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
        << "    [--" << FULL_LIST_NAME << "]\n"
        << std::endl;
}

void
KeysetClient::create_help()
{
    std::cout
        << "** KeySet create **\n\n"
        << "  " << g_prog_name << " --keyset-create=<keyset_handle> \\\n"
        << "    --admins=<list_of_admins> \\\n"
        << "    [--authinfopw=<authinfo_password>] \\\n"
        << "    --dsrecords=<list_of_dsrecords> \\\n"
        << "    --" << KEYSET_DNSKEY_NAME << "=<list_od_dnskeys>\n\n"
        << "example:\n"
        << "\t$ " << g_prog_name << " --keyset-create2=\"KEY::234\" --admins=\"CON::100 CON::50\" --dsrecords=\"0 0 0 348975238 -1\"\n"
        << "Commmand create keyset with handle ``KEY::234'' and with admin contacts selected by handles (``CON::100'' and ``CON::50'')\n"
        << "and dsrecord with NULL value for maxSigLife (structure of dsrecord is: ``keytag alg digesttype digest maxsiglife'')\n"
        << "dsrecord can also look like: ``0 0 0 348975238 -1 0 0 1 sdfioure 1000'' to create two dsrecords"
        << std::endl;
}
void
KeysetClient::update_help()
{
    std::cout
        << "** KeySet update **\n\n"
        << "  " << g_prog_name << " --keyset-update=<keyset_handle> \\\n"
        << "    [--authinfopw=<new_authinfo_password>] \\\n"
        << "    [--admins_add=<list_of_admins_to_add>] \\\n"
        << "    [--admins_rem=<list_of_admins_to_rem>] \\\n"
        << "    [--dsrec-add=<list_of_dsrecords_to_add>] \\\n"
        << "    [--dsrec-rem=<list_of_dsrecords_to_rem>] \\\n"
        << "    [--" << KEYSET_DNSKEY_ADD_NAME << "=<list_of_dnskeys_to_add>] \\\n"
        << "    [--" << KEYSET_DNSKEY_REM_NAME << "=<list_of_dnskeys_to_rem>]\n\n"
        << "List of DSRecords to remove or add (parameters ``--dsrec-add'' and ``--dsrec-rem'')"
        << "must look like:\n"
        << "\t --dsrec-add=\"keytag alg digesttype digest maxsiglife [keytag2 alg2...]\n"
        << "To input NULL value just pass ``NULL'' or ``-1''  string instead of some meaningful number or string\n\n"
        << "Example:\n"
        << "\t $ " << g_prog_name << " --keyset-update=\"KEY::002\" --dsrec-add=\"0 1 0 oJB1W6WNGv+ldvQ3WDG0MQkg5IEhjRip8WTr== NULL\"\n"
        << "This means, that keyset with handle ``KEY::002'' is updated. New DSRecord is added (with NULL value as maxSigLife)"
        << std::endl;
}
void
KeysetClient::delete_help()
{
    std::cout
        << "** Keyset delete **\n\n"
        << "  " << g_prog_name << " --keyset-delete=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --delete-keyset=\"KEY::100\"\n"
        << "This command delete keyset and all of its dsrecords assuming keyset it not assigned to any domain (use domain-update options\n"
        << "to set or unset keyset (not only) for domain"
        << std::endl;
}
void
KeysetClient::info_help()
{
    std::cout
        << "** Keyset info **\n\n"
        << "  " << g_prog_name << " --keyset-info=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --keyset-info=\"KEY::001\"\n"
        << "Command print on screen some informatins about keyset (identified by handle) such as handle, auth info password, roid,\n"
        << "tech contact handles, and dsrecord informations"
        << std::endl;
}
void
KeysetClient::check_help()
{
    std::cout
        << "** Keyset check **\n\n"
        << "  " << g_prog_name << " --keyset-check=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --keyset-check=\"K:02344\"\n"
        << "Command return 1 if keyset handle is free for registration and 0 if is already used in registry."
        << std::endl;
}

} //namespace Admin;
