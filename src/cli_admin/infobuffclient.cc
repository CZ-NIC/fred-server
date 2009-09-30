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
#include "infobuffclient.h"
#include "register/info_buffer.h"

namespace Admin {

const struct options *
InfoBuffClient::getOpts()
{
    return m_opts;
}

void
InfoBuffClient::runMethod()
{
    if (m_conf.hasOpt(INFOBUFF_SHOW_OPTS_NAME)) {
        show_opts();
    } else if (m_conf.hasOpt(INFOBUFF_MAKE_INFO_NAME)) {
        make_info();
    } else if (m_conf.hasOpt(INFOBUFF_GET_CHUNK_NAME)) {
        get_chunk();
    }
}

void
InfoBuffClient::show_opts() 
{
    callHelp(m_conf, no_help);
    print_options("InfoBuff", getOpts(), getOptsCount());
}

void
InfoBuffClient::make_info()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());
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
    std::string requestName("");
    if (m_conf.hasOpt(INFOBUFF_REQUEST_NAME)) {
        requestName = m_conf.get<std::string>(INFOBUFF_REQUEST_NAME);
    }
    if (type < 1 || type > 10) {
        std::cerr
            << INFOBUFF_MAKE_INFO_NAME
            << " must be number between 1 and 10 (inclusive border values)" 
            << std::endl;
        return;
    }

    Register::InfoBuffer::Type type_type =
        type == 1 ? Register::InfoBuffer::T_LIST_CONTACTS :
        type == 2 ? Register::InfoBuffer::T_LIST_DOMAINS :
        type == 3 ? Register::InfoBuffer::T_LIST_NSSETS :
        type == 4 ? Register::InfoBuffer::T_LIST_KEYSETS :
        type == 5 ? Register::InfoBuffer::T_DOMAINS_BY_NSSET :
        type == 6 ? Register::InfoBuffer::T_DOMAINS_BY_CONTACT :
        type == 7 ? Register::InfoBuffer::T_DOMAINS_BY_KEYSET :
        type == 8 ? Register::InfoBuffer::T_NSSETS_BY_CONTACT :
        type == 9 ? Register::InfoBuffer::T_NSSETS_BY_NS :
        Register::InfoBuffer::T_KEYSETS_BY_CONTACT;
    if (m_conf.hasOpt(REGISTRAR_ID_NAME)) {
        infoBuffMan->info(
                m_conf.get<unsigned int>(REGISTRAR_ID_NAME),
                type_type, requestName);
    } else if (m_conf.hasOpt(REGISTRAR_HANDLE_NAME)) {
        infoBuffMan->info(
                m_conf.get<std::string>(REGISTRAR_HANDLE_NAME),
                type_type, requestName);
    } else {
        std::cerr << "You have to specify either ``--" << REGISTRAR_ID_NAME
            << "'' or ``--" << REGISTRAR_HANDLE_NAME << "''" << std::endl;
    }
}

void
InfoBuffClient::get_chunk()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());
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
    if (m_conf.hasOpt(REGISTRAR_ID_NAME)) {
        std::auto_ptr<Register::InfoBuffer::Chunk> chunk(
                infoBuffMan->getChunk(
                    m_conf.get<unsigned int>(REGISTRAR_ID_NAME),
                    m_conf.get<unsigned int>(INFOBUFF_GET_CHUNK_NAME))
                );
        for (unsigned long i = 0; i < chunk->getCount(); i++) {
            std::cout << chunk->getNext() << std::endl;
        }
    } else if (m_conf.hasOpt(REGISTRAR_HANDLE_NAME)) {
        std::auto_ptr<Register::InfoBuffer::Chunk> chunk(
                infoBuffMan->getChunk(
                    m_conf.get<std::string>(REGISTRAR_HANDLE_NAME),
                    m_conf.get<unsigned int>(INFOBUFF_GET_CHUNK_NAME))
                );
        for (unsigned long i = 0; i < chunk->getCount(); i++) {
            std::cout << chunk->getNext() << std::endl;
        }
    } else {
        std::cerr << "You have to specify either ``--" << REGISTRAR_ID_NAME
            << "'' or ``--" << REGISTRAR_HANDLE_NAME << "''" << std::endl;
    }
} // InfoBuffClient::get_chunk()

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_INFOBUFF, name, name##_DESC, type, callable, visible}

const struct options
InfoBuffClient::m_opts[] = {
    add_REGISTRAR_ID,
    add_REGISTRAR_HANDLE,
    ADDOPT(INFOBUFF_MAKE_INFO_NAME, TYPE_UINT, true, true),
    ADDOPT(INFOBUFF_GET_CHUNK_NAME, TYPE_UINT, true, true),
    ADDOPT(INFOBUFF_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(INFOBUFF_REGISTRAR_NAME, TYPE_UINT, false, false),
    ADDOPT(INFOBUFF_REQUEST_NAME, TYPE_STRING, false, false),
};

#undef ADDOPT

int 
InfoBuffClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

