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
#include "infobuffclient.h"
#include "register/info_buffer.h"

namespace Admin {

InfoBuffClient::InfoBuffClient()
{
    m_options = new boost::program_options::options_description(
            "Info buffer related options");
    m_options->add_options()
        addOptUInt(INFOBUFF_MAKE_INFO_NAME)
        addOptUInt(INFOBUFF_GET_CHUNK_NAME)
        addOpt(INFOBUFF_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "Info buffer related sub options");
    m_optionsInvis->add_options()
        addOptUInt(INFOBUFF_REGISTRAR_NAME)
        addOptStrDef(INFOBUFF_REQUEST_NAME, "");
}
InfoBuffClient::InfoBuffClient(
        std::string connstring,
        std::string nsAddr) : BaseClient(connstring, nsAddr)
{
    m_db.OpenDatabase(connstring.c_str());
    m_options = NULL;
    m_optionsInvis = NULL;
}

InfoBuffClient::~InfoBuffClient()
{
    delete m_options;
    delete m_optionsInvis;
}

void
InfoBuffClient::init(
        std::string connstring,
        std::string nsAddr,
        Config::Conf &conf)
{
    BaseClient::init(connstring, nsAddr);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
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

void
InfoBuffClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
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
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::InfoBuffer::Manager> infoBuffMan(
            Register::InfoBuffer::Manager::create(
                &m_db,
                domMan.get(),
                nssMan.get(),
                conMan.get(),
                keyMan.get())
            );
    unsigned int type = m_conf.get<unsigned int>(INFOBUFF_MAKE_INFO_NAME);
    if (type < 1 || type > 7)
        std::cerr << INFOBUFF_MAKE_INFO_NAME << " must be number between 1 and 7" << std::endl;
    else {
        infoBuffMan->info(
                m_conf.get<unsigned int>(REGISTRAR_ID_NAME),
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
                m_conf.get<std::string>(INFOBUFF_REQUEST_NAME));
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
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
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
                m_conf.get<unsigned int>(REGISTRAR_ID_NAME),
                m_conf.get<unsigned int>(INFOBUFF_GET_CHUNK_NAME))
            );
    for (unsigned long i = 0; i < chunk->getCount(); i++)
        std::cout << chunk->getNext() << std::endl;
    return 0;
}

} // namespace Admin;


