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
#include "fileclient.h"

namespace Admin {

#define LOGIN_FILECLIENT \
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

#define LOGOUT_FILECLIENT \
    epp->ClientLogout(clientId,"system_delete_logout","<system_delete_logout/>");

FileClient::FileClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "File related options");
    m_options->add_options()
        add_opt(FILE_LIST_NAME)
        add_opt(FILE_LIST_HELP_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "File related invisible options");
    m_optionsInvis->add_options()
        add_opt_type(ID_NAME, unsigned int)
        add_opt_type(NAME_NAME, std::string)
        add_opt_type(TYPE_NAME, int)
        add_opt_type(FILE_PATH_NAME, std::string)
        add_opt_type(FILE_MIME_NAME, std::string)
        add_opt_type(FILE_SIZE_NAME, int);

}
FileClient::FileClient(
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

FileClient::~FileClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
FileClient::init(
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
FileClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
FileClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

void
FileClient::list()
{
    std::auto_ptr<Register::File::Manager> fileMan(
            Register::File::Manager::create(m_dbman));
    std::auto_ptr<Register::File::List> fileList(
            fileMan->createList());

    Database::Filters::File *fileFilter;
    fileFilter = new Database::Filters::FileImpl();

    if (m_varMap.count(ID_NAME))
        fileFilter->addId().setValue(
                Database::ID(m_varMap[ID_NAME].as<unsigned int>()));
    if (m_varMap.count(NAME_NAME))
        fileFilter->addName().setValue(
                m_varMap[NAME_NAME].as<std::string>());
    if (m_varMap.count(FILE_PATH_NAME))
        fileFilter->addPath().setValue(
                m_varMap[FILE_PATH_NAME].as<std::string>());
    if (m_varMap.count(FILE_MIME_NAME))
        fileFilter->addPath().setValue(
                m_varMap[FILE_MIME_NAME].as<std::string>());
    if (m_varMap.count(FILE_SIZE_NAME))
        fileFilter->addSize().setValue(
                m_varMap[FILE_SIZE_NAME].as<int>());
    if (m_varMap.count(TYPE_NAME))
        fileFilter->addType().setValue(
                m_varMap[TYPE_NAME].as<int>());

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(fileFilter);
    fileList->setLimit(m_varMap[LIMIT_NAME].as<unsigned int>());

    fileList->reload(*unionFilter);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < fileList->getCount(); i++) {
        Register::File::File *file = fileList->get(i);
        std::cout
            << "\t<file>\n"
            << "\t\t<id>" << file->getId() << "</id>\n"
            << "\t\t<name>" << file->getName() << "</name>\n"
            << "\t\t<path>" << file->getPath() << "</path>\n"
            << "\t\t<mime>" << file->getMimeType() << "</mime>\n"
            << "\t\t<type>" << file->getType() << "</type>\n"
            << "\t\t<type_desc>" << file->getTypeDesc() << "</type_desc>\n"
            << "\t\t<crdate>" << file->getCreateTime() << "</crdate>\n"
            << "\t\t<size>" << file->getSize() << "</size>\n"
            << "\t</file>\n";
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    // XXX this delete cause segfault :(
    // delete fileFilter;
    delete unionFilter;
}

void
FileClient::list_help()
{
    std::cout << "file client list help" << std::endl;
}

} // namespace Admin;


