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
#include "fileclient.h"

namespace Admin {

FileClient::FileClient():
    m_connstring(""), m_nsAddr("")
{
    m_options = new boost::program_options::options_description(
            "File related options");
    m_options->add_options()
        addOpt(FILE_LIST_NAME)
        addOpt(FILE_LIST_HELP_NAME)
        addOpt(FILE_SHOW_OPTS_NAME);

    m_optionsInvis = new boost::program_options::options_description(
            "File related sub options");
    m_optionsInvis->add_options()
        add_ID()
        add_NAME()
        addOptInt(FILE_TYPE_NAME)
        addOptStr(FILE_PATH_NAME)
        addOptStr(FILE_MIME_NAME)
        addOptInt(FILE_SIZE_NAME)
        add_CRDATE();

}
FileClient::FileClient(
        std::string connstring,
        std::string nsAddr):
    m_connstring(connstring), m_nsAddr(nsAddr)
{
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
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
        Config::Conf &conf)
{
    m_connstring = connstring;
    m_nsAddr = nsAddr;
    m_dbman = new Database::Manager(m_connstring);
    m_db.OpenDatabase(connstring.c_str());
    m_conf = conf;
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
FileClient::show_opts() const
{
    std::cout << *m_options << std::endl;
    std::cout << *m_optionsInvis << std::endl;
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

    apply_ID(fileFilter);
    apply_NAME(fileFilter);
    apply_CRDATE(fileFilter);

    if (m_conf.hasOpt(FILE_PATH_NAME))
        fileFilter->addPath().setValue(
                m_conf.get<std::string>(FILE_PATH_NAME));
    if (m_conf.hasOpt(FILE_MIME_NAME))
        fileFilter->addPath().setValue(
                m_conf.get<std::string>(FILE_MIME_NAME));
    if (m_conf.hasOpt(FILE_SIZE_NAME))
        fileFilter->addSize().setValue(
                m_conf.get<int>(FILE_SIZE_NAME));
    if (m_conf.hasOpt(FILE_TYPE_NAME))
        fileFilter->addType().setValue(
                m_conf.get<int>(FILE_TYPE_NAME));

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(fileFilter);
    fileList->setLimit(m_conf.get<unsigned int>(LIMIT_NAME));

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


