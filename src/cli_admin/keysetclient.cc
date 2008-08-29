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
#include "keysetclient.h"

namespace Admin {

#define LOGIN_KEYSETCLIENT \
CorbaClient cc(0, NULL, m_nsAddr.c_str()); \
CORBA::Object_var o = cc.getNS()->resolve("EPP"); \
ccReg::EPP_var epp; \
epp = ccReg::EPP::_narrow(o); \
CORBA::Long clientId = 0; \
ccReg::Response_var r; \
if (!m_db.ExecSelect( \
            "SELECT r.handle,ra.cert,ra.password " \
            "FROM registrar r, registraracl ra " \
            "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ") \
        ) \
    return -1; \
if (!m_db.GetSelectRows()) \
    return -1; \
std::string handle = m_db.GetFieldValue(0,0); \
std::string cert = m_db.GetFieldValue(0,1); \
std::string password = m_db.GetFieldValue(0,2); \
m_db.FreeSelect(); \
r = epp->ClientLogin(handle.c_str(),password.c_str(),"","system_delete_login","<system_delete_login/>", \
        clientId,cert.c_str(),ccReg::EN); \
if (r->code != 1000 || !clientId) { \
    std::cerr << "Cannot connect: " << r->code << std::endl; \
    return -1; \
}

#define LOGOUT_KEYSETCLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

KeysetClient::KeysetClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "KeySet related options");
    m_options->add_options()
        ADD_OPT(KEYSET_LIST_NAME, "list of all keysets (via filters)")
        ADD_OPT(KEYSET_LIST_PLAIN_NAME, "list of all keysets (via ccReg_i)")
        ADD_OPT_TYPE(KEYSET_INFO_NAME, "keyset info (via epp_impl)", std::string)
        ADD_OPT_TYPE(KEYSET_INFO2_NAME, "keyset info (via ccReg_i::info method)", std::string)
        ADD_OPT_TYPE(KEYSET_CHECK_NAME, "check keyset state", std::string)
        ADD_OPT_TYPE(KEYSET_SEND_AUTH_INFO_NAME, "send authorization info", std::string)
        ADD_OPT_TYPE(KEYSET_CREATE_NAME, "create keyset", std::string)
        ADD_OPT_TYPE(KEYSET_DELETE_NAME, "delete keyset", std::string)
        ADD_OPT_TYPE(KEYSET_UPDATE_NAME, "update keyset", std::string)
        ADD_OPT_TYPE(KEYSET_TRANSFER_NAME, "transfer keyset", std::string)
        ADD_OPT(KEYSET_CREATE_HELP_NAME, "help for keyset creating")
        ADD_OPT(KEYSET_UPDATE_HELP_NAME, "help for keyset updating")
        ADD_OPT(KEYSET_DELETE_HELP_NAME, "help for keyset deleting")
        ADD_OPT(KEYSET_CHECK_HELP_NAME, "help for keyset checking")
        ADD_OPT(KEYSET_INFO_HELP_NAME, "help for keyset info");

    m_optionsInvis = new boost::program_options::options_description(
            "KeySet related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_TYPE(KEYSET_DSRECORDS_NAME, "list of dsrecords (create keyset)", std::string)
        ADD_OPT_DEF(KEYSET_DSREC_ADD_NAME, "list of dsrecords to add (update keyset)", std::string, "")
        ADD_OPT_DEF(KEYSET_DSREC_REM_NAME, "list of dsrecords to rem (update keyset)", std::string, "");
}

KeysetClient::KeysetClient(
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

    // CorbaClient cc(0, NULL, m_nsAddr.c_str());
    // CORBA::Object_var o = cc.getNS()->resolve("EPP");
    // m_clientId = 0;
    // ccReg::Response_var r;
    // if (!m_db.ExecSelect(
                // "SELECT r.handle, ra.cert, ra.password "
                // "FROM registrar r, registraracl ra "
                // "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ")
            // )
        // throw std::runtime_error("some weird sql error");
    // if (!m_db.GetSelectRows())
        // throw std::runtime_error("no rows in result");
    // std::string handle = m_db.GetFieldValue(0, 0);
    // std::string cert = m_db.GetFieldValue(0, 1);
    // std::string pass = m_db.GetFieldValue(0, 2);
// 
    // m_db.FreeSelect();
    // r = m_epp->ClientLogin(handle.c_str(), pass.c_str(), "", "system_delete_login",
            // "<system_detele_login/>", m_clientId, cert.c_str(), ccReg::EN);
    // if (r->code != 1000 || !m_clientId)
        // throw std::runtime_error("cannot connect");
}

KeysetClient::~KeysetClient()
{
    //m_epp->ClientLogout(m_clientId, "system_delete_logout", "<system_delete_logout/>");
}

void
KeysetClient::init(
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
KeysetClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
KeysetClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
KeysetClient::keyset_list()
{
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(&m_db, true));
    std::auto_ptr<Register::KeySet::List> keyList(
            keyMan->createList());

    Database::Filters::KeySet *keyFilter;
    keyFilter = new Database::Filters::KeySetHistoryImpl();
    if (m_varMap.count(ID_NAME))
        keyFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(HANDLE_NAME))
        keyFilter->addHandle().setValue(
                m_varMap[HANDLE_NAME].as<std::string>());

    if (m_varMap.count(CONTACT_ID_NAME))
        keyFilter->addTechContact().addId().setValue(
                Database::ID(m_varMap[CONTACT_ID_NAME].as<unsigned int>()));
    if (m_varMap.count(CONTACT_NAME_NAME))
        keyFilter->addTechContact().addName().setValue(
                m_varMap[CONTACT_NAME_NAME].as<std::string>());
    if (m_varMap.count(CONTACT_HANDLE_NAME))
        keyFilter->addTechContact().addHandle().setValue(
                m_varMap[CONTACT_HANDLE_NAME].as<std::string>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();
    
    unionFilter->addFilter(keyFilter);
    keyList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    keyList->reload(*unionFilter, m_dbman);
    std::cout << "<objects>" << std::endl;
    for (unsigned int i = 0; i < keyList->getCount(); i++) {
        std::cout
            << "\t<keyset>\n"
            << "\t\t<id>" << keyList->getKeySet(i)->getId() << "</id>\n"
            << "\t\t<handle>" << keyList->getKeySet(i)->getHandle() << "</handle>\n";
        for (unsigned int j = 0; j < keyList->getKeySet(i)->getAdminCount(); j++)
            std::cout
                << "\t\t<admin>" << keyList->getKeySet(i)->getAdminByIdx(j) << "</admin>\n";
        std::cout
            << "\t\t<dsrecord>\n";
        for (unsigned int j = 0; j < keyList->getKeySet(i)->getDSRecordCount(); j++)
            std::cout
                << "\t\t\t<id>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getId() << "</id>\n"
                << "\t\t\t<keytag>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getKeyTag() << "</keytag>\n"
                << "\t\t\t<alg>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getAlg() << "</alg>\n"
                << "\t\t\t<digesttype>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getDigestType() << "</digesttype>\n"
                << "\t\t\t<digest>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getDigest() << "</digest>\n"
                << "\t\t\t<maxsiglife>" << keyList->getKeySet(i)->getDSRecordByIdx(j)->getMaxSigLife() << "</maxsiglife>\n";
        std::cout
            << "\t\t</dsrecord>\n";
        std::cout
            << "\t</keyset>\n";
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    return 0;
}

int
KeysetClient::keyset_list_plain()
{
    std::string xml;
    std::string cltrid;

    xml = "<handle>" + m_varMap[KEYSET_LIST_PLAIN_NAME].as<std::string>() + "</handle>";
    cltrid = "list_keysets";
    ccReg::Lists *k;
    LOGIN_KEYSETCLIENT;

    r = epp->KeySetList(k, clientId, cltrid.c_str(), xml.c_str());

    if (r->code != 1000) {
        std::cerr << "An error has occured: " << r->code;
        return -1;
    }
    for (int i = 0; i < (int)k->length(); i++)
        std::cout << (*k)[i] << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_check()
{
    std::string xml;
    std::string cltrid;

    ccReg::CheckResp *a;
    
    std::string name = m_varMap[KEYSET_CHECK_NAME].as<std::string>().c_str();

    cltrid = "keyset_check";
    xml = "<handle>" + name + "</handle>";

    ccReg::Check names;
    names.length(1);
    names[0] = CORBA::string_dup(name.c_str());

    LOGIN_KEYSETCLIENT;

    r = epp->KeySetCheck(names, a, clientId, cltrid.c_str(), xml.c_str());

    std::cout << (*a)[0].avail << std::endl;
    std::cout << (*a)[0].reason << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_send_auth_info()
{
    std::string name = m_varMap[KEYSET_SEND_AUTH_INFO_NAME].as<std::string>().c_str();
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "keyset_send_auth_info";

    LOGIN_KEYSETCLIENT;
    r = epp->keysetSendAuthInfo(name.c_str(), clientId, cltrid.c_str(), xml.c_str());

    std::cout << r->code << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_transfer()
{
    std::string key_handle = m_varMap[KEYSET_TRANSFER_NAME].as<std::string>().c_str();
    std::string authinfopw = m_varMap[AUTH_INFO_PW_NAME].as<std::string>().c_str();

    std::string cltrid;
    std::string xml;

    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "transfer_keyset";

    LOGIN_KEYSETCLIENT;

    r = epp->KeySetTransfer(
            key_handle.c_str(),
            authinfopw.c_str(),
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << r->code << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_update()
{
    std::string key_handle = m_varMap[KEYSET_UPDATE_NAME].as<std::string>().c_str();
    std::string authinfopw = m_varMap[AUTH_INFO_PW_NAME].as<std::string>().c_str();
    std::string admins_add = m_varMap[ADMIN_ADD_NAME].as<std::string>().c_str();
    std::string admins_rem = m_varMap[ADMIN_REM_NAME].as<std::string>().c_str();
    std::string dsrec_add = m_varMap[KEYSET_DSREC_ADD_NAME].as<std::string>().c_str();
    std::string dsrec_rem = m_varMap[KEYSET_DSREC_REM_NAME].as<std::string>().c_str();

    std::vector<std::string> admins_add_list = separateSpaces(admins_add.c_str());
    std::vector<std::string> admins_rem_list = separateSpaces(admins_rem.c_str());

    std::vector<std::string> dsrec_add_list = separateSpaces(dsrec_add.c_str());
    std::vector<std::string> dsrec_rem_list = separateSpaces(dsrec_rem.c_str());

    ccReg::TechContact admin_add;
    admin_add.length(admins_add_list.size());
    for (int i = 0; i < (int)admins_add_list.size(); i++)
        admin_add[i] = CORBA::string_dup(admins_add_list[i].c_str());
    ccReg::TechContact admin_rem;
    admin_rem.length(admins_rem_list.size());
    for (int i = 0; i < (int)admins_rem_list.size(); i++)
        admin_rem[i] = CORBA::string_dup(admins_rem_list[i].c_str());

    if ((dsrec_add_list.size() % 5) != 0) {
        std::cout << "Bad number of ``dsrec-add'' items." << std::endl;
        return 1;
    }
    if ((dsrec_rem_list.size() % 5) != 0) {
        std::cout << "Bad number of ``dsrec-rem'' items." << std::endl;
        return 1;
    }
    ccReg::DSRecord dsrecord_add;
    dsrecord_add.length(dsrec_add_list.size() / 5);

    ccReg::DSRecord dsrecord_rem;
    dsrecord_rem.length(dsrec_rem_list.size() / 5);

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

    std::string cltrid;
    std::string xml;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_update";
    LOGIN_KEYSETCLIENT;

    r = epp->KeySetUpdate(
            key_handle.c_str(),
            authinfopw.c_str(),
            admin_add,
            admin_rem,
            dsrecord_add,
            dsrecord_rem,
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << "return code: " << r->code << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_delete()
{
    std::string key_handle = m_varMap[KEYSET_DELETE_NAME].as<std::string>().c_str();

    std::cout << key_handle << std::endl;

    std::string cltrid;
    std::string xml;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_delete";

    LOGIN_KEYSETCLIENT;

    r = epp->KeySetDelete(
            key_handle.c_str(),
            clientId,
            cltrid.c_str(),
            xml.c_str());
    std::cout << "return code: " << r->code << std::endl;
    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_create()
{
    std::string key_handle = m_varMap[KEYSET_CREATE_NAME].as<std::string>().c_str();
    std::string admins = m_varMap[ADMIN_NAME].as<std::string>().c_str();
    std::string authinfopw = m_varMap[AUTH_INFO_PW_NAME].as<std::string>().c_str();
    std::string dsrecords = m_varMap[KEYSET_DSRECORDS_NAME].as<std::string>().c_str();

    std::vector<std::string> admins_list = separateSpaces(admins.c_str());
    std::vector<std::string> dsrecords_list = separateSpaces(dsrecords.c_str());

    ccReg::TechContact admin;
    admin.length(admins_list.size());
    for (int i = 0; i < (int)admins_list.size(); i++)
        admin[i] = CORBA::string_dup(admins_list[i].c_str());
    
    if ((dsrecords_list.size() % 5) != 0) {
        std::cerr << "Bad nubmer of ``dsrecords'' items." << std::endl;
        exit(1);
    }
    ccReg::DSRecord dsrec;
    dsrec.length(dsrecords_list.size() / 5);

    for (int i = 0; i < (int)(dsrecords_list.size() / 5); i++) {
        dsrec[i].keyTag = atol(dsrecords_list[i * 5 + 0].c_str());
        dsrec[i].alg = atol(dsrecords_list[i * 5 + 1].c_str());
        dsrec[i].digestType = atol(dsrecords_list[i * 5 + 2].c_str());
        dsrec[i].digest = CORBA::string_dup(dsrecords_list[i * 5 + 3].c_str());
        dsrec[i].maxSigLife = atol(dsrecords_list[i * 5 + 4].c_str());
    }

    std::string cltrid;
    std::string xml;

    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "keyset_create";

    ccReg::timestamp crData;

    LOGIN_KEYSETCLIENT;

    r = epp->KeySetCreate(
            key_handle.c_str(),
            authinfopw.c_str(),
            admin,
            dsrec,
            crData,
            clientId,
            cltrid.c_str(), xml.c_str());

    std::cout << "return code: " << r->code << std::endl;

    LOGOUT_KEYSETCLIENT;
    return 0;
}

int
KeysetClient::keyset_info()
{
    std::string name = m_varMap[KEYSET_INFO_NAME].as<std::string>();
    std::string cltrid;
    std::string xml;
    xml = "<handle>" + name + "</handle>";
    cltrid = "info_keyset";

    ccReg::KeySet *k = new ccReg::KeySet;

    LOGIN_KEYSETCLIENT;
    epp->KeySetInfo(name.c_str(), k, clientId, cltrid.c_str(), xml.c_str());
    LOGOUT_KEYSETCLIENT;

    std::cout << k->handle << std::endl;
    std::cout << k->AuthInfoPw << std::endl;
    std::cout << k->ROID << std::endl;
    std::cout << k->dsrec[0].digest << std::endl;
    std::cout << k->tech[0] << std::endl;

    return 0;
}

int
KeysetClient::keyset_info2()
{
    std::string key_handle = m_varMap[KEYSET_INFO2_NAME].as<std::string>();
    std::string cltrid;
    std::string xml;

    CORBA::Long count;
    xml = "<handle>" + key_handle + "</handle>";
    cltrid = "info_keyset_2";
    ccReg::InfoType type = ccReg::IT_LIST_KEYSETS;

    LOGIN_KEYSETCLIENT;

    r = epp->info(
            type,
            key_handle.c_str(),
            count,
            clientId,
            cltrid.c_str(),
            xml.c_str());

    std::cout << "return code: " << r->code << std::endl;


    LOGOUT_KEYSETCLIENT;
    return 0;
}

void
KeysetClient::keyset_create_help()
{
    std::cout
        << "** KeySet create **\n\n"
        << "  " << g_prog_name << " --keyset-create=<keyset_handle> \\\n"
        << "    --admins=<list_of_admins> \\\n"
        << "    [--authinfopw=<authinfo_password>] \\\n"
        << "    --dsrecords=<list_of_dsrecords>\n\n"
        << "example:\n"
        << "\t$ " << g_prog_name << " --keyset-create2=\"KEY::234\" --admins=\"CON::100 CON::50\" --dsrecords=\"0 0 0 348975238 -1\"\n"
        << "Commmand create keyset with handle ``KEY::234'' and with admin contacts selected by handles (``CON::100'' and ``CON::50'')\n"
        << "and dsrecord with NULL value for maxSigLife (structure of dsrecord is: ``keytag alg digesttype digest maxsiglife'')\n"
        << "dsrecord can also look like: ``0 0 0 348975238 -1 0 0 1 sdfioure 1000'' to create two dsrecords"
        << std::endl;
}
void
KeysetClient::keyset_update_help()
{
    std::cout
        << "** KeySet update **\n\n"
        << "  " << g_prog_name << " --keyset-update=<keyset_handle> \\\n"
        << "    [--authinfopw=<new_authinfo_password>] \\\n"
        << "    [--admins_add=<list_of_admins_to_add>] \\\n"
        << "    [--admins_rem=<list_of_admins_to_rem>] \\\n"
        << "    [--dsrec-add=<list_of_dsrecords_to_add>] \\\n"
        << "    [--dsrec-rem=<list_of_dsrecords_to_rem>]\n\n"
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
KeysetClient::keyset_delete_help()
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
KeysetClient::keyset_info_help()
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
KeysetClient::keyset_check_help()
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
