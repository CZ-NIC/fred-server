#include "config.h"


#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include "log/logger.h"
#include "old_utils/dbsql.h"
#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "corba/nameservice.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"
#include "corba/ccReg.hh"

#include "simple.h"

#define STR(x) x.str().c_str()

#define LOGIN \
    std::stringstream nsAddr; \
nsAddr << varMap["nshost"].as<std::string>() << ":" << varMap["nsport"].as<unsigned int>(); \
CorbaClient cc(argc, argv, nsAddr.str()); \
CORBA::Object_var o = cc.getNS()->resolve("EPP"); \
ccReg::EPP_var epp; \
epp = ccReg::EPP::_narrow(o); \
CORBA::Long clientID = 0; \
ccReg::Response_var r; \
if (!db.ExecSelect( \
            "SELECT r.handle,ra.cert,ra.password " \
            "FROM registrar r, registraracl ra " \
            "WHERE r.id=ra.registrarid AND r.system='t' LIMIT 1 ") \
        ) \
    return -1; \
if (!db.GetSelectRows()) \
    return -1; \
std::string handle = db.GetFieldValue(0,0); \
std::string cert = db.GetFieldValue(0,1); \
std::string password = db.GetFieldValue(0,2); \
db.FreeSelect(); \
r = epp->ClientLogin(handle.c_str(),password.c_str(),"","system_delete_login","<system_delete_login/>", \
        clientID,cert.c_str(),ccReg::EN); \
if (r->code != 1000 || !clientID) { \
    std::cerr << "Cannot connect: " << r->code << std::endl; \
    return -1; \
}

#define LOGOUT \
    epp->ClientLogout(clientID,"system_delete_logout","<system_delete_logout/>");

using namespace boost::posix_time;

const char *corbaOpts[][2] = {
    {"nativeCharCodeSet", "UTF-8"},
    {NULL, NULL},
};

const char g_prog_name[]    = "simple-client";
const char g_version[]      = "0.1";

std::vector<std::string> separateSpaces(const char *string);

class simpleNS {
    CORBA::ORB_ptr m_orb;
    CosNaming::NamingContext_var m_rootContext;
    std::string m_hostname;
public:
    simpleNS(CORBA::ORB_ptr orb, const std::string &hostname)
    {
        try {
            CORBA::Object_var obj;
            // std::string str;
            // str = "corbaname::" + hostname;
            // std::cout << str << std::endl;
            obj = orb->string_to_object(("corbaname::" + hostname).c_str());
            m_rootContext = CosNaming::NamingContext::_narrow(obj);
            if (CORBA::is_nil(m_rootContext))
                std::cerr << "Nebezito" << std::endl;
        } catch (...) {
            std::cerr << "nebezi to" << std:: endl;
        }
    }
    ~simpleNS() {}
};

class simpleCC {
    CORBA::ORB_var m_orb;
    std::auto_ptr<simpleNS> m_ns;
public:
    simpleCC(int argc, char **argv, const std::string &nshost)
    {
        m_orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        m_ns.reset(new simpleNS(m_orb, nshost));
    }
    ~simpleCC()
    {
        m_orb->destroy();
    }
    simpleNS *getNS()
    {
        return m_ns.get();
    }
};

class CorbaClient {
    CORBA::ORB_var orb;
    std::auto_ptr<NameService> ns;
public:
    CorbaClient(int argc, char **argv, const std::string &nshost)
    {
        orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        ns.reset(new NameService(orb, nshost));
    }
    ~CorbaClient()
    {
        orb->destroy();
    }
    NameService *getNS()
    {
        return ns.get();
    }
};

void
list_domain()
{
}

int
main(int argc, char **argv)
{
    try {
    boost::program_options::options_description opts("Allowed options");
    opts.add_options()
        ("help,h",
         "print this help and quit")
        ("version,V",
         "print version and license information and quit")
        ("conf",
         boost::program_options::value<std::string>()->default_value(CONFIG_FILE),
         "configuration file")
        ("lang",
         boost::program_options::value<std::string>()->default_value("cs"),
         "communication language");

    boost::program_options::options_description invis("Invisible options");
    invis.add_options()
        ("moo",
         "moo");

    boost::program_options::options_description confOpts("Configuration options");
    confOpts.add_options()
        ("dbname", boost::program_options::value<std::string>()->default_value("fred"),
         "database name")
        ("dbuser", boost::program_options::value<std::string>()->default_value("fred"),
         "database user")
        ("dbpass", boost::program_options::value<std::string>(),
         "database password")
        ("dbhost", boost::program_options::value<std::string>()->default_value("localhost"),
         "database host address")
        ("dbport", boost::program_options::value<unsigned int>()->default_value(22345),
         "database port")
        ("nshost", boost::program_options::value<std::string>()->default_value("localhost"),
         "CORBA nameservice host")
        ("nsport", boost::program_options::value<unsigned int>()->default_value(22346),
         "CORBA nameservice port")
        ("log_level", boost::program_options::value<unsigned int>()->default_value(ERROR_LOG),
         "minimal level of logging")
        ("log_local", boost::program_options::value<unsigned int>()->default_value(2),
         "syslog local facility number");

    boost::program_options::options_description fileDesc("");
    fileDesc.add(confOpts);
    fileDesc.add_options()
        ("*", boost::program_options::value<std::string>(),
         "other-options");

    boost::program_options::options_description commOpts("Common options");
    commOpts.add_options()
        ("limit", boost::program_options::value<unsigned int>()->default_value(50),
         "limit for output");

    boost::program_options::options_description domOpts("Domain related options");
    domOpts.add_options()
        ("domain-list", "list of all domains")
        ("domain-id", boost::program_options::value<unsigned int>(),
         "list domain with specific id")
        ("domain-fqdn", boost::program_options::value<std::string>(),
         "list domain with specific fully qualified domain name")
        ("domain-nsset", boost::program_options::value<unsigned int>(),
         "list domain(s) with specific nsset id")
        ("domain-keyset", boost::program_options::value<unsigned int>(),
         "list domain(s) with specific keyset id")
        ("domain-keyset-handle", boost::program_options::value<std::string>(),
         "list domain(s) with specific keyset handle")
        ("domain-zone", boost::program_options::value<unsigned int>(),
         "list domain(s) with specific zone id")
        ("domain-reg", boost::program_options::value<unsigned int>(),
         "list domain(s) with specific registrant id")
        ("domain-anykeyset",
         "list all domains with keyset")
        ("domain-info", boost::program_options::value<std::string>(),
         "domain info")
        ("domain-list2",
         "list domains")
        ("domain-create", boost::program_options::value<std::string>(),
         "create domain")
        ("domain-update", boost::program_options::value<std::string>(),
         "update domain")
        ("domain-create-help",
         "help for creating domain")
        ("domain-update-help",
         "help for changing domain values");

    boost::program_options::options_description dcOpts("Domain and keyset create/update"
            " related options (dc=domain create, du=domain update, kc=keyset create, ku=keyset update)");
    dcOpts.add_options()
        ("registrant", boost::program_options::value<std::string>()->default_value(""),
         "domain registrant (dc, du)")
        ("nsset", boost::program_options::value<std::string>()->default_value(""),
         "domain nsset (dc, du)")
        ("keyset", boost::program_options::value<std::string>()->default_value(""),
         "domain keyset (dc, du)")
        ("authinfopw", boost::program_options::value<std::string>()->default_value(""),
         "password (dc, du)")
        ("period", boost::program_options::value<unsigned int>()->default_value(0),
         "period of domain validity in months (12 months equals 1 year ;) (dc)")
        ("admins", boost::program_options::value<std::string>()->default_value(""),
         "sequence of administration contacts (dc kc)")
        ("admins-add", boost::program_options::value<std::string>()->default_value(""),
         "seqeuence of added admin contacts (du, ku)")
        ("admins-rem", boost::program_options::value<std::string>()->default_value(""),
         "sequence of removed admin contacts (du, ku)")
        ("admins-rem-temp", boost::program_options::value<std::string>()->default_value(""),
         "sequence of removed temporary admin contact (du)")
        ("dsrecords", boost::program_options::value<std::string>(),
         "list of dsrecords (ku)")
        ("dsrec-keytag", boost::program_options::value<unsigned int>()->default_value(0),
         "keytag value for DSRecord (kc)")
        ("dsrec-alg", boost::program_options::value<unsigned int>()->default_value(0),
         "algorithm value for DSRecord (kc)")
        ("dsrec-digest-type", boost::program_options::value<unsigned int>()->default_value(0),
         "type of digest for DSRecord (kc)")
        ("dsrec-digest", boost::program_options::value<std::string>()->default_value(""),
         "digest for DSRecord (kc)")
        ("dsrec-max-sig-life", boost::program_options::value<unsigned int>(),
         "(kc)")
        ("dsrec-add", boost::program_options::value<std::string>()->default_value(""),
         "list of DSRecords to add to KeySet (ku)")
        ("dsrec-rem", boost::program_options::value<std::string>()->default_value(""),
         "list of DSRecords to rem to KeySet (ku)");

    boost::program_options::options_description keyOpts("KeySet related options");
    keyOpts.add_options()
        ("keyset-list",
         "list of all keysets (via filters)")
        ("keyset-id", boost::program_options::value<unsigned int>(),
         "list keyset with specific id")
        ("keyset-contact", boost::program_options::value<unsigned int>(),
         "list keyset with specific tech contact")
        ("keyset-contact-name", boost::program_options::value<std::string>(),
         "list keyset with specific tech contact name")
        ("keyset-contact-handle", boost::program_options::value<std::string>(),
         "list keyset with specific tech contact handle")
        ("keyset-handle", boost::program_options::value<std::string>(),
         "list keyset with specific handle")
        ("keyset-fqdn", boost::program_options::value<std::string>(),
         "list keyset with specific domain fqdn")
        ("keyset-info", boost::program_options::value<std::string>(),
         "keyset info from epp_impl")
        ("keyset-info2", boost::program_options::value<std::string>(),
         "keyset info via ccReg_i:info method")
        ("keyset-check", boost::program_options::value<std::string>(),
         "check keyset existence")
        ("keyset-list2", 
         "list of all keysets (via ``old style'' backend function)")
        ("keyset-send-auth-info", boost::program_options::value<std::string>(),
         "send authrization info")
        ("keyset-create", boost::program_options::value<std::string>(),
         "create keyset")
        ("keyset-create2", boost::program_options::value<std::string>(),
         "create keyset 2")
        ("keyset-delete", boost::program_options::value<std::string>(),
         "delete keyset")
        ("keyset-update", boost::program_options::value<std::string>(),
         "update keyset")
        ("keyset-transfer", boost::program_options::value<std::string>(),
         "transfer keyset")
        ("keyset-update-help",
         "help for keyset update")
        ("keyset-delete-help",
         "help for keyset delete")
        ("keyset-check-help",
         "help for keyset check")
        ("keyset-info-help",
         "help for keyset info")
        ("keyset-create-help",
         "help for creating")
        ("reload",
         "something stupid, can be used with ``handle-filter'' and ``admin-filter''")
        ("handle-filter", boost::program_options::value<std::string>(),
         "filter for handler")
        ("admin-filter", boost::program_options::value<std::string>(),
         "filter for admin contact")
        ("keyset-by-handle", boost::program_options::value<std::string>(),
         "get keyset detail by ccReg_admin_i::getKeySetDetail(char *) method");

    boost::program_options::options_description conOpts("Contact related options");
    conOpts.add_options()
        ("contact-list", "list of all contacts")
        ("contact-id", boost::program_options::value<unsigned int>(),
         "list contact with specific id")
        ("contact-handle", boost::program_options::value<std::string>(),
         "list contact with specifig handle")
        ("contact-info", boost::program_options::value<std::string>(),
         "detailed contact info");

    boost::program_options::variables_map varMap;

    boost::program_options::options_description all("All allowed options");
    all.add(opts).add(invis).add(confOpts).
        add(commOpts).add(domOpts).add(dcOpts).
        add(keyOpts).add(conOpts);

    boost::program_options::options_description visible("Allowed options");
    visible.add(opts).add(confOpts).add(commOpts).
        add(domOpts).add(dcOpts).add(keyOpts).add(conOpts);


    boost::program_options::store(
            boost::program_options::parse_command_line(
                argc,
                argv,
                all
                ),
            varMap);


    std::ifstream configFile(varMap["conf"].as<std::string>().c_str());
    boost::program_options::store(
            boost::program_options::parse_config_file(
                configFile,
                fileDesc),
            varMap);

    if (varMap.count("help") || argc == 1) {
        std::cout << visible << std::endl;
        exit(0);
    }
    if (varMap.count("version")) {
        print_version();
        exit(0);
    }
    if (varMap.count("moo")) {
        print_moo();
        exit(0);
    }

    // SysLogger::get().setLevel(
            // varMap["log_level"].as<unsigned int>());
    // SysLogger::get().setFacility(
            // varMap["log_local"].as<unsigned int>());    

    Logging::Manager::instance_ref().get("tracer").addHandler(Logging::Log::LT_SYSLOG);
    Logging::Manager::instance_ref().get("tracer").setLevel(Logging::Log::LL_TRACE);
    Logging::Manager::instance_ref().get("db").addHandler(Logging::Log::LT_SYSLOG);
    Logging::Manager::instance_ref().get("db").setLevel(Logging::Log::LL_DEBUG);    

    std::stringstream connstring;

    connstring << "dbname=" << varMap["dbname"].as<std::string>() 
               << " user=" << varMap["dbuser"].as<std::string>();
    if (varMap.count("dbhost"))
        connstring << " host=" << varMap["dbhost"].as<std::string>();
    if (varMap.count("dbport"))
        connstring << " port=" << varMap["dbport"].as<unsigned int>();
    if (varMap.count("dbpass"))
        connstring << " password=" << varMap["dbpass"].as<std::string>();


    Database::Manager *dbman = new Database::Manager(connstring.str()); // = (Database::Manager *)new 
        //Database::PSQLManager(connstring.str());
    DB db;
    if (!db.OpenDatabase(connstring.str().c_str())) {
        std::cout << "Database connection error (" << connstring << ")" << std::endl;
      return -1;
    }

    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&db));


    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    if (varMap.count("keyset-list") || varMap.count("keyset-id") ||
            varMap.count("keyset-contact") || varMap.count("keyset-contact-name") ||
            varMap.count("keyset-handle") || varMap.count("keyset-contact-handle") ||
            varMap.count("keyset-fqdn")
            )
    {
        std::auto_ptr<Register::KeySet::Manager> keyMan(
                Register::KeySet::Manager::create(&db, true));
        std::auto_ptr<Register::KeySet::List> keyList(
                keyMan->createList());

        Database::Filters::KeySet *keyFilter;//=
        keyFilter = new Database::Filters::KeySetHistoryImpl();

        if (varMap.count("keyset-id"))
            keyFilter->addId().setValue(
                    Database::ID(varMap["keyset-id"].as<unsigned int>()));
        if (varMap.count("keyset-contact"))
            keyFilter->addTechContact().addId().setValue(
                    Database::ID(varMap["keyset-contact"].as<unsigned int>()));
        if (varMap.count("keyset-contact-name"))
            keyFilter->addTechContact().addName().setValue(
                    varMap["keyset-contact-name"].as<std::string>());
        if (varMap.count("keyset-handle"))
            keyFilter->addHandle().setValue(
                    varMap["keyset-handle"].as<std::string>());
        if (varMap.count("keyset-contact-handle"))
            keyFilter->addTechContact().addHandle().setValue(
                    varMap["keyset-contact-handle"].as<std::string>());
        // if (varMap.count("keyset-fqdn"))
            // keyFilter->addDomain().addFQDN().setValue(
                    // varMap["keyset-fqdn"].as<std::string>());

        unionFilter->addFilter(keyFilter);

        keyList->setLimit(varMap["limit"].as<unsigned int>());
        keyList->reload(*unionFilter, dbman);
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
    } else if (varMap.count("reload")) {
        std::auto_ptr<Register::KeySet::Manager> keyMan(
                Register::KeySet::Manager::create(&db, true));
        std::auto_ptr<Register::KeySet::List> keyList(
                keyMan->createList());

        if (varMap.count("handle-filter"))
            keyList->setHandleFilter(varMap["handle-filter"].as<std::string>());
        if (varMap.count("admin-filter"))
            keyList->setAdminFilter(varMap["admin-filter"].as<std::string>());

        keyList->reload();

    } else if (varMap.count("contact-info")) {
        LOGIN;

        std::string name = varMap["contact-info"].as<std::string>();
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<name>" << name << "</name>";
        std::cout << xml.str() << std::endl;
        cltrid << "info_contact";

        ccReg::Contact *c = new ccReg::Contact;
        
        epp->ContactInfo(name.c_str(), c, clientID, STR(cltrid), STR(xml));

        std::cout << c->Name << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-check")) {
        LOGIN;

        ccReg::CheckResp *a;
        std::stringstream cltrid;
        std::stringstream xml;
        
        std::string name = varMap["keyset-check"].as<std::string>().c_str();

        cltrid << "keyset_check";
        xml << "<name>" << name << "</name>";

        ccReg::Check names;
        names.length(1);
        names[0] = CORBA::string_dup(name.c_str());

        r = epp->KeySetCheck(names, a, clientID, STR(cltrid), STR(xml));

        std::cout << (*a)[0].avail << std::endl;
        std::cout << (*a)[0].reason << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-send-auth-info")) {
        LOGIN;

        std::string name = varMap["keyset-send-auth-info"].as<std::string>().c_str();
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<handle>" << name << "</handle>";
        cltrid << "keyset_send_auth_info";

        r = epp->keysetSendAuthInfo(name.c_str(), clientID, STR(cltrid), STR(xml));

        std::cout << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-by-handle")) {
        std::cout << "keyset-by-handle" << std::endl;
        Conf *cfg = new Conf();
        std::stringstream nsAddr;
        nsAddr << varMap["nshost"].as<std::string>() << ":" << varMap["nsport"].as<unsigned int>();
        //CorbaClient cc(argc, argv, nsAddr.str());
        //ccReg_Admin_i *admin;
        //ccReg_Admin_i adminn(connstring.str(),cc.getNS(), *cfg, false);
        // ccReg_Admin_i *admin = new ccReg_Admin_i(
                // connstring.str(),
                // cc.getNS(),
                // *cfg,
                // false);

        //delete admin;
    } else if (varMap.count("domain-list2")) {
        LOGIN;
        std::string name = varMap["domain-list2"].as<std::string>().c_str();
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<name>" << name << "</name>";
        cltrid << "list_domains";

        ccReg::Lists *k;

        r = epp->DomainList(k, clientID, STR(cltrid), STR(xml));

        for (int i = 0; i < (int)k->length(); i++)
            std::cout << (*k)[i] << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-transfer")) {
        std::string key_handle = varMap["keyset-transfer"].as<std::string>().c_str();
        std::string authinfopw = varMap["authinfopw"].as<std::string>().c_str();
        std::stringstream cltrid;
        std::stringstream xml;

        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "transfer_keyset";

        LOGIN;

        r = epp->KeySetTransfer(
                key_handle.c_str(),
                authinfopw.c_str(),
                clientID,
                STR(cltrid),
                STR(xml));
        std::cout << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-list2")) {
        LOGIN;
        std::string name = varMap["keyset-list2"].as<std::string>().c_str();
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<name>" << name << "</name>";
        cltrid << "list_keysets";

        ccReg::Lists *k;

        r = epp->KeySetList(k, clientID, STR(cltrid), STR(xml));

        for (int i = 0; i < (int)k->length(); i++)
            std::cout << (*k)[i] << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-update")) {
        std::string key_handle = varMap["keyset-update"].as<std::string>().c_str();
        std::string authinfopw = varMap["authinfopw"].as<std::string>().c_str();
        std::string admins_add = varMap["admins-add"].as<std::string>().c_str();
        std::string admins_rem = varMap["admins-rem"].as<std::string>().c_str();
        std::string dsrec_add = varMap["dsrec-add"].as<std::string>().c_str();
        std::string dsrec_rem = varMap["dsrec-rem"].as<std::string>().c_str();

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

        LOGIN;
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "keyset_update";

        r = epp->KeySetUpdate(
                key_handle.c_str(),
                authinfopw.c_str(),
                admin_add,
                admin_rem,
                dsrecord_add,
                dsrecord_rem,
                clientID,
                STR(cltrid),
                STR(xml));
        std::cout << "return code: " << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-delete")) {
        std::string key_handle = varMap["keyset-delete"].as<std::string>().c_str();

        std::cout << key_handle << std::endl;

        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "keyset_delete";

        LOGIN;

        r = epp->KeySetDelete(
                key_handle.c_str(),
                clientID,
                STR(cltrid),
                STR(xml));
        std::cout << "return code: " << r->code << std::endl;
        LOGOUT;
    } else if (varMap.count("keyset-update-help")) {
        keyset_update_help();
    } else if (varMap.count("keyset-create-help")) {
        keyset_create_help();
    } else if (varMap.count("keyset-delete-help")) {
        keyset_delete_help();
    } else if (varMap.count("keyset-info-help")) {
        keyset_info_help();
    } else if (varMap.count("keyset-check-help")) {
        keyset_check_help();
    } else if (varMap.count("keyset-create")) {
        std::string key_handle = varMap["keyset-create"].as<std::string>().c_str();
        std::string admins = varMap["admins"].as<std::string>().c_str();
        std::string authinfopw = varMap["authinfopw"].as<std::string>().c_str();
        std::string dsrecords = varMap["dsrecords"].as<std::string>().c_str();

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

        std::stringstream cltrid;
        std::stringstream xml;

        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "keyset_create";

        ccReg::timestamp crData;

        LOGIN;

        r = epp->KeySetCreate(key_handle.c_str(), authinfopw.c_str(),
                admin, dsrec, crData, clientID,
                STR(cltrid), STR(xml));

        std::cout << "return code: " << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-create2")) {
        std::string key_handle = varMap["keyset-create2"].as<std::string>().c_str();
        std::string admins = varMap["admins"].as<std::string>().c_str();
        std::string authInfoPw = varMap["authinfopw"].as<std::string>().c_str();
        unsigned int keytag = varMap["dsrec-keytag"].as<unsigned int>();
        unsigned int alg = varMap["dsrec-alg"].as<unsigned int>();
        unsigned int digestType = varMap["dsrec-digest-type"].as<unsigned int>();
        std::string digest = varMap["dsrec-digest"].as<std::string>().c_str();
        int maxSigLife = -1;
        if (varMap.count("dsrec-max-sig-life"))
            maxSigLife = (int)varMap["dsrec-max-sig-life"].as<unsigned int>();

        bool empty_param = false;
        if (admins.empty()) {
            std::cerr << "Parameter ``admins'' is not allowed to be empty!" << std::endl;
            empty_param = true;
        }
        if (empty_param)
            exit(1);

        std::cout <<  key_handle << std::endl;
        std::cout <<  admins << std::endl;
        std::cout <<  keytag << std::endl;
        std::cout <<  alg << std::endl;
        std::cout <<  digestType << std::endl;
        std::cout <<  digest << std::endl;
        std::cout << maxSigLife << std::endl;

        ccReg::DSRecord dsrec;
        //TODO allow more than one ds record - see keyset-create
        dsrec.length(1);
        dsrec[0].keyTag = keytag;
        dsrec[0].alg = alg;
        dsrec[0].digestType = digestType;
        dsrec[0].digest = CORBA::string_dup(digest.c_str());
        dsrec[0].maxSigLife = maxSigLife;

        /* lets get separated admin contact from its list */
        std::vector<std::string> admins_list;
        char *tok;
        char *str;
        str = (char *)std::malloc(sizeof(char) * (admins.length()));
        std::strcpy(str, admins.c_str());
        /* list of admin contact is seperated with spaces */
        tok = std::strtok(str, " ");
        while (tok != NULL) {
            admins_list.push_back(std::string(tok));
            tok = std::strtok(NULL, " ");
        }
        std::free(str);

        ccReg::TechContact admin;
        admin.length(admins_list.size());
        for (int i = 0; i < (int)admins_list.size(); i++)
            admin[i] = CORBA::string_dup(admins_list[i].c_str());

        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "keyset_create";

        ccReg::timestamp crDate;
        LOGIN;

        r = epp->KeySetCreate(key_handle.c_str(), authInfoPw.c_str(),
                admin, dsrec, crDate, 
                clientID, STR(cltrid), STR(xml));

        std::cout << "return code: " << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("domain-create-help")) {
        domain_create_help();
    } else if (varMap.count("domain-update-help")) {
        domain_update_help();
    } else if (varMap.count("domain-create")) {
        std::string fqdn = varMap["domain-create"].as<std::string>().c_str();
        std::string registrant = varMap["registrant"].as<std::string>().c_str();
        std::string nsset = varMap["nsset"].as<std::string>().c_str();
        std::string keyset = varMap["keyset"].as<std::string>().c_str();
        std::string authInfoPw = varMap["authinfopw"].as<std::string>().c_str();
        std::string admins = varMap["admins"].as<std::string>().c_str();
        unsigned int period = varMap["period"].as<unsigned int>();

        bool empty_param = false;
        if (registrant.empty()) {
            std::cerr << "Parameter ``registrant'' is not allowed to be empty!" << std::endl;
            empty_param = true;
        }
        if (period == 0) {
            std::cerr << "Parameter ``period'' must be number bigger (and not equal) than zero!" << std::endl;
            empty_param = true;
        }
        if (admins.empty()) {
            std::cerr << "Parameter ``admins'' is not allowed to be empty!" << std::endl;
            empty_param = true;
        }
        if (empty_param)
            exit(1);

        /* lets get separated admin contact from its list */
        std::vector<std::string> admins_list;
        char *tok;
        char *str;
        str = (char *)std::malloc(sizeof(char) * (admins.length()));
        std::strcpy(str, admins.c_str());
        /* list of admin contact is seperated with spaces */
        tok = std::strtok(str, " ");
        while (tok != NULL) {
            admins_list.push_back(std::string(tok));
            tok = std::strtok(NULL, " ");
        }
        std::free(str);

        ccReg::AdminContact admin;
        admin.length(admins_list.size());
        for (int i = 0; i < (int)admins_list.size(); i++)
            admin[i] = CORBA::string_dup(admins_list[i].c_str());


        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<fqdn>" << fqdn << "</fqdn>";
        cltrid << "domain_create";

        ccReg::Period_str period_str;
        period_str.count = (short int)period;
        period_str.unit = ccReg::unit_month;

        ccReg::timestamp crDate;
        ccReg::timestamp exDate;

        ccReg::ExtensionList extList;
        extList.length(0);

        LOGIN;

        r = epp->DomainCreate(fqdn.c_str(), registrant.c_str(),
                nsset.c_str(), keyset.c_str(), authInfoPw.c_str(),
                period_str, admin, crDate, exDate,
                clientID, STR(cltrid), STR(xml), extList);

        std::cout << "return code: " << r->code << std::endl;

        LOGOUT;
    } else if (varMap.count("domain-update")) {
        std::string fqdn = varMap["domain-update"].as<std::string>();
        std::string registrant = varMap["registrant"].as<std::string>();
        std::string nsset = varMap["nsset"].as<std::string>();
        std::string keyset = varMap["keyset"].as<std::string>();
        std::string authinfopw = varMap["authinfopw"].as<std::string>();
        std::string admins_add = varMap["admins-add"].as<std::string>();
        std::string admins_rem = varMap["admins-rem"].as<std::string>();
        std::string admins_rem_temp = varMap["admins-rem-temp"].as<std::string>();

        std::vector<std::string> admins_add_list;
        std::vector<std::string> admins_rem_list;
        std::vector<std::string> admins_rem_temp_list;
        /* lets get separated admin contact from its list */
        char *tok;
        char *str;
        str = (char *)std::malloc(sizeof(char) * (admins_add.length()));
        std::strcpy(str, admins_add.c_str());
        /* list of admin contact is seperated with spaces */
        tok = std::strtok(str, " ");
        while (tok != NULL) {
            admins_add_list.push_back(std::string(tok));
            tok = std::strtok(NULL, " ");
        }
        std::free(str);

        str = (char *)std::malloc(sizeof(char) * (admins_rem.length()));
        std::strcpy(str, admins_rem.c_str());
        /* list of admin contact is seperated with spaces */
        tok = std::strtok(str, " ");
        while (tok != NULL) {
            admins_rem_list.push_back(std::string(tok));
            tok = std::strtok(NULL, " ");
        }
        std::free(str);

        str = (char *)std::malloc(sizeof(char) * (admins_rem_temp.length()));
        std::strcpy(str, admins_rem_temp.c_str());
        /* list of admin contact is seperated with spaces */
        tok = std::strtok(str, " ");
        while (tok != NULL) {
            admins_rem_temp_list.push_back(std::string(tok));
            tok = std::strtok(NULL, " ");
        }
        std::free(str);

        ccReg::AdminContact admin_add;
        ccReg::AdminContact admin_rem;
        ccReg::AdminContact admin_rem_temp;

        admin_add.length(admins_add_list.size());
        for (int i = 0; i < (int)admins_add_list.size(); i++)
            admin_add[i] = CORBA::string_dup(admins_add_list[i].c_str());
        admin_rem.length(admins_rem_list.size());
        for (int i = 0; i < (int)admins_rem_list.size(); i++)
            admin_rem[i] = CORBA::string_dup(admins_rem_list[i].c_str());
        admin_rem_temp.length(admins_rem_temp_list.size());
        for (int i = 0; i < (int)admins_rem_temp_list.size(); i++)
            admin_rem_temp[i] = CORBA::string_dup(admins_rem_temp_list[i].c_str());

        ccReg::ExtensionList extList;
        extList.length(0);

        LOGIN;

        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<fqdn>" << fqdn << "</fqdn>";
        cltrid << "domain_update";

        r = epp->DomainUpdate(fqdn.c_str(), registrant.c_str(),
                authinfopw.c_str(), nsset.c_str(), keyset.c_str(), 
                admin_add, admin_rem, admin_rem_temp,
                clientID, STR(cltrid), STR(xml), extList);

        std::cout << "return code: " << r->code << std::endl;
        LOGOUT;
    } else if (varMap.count("domain-info")) {
        std::string name = varMap["domain-info"].as<std::string>();

        LOGIN;
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<name>" << name << "</name>";
        cltrid << "info_keyset";

        ccReg::Domain *k = new ccReg::Domain;
        epp->DomainInfo(name.c_str(), k, clientID, STR(cltrid), STR(xml));

        std::cout << k->name << std::endl;
        std::cout << k->AuthInfoPw << std::endl;
        std::cout << k->ROID << std::endl;
        std::cout << k->keyset << std::endl;

        LOGOUT;
    } else if (varMap.count("keyset-info2")) {
        std::string key_handle = varMap["keyset-info2"].as<std::string>();
        std::stringstream cltrid;
        std::stringstream xml;

        CORBA::Long count;
        xml << "<handle>" << key_handle << "</handle>";
        cltrid << "info_keyset_2";
        ccReg::InfoType type = ccReg::IT_LIST_KEYSETS;

        LOGIN;

        r = epp->info(type, key_handle.c_str(), count,
                clientID, STR(cltrid), STR(xml));

        std::cout << "return code: " << r->code << std::endl;


        LOGOUT;
    } else if (varMap.count("keyset-info")) {
        LOGIN;
        std::string name = varMap["keyset-info"].as<std::string>();
        std::stringstream cltrid;
        std::stringstream xml;
        xml << "<handle>" << name << "</handle>";
        cltrid << "info_keyset";

        ccReg::KeySet *k = new ccReg::KeySet;
        epp->KeySetInfo(name.c_str(), k, clientID, STR(cltrid), STR(xml));

        std::cout << k->handle << std::endl;
        std::cout << k->AuthInfoPw << std::endl;
        std::cout << k->ROID << std::endl;
        std::cout << k->dsrec[0].digest << std::endl;
        std::cout << k->tech[0] << std::endl;

        LOGOUT;
    } else if (varMap.count("contact-list") || varMap.count("contact-id") ||
            varMap.count("contact-handle")
            ) {
        std::auto_ptr<Register::Contact::Manager> conMan(
                Register::Contact::Manager::create(&db, true));
        std::auto_ptr<Register::Contact::List> conList(
                conMan->createList());

        Database::Filters::Contact *conFilter;
        conFilter = new Database::Filters::ContactHistoryImpl();

        if (varMap.count("contact-id"))
            conFilter->addId().setValue(
                    Database::ID(varMap["contact-id"].as<unsigned int>()));

        if (varMap.count("contact-handle"))
            conFilter->addHandle().setValue(
                    varMap["contact-handle"].as<std::string>());

        unionFilter->addFilter(conFilter);

        conList->setLimit(varMap["limit"].as<unsigned int>());
        conList->reload(*unionFilter, dbman);
        std::cout << "<objects>" << std::endl;
        for (unsigned int i = 0; i < conList->getCount(); i++) {
            std::cout
                << "\t<contact>\n"
                << "\t\t<id>" << conList->getContact(i)->getId() << "</id>\n"
                << "\t\t<handle>" << conList->getContact(i)->getHandle() << "</handle>\n"
                << "\t\t<name>" << conList->getContact(i)->getName() << "</name>\n"
                << "\t\t<street1>" << conList->getContact(i)->getStreet1() << "</street1>\n"
                << "\t\t<street2>" << conList->getContact(i)->getStreet2() << "</street2>\n"
                << "\t\t<street3>" << conList->getContact(i)->getStreet3() << "</street3>\n"
                << "\t\t<city>" << conList->getContact(i)->getCity() << "</city>\n"
                << "\t\t<postal_code>" << conList->getContact(i)->getPostalCode() << "</postal_code>\n"
                << "\t\t<province>" << conList->getContact(i)->getProvince() << "</province>\n"
                << "\t\t<country>" << conList->getContact(i)->getCountry() << "</country>\n"
                << "\t\t<telephone>" << conList->getContact(i)->getTelephone() << "</telephone>\n"
                << "\t\t<fax>" << conList->getContact(i)->getFax() << "</fax>\n"
                << "\t\t<email>" << conList->getContact(i)->getEmail() << "</email>\n"
                << "\t\t<notify_email>" << conList->getContact(i)->getNotifyEmail() << "</notify_email>\n"
                << "\t</contact>\n";
        }
        std::cout << "</object>" << std::endl;
        unionFilter->clear();
    } else if (varMap.count("domain-list") || varMap.count("domain-keyset") ||
            varMap.count("domain-nsset") || varMap.count("domain-zone") ||
            varMap.count("domain-reg") || varMap.count("domain-anykeyset") ||
            varMap.count("domain-id") || varMap.count("domain-fqdn") ||
            varMap.count("domain-keyset-handle")
            ) 
    {
        std::auto_ptr<Register::Domain::Manager> domMan(
                Register::Domain::Manager::create(&db, zoneMan.get()));
        std::auto_ptr<Register::Domain::List> domList(
                domMan->createList());

        Database::Filters::Domain *domFilter;
        domFilter = new Database::Filters::DomainHistoryImpl();

        if (varMap.count("domain-keyset"))
            domFilter->addKeySetId().setValue(
                    Database::ID(varMap["domain-keyset"].as<unsigned int>()));

        if (varMap.count("domain-keyset-handle"))
            domFilter->addKeySet().addHandle().setValue(
                    varMap["domain-keyset-handle"].as<std::string>());

        if (varMap.count("domain-nsset"))
            domFilter->addNSSetId().setValue(
                    Database::ID(varMap["domain-nsset"].as<unsigned int>()));

        if (varMap.count("domain-zone"))
            domFilter->addZoneId().setValue(
                    Database::ID(varMap["domain-zone"].as<unsigned int>()));

        if (varMap.count("domain-reg"))
            domFilter->addRegistrantId().setValue(
                    Database::ID(varMap["domain-reg"].as<unsigned int>()));

        if (varMap.count("domain-id"))
            domFilter->addId().setValue(
                    Database::ID(varMap["domain-id"].as<unsigned int>()));

        if (varMap.count("domain-fqdn"))
            domFilter->addFQDN().setValue(
                    varMap["domain-fqdn"].as<std::string>());
        
        if (varMap.count("domain-anykeyset"))
            domFilter->addKeySet();

        //domFilter->addKeySet();

        unionFilter->addFilter(domFilter);


        domList->setLimit(varMap["limit"].as<unsigned int>());
        domList->reload(*unionFilter, dbman);
        std::cout << "<objects>" << std::endl;
        for (unsigned int i = 0; i < domList->getCount(); i++) {
            std::cout
                << "\t<domain>\n"
                << "\t\t<id>" << domList->getDomain(i)->getId() << "</id>\n"
                << "\t\t<fqdn>" << domList->getDomain(i)->getFQDN() << "</fqdn>\n"
                << "\t\t<zone>" << domList->getDomain(i)->getZoneId() << "</zone>\n"
                << "\t\t<nsset>" << domList->getDomain(i)->getNSSetId() << "</nsset>\n"
                << "\t\t<nsset_h>" << domList->getDomain(i)->getNSSetHandle() << "</nsset_h>\n"
                << "\t\t<keyset>" << domList->getDomain(i)->getKeySetId() << "</keyset>\n"
                << "\t\t<keyset_h>" << domList->getDomain(i)->getKeySetHandle() << "</keyset_h>\n"
                << "\t\t<registrant>" << domList->getDomain(i)->getRegistrantId() << "</registrant>\n"
                << "\t\t<registrant_h>" << domList->getDomain(i)->getRegistrantHandle() << "</registrant_h>\n";
            for (unsigned int j = 0; j < domList->getDomain(i)->getAdminCount(); j++)
                std::cout
                    << "\t\t<admin>" << domList->getDomain(i)->getAdminIdByIdx(j) << "</admin>\n"
                    << "\t\t<admin_h>" << domList->getDomain(i)->getAdminHandleByIdx(j) << "</admin_h>\n";
            std::cout
                << "\t\t<exp_date>" << domList->getDomain(i)->getExpirationDate() << "</exp_date>\n"
                << "\t</domain>\n";
        }
        std::cout << "</object>" << std::endl;
    }
    //std::cout << "</objects>\n" << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    return 2;
    }
    catch (Register::SQL_ERROR) {
        std::cerr << "SQL ERROR \n";
    return 1;
    }

    return 0;
}

std::vector<std::string>
separateSpaces(const char *string)
{
    std::vector<std::string> ret;
    char *tok;
    char *str;

    if ((str = (char *)std::malloc(sizeof(char) * std::strlen(string))) == NULL) {
        std::cerr << "Cannot alocate memory (separateSpace)" << std::endl;
        exit(1);
    }
    if ((str = std::strcpy(str, string)) == NULL) {
        std::cerr << "Cannot alocate memory (separateSpace)" << std::endl;
        exit(1);
    }

    tok = std::strtok(str, " ");
    while (tok != NULL) {
        ret.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);
    
    return ret;
}
void
print_version()
{
    std::cout
        << g_prog_name << " version " << g_version << "\nFor list of all options type: ``$ "
        << g_prog_name << " --help''\n\n"
        << "Copyright (c) 2008 CZ.NIC z.s.p.o.\n"
        << "This file is part of FRED (for details see fred.nic.cz)\n"
        << "Licence information:\n"
        << "FRED is free software: you can redistribute it and/or modify\n"
        << "it under the terms of the GNU General Public License as published by\n"
        << "the Free Software Foundation, version 2 of the License.\n\n"
        << "FRED is distributed in the hope that it will be useful,\n"
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        << "GNU General Public License for more details.\n\n"
        << "You should have received a copy of the GNU General Public License\n"
        << "along with FRED.  If not, see <http://www.gnu.org/licenses/>."
        << std::endl;
}
void
print_moo()
{
    std::cout
        << "         (__) \n"
        << "         (oo) \n"
        << "   /------\\/ \n"
        << "  / |    ||   \n"
        << " *  /\\---/\\ \n"
        << "    ~~   ~~   \n"
        << "....\"Have you mooed today?\"..."
        << std::endl;
}
void
keyset_create_help()
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
keyset_update_help()
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
keyset_delete_help()
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
keyset_info_help()
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
keyset_check_help()
{
    std::cout
        << "** Keyset check **\n\n"
        << "  " << g_prog_name << " --keyset-check=<keyset_handle>\n\n"
        << "Example:\n"
        << "\t$ " << g_prog_name << " --keyset-check=\"K:02344\"\n"
        << "Command return 1 if keyset handle is free for registration and 0 if is already used in registry."
        << std::endl;
}

void
domain_update_help()
{
    std::cout
        << "** Domain update **\n\n"
        << "  " << g_prog_name << " --domain-update=<domain_fqdn> \\\n"
        << "    [--registrant=<registrant_handle> \\\n"
        << "    [--nsset=<new_nsset>] \\\n"
        << "    [--keyset=<new_keyset>] \\\n"
        << "    [--authinfopw=<new_authinfo_password>] \\\n"
        << "    [--admins-add=<list_of_admins_to_add>] \\\n"
        << "    [--admins-rem=<list_of_admins_to_rem>] \\\n"
        << "    [--admins-rem-temp=<list_of_temp_admins_to_rem>]"
        << std::endl;
}

void
domain_create_help()
{
    std::cout
        << "** Domain create **\n\n"
        << "  " << g_prog_name << " --domain-create=<domain_fqdn> \\\n"
        << "    --registrant=<registrant_handle> \\\n"
        << "    [--nsset=<nsset_handle>] \\\n"
        << "    [--keyset=<keyset_handle>] \\\n"
        << "    [--authinfopw=<authinfo_password>] \\\n"
        << "    --admins=<list_of_admins_contact_handles> \\\n"
        << "    --period=<period_in_months>\n\n"
        << "Domain creation example:\n"
        << "\t$ " << g_prog_name << " --domain-create=example.cz "
        << "--admins=\"CON::001 CON::005\" --nsset=\"NSS::137\" "
        << "--period=24 --registrar=\"CON::005\"\n"
        << "will create domain with FQDN ``example.cz'', administrator contacts "
        << "``CON::001'' and ``CON::005'', with NSSet ``NSS::137'', valid for "
        << "2 years without any keyset."
        << std::endl;
}
