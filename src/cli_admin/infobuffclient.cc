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
#include "infobuffclient.h"
#include "register/info_buffer.h"

namespace Admin {

InfoBuffClient::InfoBuffClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "Info buffer related options");
    m_options->add_options()
        ADD_OPT_TYPE(INFOBUFF_MAKE_INFO_NAME, "invoke generation of list of o for given registrar", unsigned int)
        ADD_OPT_TYPE(INFOBUFF_GET_CHUNK_NAME, "output chunk of buffer for given registrar", unsigned int);

    m_optionsInvis = new boost::program_options::options_description(
            "Info buffer related invisible options");
    m_optionsInvis->add_options()
        ADD_OPT_DEF(INFOBUFF_REQUEST_NAME, "handle for query", std::string, "");
}
InfoBuffClient::InfoBuffClient(
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

InfoBuffClient::~InfoBuffClient()
{
    delete m_dbman;
    delete m_options;
    delete m_optionsInvis;
}

void
InfoBuffClient::init(
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
InfoBuffClient::getVisibleOptions() const
{
    return m_options;
}

boost::program_options::options_description *
InfoBuffClient::getInvisibleOptions() const
{
    return m_optionsInvis;
}

int
InfoBuffClient::make_info()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::InfoBuffer::Manager> infoBuffMan(
            Register::InfoBuffer::Manager::create(
                &m_db,
                domMan.get(),
                nssMan.get(),
                conMan.get(),
                keyMan.get())
            );
    unsigned int type = m_varMap[INFOBUFF_MAKE_INFO_NAME].as<unsigned int>();
    if (type < 1 || type > 7)
        std::cerr << INFOBUFF_MAKE_INFO_NAME << " must be number between 1 and 7" << std::endl;
    else {
        infoBuffMan->info(
                m_varMap[REGISTRAR_ID_NAME].as<unsigned int>(),
                type == 1 ? Register::InfoBuffer::T_LIST_CONTACTS :
                type == 2 ? Register::InfoBuffer::T_LIST_DOMAINS :
                type == 3 ? Register::InfoBuffer::T_LIST_NSSETS :
                type == 4 ? Register::InfoBuffer::T_LIST_KEYSETS :
                type == 5 ? Register::InfoBuffer::T_DOMAINS_BY_NSSET :
                type == 6 ? Register::InfoBuffer::T_DOMAINS_BY_CONTACT :
                type == 7 ? Register::InfoBuffer::T_DOMAINS_BY_KEYSET :
                type == 8 ? Register::InfoBuffer::T_NSSETS_BY_CONTACT :
                type == 9 ? Register::InfoBuffer::T_NSSETS_BY_NS :
                Register::InfoBuffer::T_KEYSETS_BY_CONTACT,      
                m_varMap[INFOBUFF_REQUEST_NAME].as<std::string>());
    }
    return 0;
}
int
InfoBuffClient::get_chunk()
{
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&m_db));
    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_varMap[RESTRICTED_HANDLES_NAME].as<unsigned int>())
            );
    std::auto_ptr<Register::InfoBuffer::Manager> infoBuffMan(
            Register::InfoBuffer::Manager::create(
                &m_db,
                domMan.get(),
                nssMan.get(),
                conMan.get(),
                keyMan.get())
            );
    std::auto_ptr<Register::InfoBuffer::Chunk> chunk(
            infoBuffMan->getChunk(
                m_varMap[REGISTRAR_ID_NAME].as<unsigned int>(),
                m_varMap[INFOBUFF_GET_CHUNK_NAME].as<unsigned int>())
            );
    for (unsigned long i = 0; i < chunk->getCount(); i++)
        std::cout << chunk->getNext() << std::endl;
    return 0;
}

} // namespace Admin;


