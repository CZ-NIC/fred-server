#include <iostream>
#include <fstream>
#include "admin/admin_impl.h"
#include "corba/nameservice.cc"
#include "corba/ccReg.hh"
#include "log/logger.h"

#include "conf/manager.h"
#include "admin/bankinginvoicing_impl.h"

const char *corbaOpts[][2] = {
    {"nativeCharCodeSet", "UTF-8"},
    {NULL, NULL},
};

class CorbaClient {
    CORBA::ORB_var orb;
    std::auto_ptr<NameService> ns;
public:
    CorbaClient(int argc, char **argv, const std::string &nshost)
    {
        orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        ns.reset(new NameService(orb, nshost, "fred"));
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

int
main(int argc, char *argv[])
{
    std::stringstream nsAddr, database;
    //nsAddr << varMap["nshost"].as<std::string>() << ":" << varMap["nsport"].as<unsigned int>();
    nsAddr << "localhost:22346";
    database << "dbname=fred user=fred host=localhost port=22345";
    CorbaClient cc(argc, argv, nsAddr.str());
    CORBA::Object_var o = cc.getNS()->resolve("EPP");

    po::options_description cmd_opts;
    cmd_opts.add_options()
      ("view-config", "View actual configuration");

    po::options_description database_opts("Database");
    database_opts.add_options()
      ("database.host",     po::value<std::string>()->default_value(""),     "Database hostname")
      ("database.port",     po::value<unsigned>()->default_value(5432),      "Database port")
      ("database.name",     po::value<std::string>()->default_value("fred"), "Database name")
      ("database.user",     po::value<std::string>()->default_value("fred"), "Database username")
      ("database.password", po::value<std::string>()->default_value(""),     "Database password")
      ("database.timeout",  po::value<unsigned>()->default_value(0),         "Database connection timeout")
      ("banking.host",     po::value<std::string>()->default_value(""),     "Database hostname (obsolete)")
      ("banking.port",     po::value<unsigned>()->default_value(5432),      "Database port (obsolete)")
      ("banking.dbname",   po::value<std::string>()->default_value("fred"), "Database name (obsolete)")
      ("banking.log_level",  po::value<unsigned>()->default_value(7),         "Old logging level (obsolete)")
      ("banking.log_local",  po::value<unsigned>()->default_value(1),         "Old logging facility (obsolete)");

    po::options_description nameservice_opts("CORBA Nameservice");
    nameservice_opts.add_options()
      ("nameservice.host",    po::value<std::string>()->default_value("localhost"), "CORBA nameservice hostname")
      ("nameservice.port",    po::value<unsigned>()->default_value(2809),           "CORBA nameservice port")
      ("nameservice.context", po::value<std::string>()->default_value("fred"),      "CORBA nameservice context");
    po::options_description log_opts("Logging");
    log_opts.add_options()
      ("log.type",            po::value<unsigned>()->default_value(1),             "Log type")
      ("log.level",           po::value<unsigned>()->default_value(8),             "Log level")
      ("log.file",            po::value<std::string>()->default_value("fred.log"), "Log file path (for log.type = 1)")
      ("log.syslog_facility", po::value<unsigned>()->default_value(1),             "Syslog facility (for log.type = 2)");
    po::options_description registry_opts("Registry");
    registry_opts.add_options()
      ("registry.restricted_handles",   po::value<bool>()->default_value(false), "To force using restricted handles for NSSETs, KEYSETs and CONTACTs")
      ("registry.disable_epp_notifier", po::value<bool>()->default_value(false), "Disable EPP notifier subsystem")
      ("registry.lock_epp_commands",    po::value<bool>()->default_value(true),  "Database locking of multiple update epp commands on one object")
      ("registry.nsset_level",          po::value<unsigned>()->default_value(3), "Default report level of new NSSET")
      ("registry.docgen_path",          po::value<std::string>()->default_value("/usr/local/bin/fred-doc2pdf"),       "PDF generator path")
      ("registry.docgen_template_path", po::value<std::string>()->default_value("/usr/local/share/fred-doc2pdf/"),    "PDF generator template path")
      ("registry.fileclient_path",      po::value<std::string>()->default_value("/usr/local/bin/filemanager_client"), "File manager client path");
    po::options_description rifd_opts("RIFD specific");
    rifd_opts.add_options()
      ("rifd.session_max",           po::value<unsigned>()->default_value(200), "RIFD maximum number of sessions")
      ("rifd.session_timeout",       po::value<unsigned>()->default_value(300), "RIFD session timeout")
      ("rifd.session_registrar_max", po::value<unsigned>()->default_value(5), "RIFD maximum munber active sessions per registrar");
    po::options_description adifd_opts("ADIFD specific");
    adifd_opts.add_options()
      ("adifd.session_max",     po::value<unsigned>()->default_value(0),    "ADIFD maximum number of sessions (0 mean not limited)")
      ("adifd.session_timeout", po::value<unsigned>()->default_value(3600), "ADIFD session timeout")
      ("adifd.session_garbage", po::value<unsigned>()->default_value(150),  "ADIFD session garbage interval");
    po::options_description logd_opts("LOGD specific");
	logd_opts.add_options()
	  ("logd.monitoring_hosts_file", po::value<std::string>()->default_value("/usr/local/etc/fred/monitoring_hosts.conf"), "File containing list of monitoring machines");

    po::options_description file_opts;
    file_opts.add(database_opts).add(nameservice_opts).add(log_opts).add(registry_opts).add(rifd_opts).add(adifd_opts).add(logd_opts);

    Config::Manager cfm = Config::ConfigManager::instance_ref();
    try {
      cfm.init(argc, argv);
      cfm.setCmdLineOptions(cmd_opts);
      cfm.setCfgFileOptions(file_opts, CONFIG_FILE, true);
      cfm.parse();
    }
    catch (Config::Manager::ConfigParseError &_err) {
      std::cerr << "config parse error: " << _err.what() << std::endl;
      exit(-20);
    }

    Config::Conf cfg = cfm.get();

    std::string dbhost = cfg.get<std::string>("database.host");
    dbhost = (dbhost.empty() ? "" : "host=" + dbhost + " ");
    std::string dbpass = cfg.get<std::string>("database.password");
    dbpass = (dbpass.empty() ? "" : "password=" + dbpass + " ");
    std::string dbname = cfg.get<std::string>("database.name");
    std::string dbuser = cfg.get<std::string>("database.user");
    unsigned    dbport = cfg.get<unsigned>("database.port");
    unsigned    dbtime = cfg.get<unsigned>("database.timeout");
    std::string conn_info = str(boost::format("%1%port=%2% dbname=%3% user=%4% %5%connect_timeout=%6%")
                                              % dbhost
                                              % dbport
                                              % dbname
                                              % dbuser
                                              % dbpass
                                              % dbtime);

    Database::Manager::init(new Database::ConnectionFactory(conn_info));

    // TODO this should be enabled after pairInvoices is moved to bank_manager and corba interface is restored 
    // ccReg_BankingInvoicing_i *bla = new ccReg_BankingInvoicing_i(cc.getNS());
    // bla->pairInvoices();
    // delete bla;

    return 0;
}
