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
#include "fileclient.h"

namespace Admin {

const struct options *
FileClient::getOpts()
{
    return m_opts;
}

void
FileClient::runMethod()
{
    if (m_conf.hasOpt(FILE_LIST_NAME)) {
        list();
    } else if (m_conf.hasOpt(FILE_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
FileClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("File", getOpts(), getOptsCount());
}

void
FileClient::list()
{
    callHelp(m_conf, list_help);

    /* init file manager */
    CorbaClient corba_client(0, 0, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    FileManagerClient fm_client(corba_client.getNS());
    Register::File::ManagerPtr file_manager(Register::File::Manager::create(&fm_client));

    std::auto_ptr<Register::File::List> fileList(file_manager->createList());

    Database::Filters::File *fileFilter;
    fileFilter = new Database::Filters::FileImpl(true);

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
    for (unsigned int i = 0; i < fileList->getSize(); i++) {
        Register::File::File *file = fileList->get(i);
        std::cout
            << "\t<file>\n"
            << "\t\t<id>" << file->getId() << "</id>\n"
            << "\t\t<name>" << file->getName() << "</name>\n"
            << "\t\t<path>" << file->getPath() << "</path>\n"
            << "\t\t<mime>" << file->getMimeType() << "</mime>\n"
            << "\t\t<type>" << file->getFileTypeId() << "</type>\n"
            << "\t\t<type_desc>" << file->getFileTypeDesc() << "</type_desc>\n"
            << "\t\t<crdate>" << file->getCrDate() << "</crdate>\n"
            << "\t\t<size>" << file->getFilesize() << "</size>\n"
            << "\t</file>\n";
    }
    std::cout << "</object>" << std::endl;

    unionFilter->clear();
    // XXX this delete cause segfault :(
    // delete fileFilter;
    delete unionFilter;
} // FileClient::list

void
FileClient::list_help()
{
    std::cout
        << "** File list **\n\n"
        << "  $ " << g_prog_name << " --" << FILE_LIST_NAME << " \\\n"
        << "  [--" << ID_NAME << "=<id_number>] \\\n"
        << "  [--" << NAME_NAME << "=<name>] \\\n"
        << "  [--" << FILE_TYPE_NAME << "=<type_name>] \\\n"
        << "  [--" << FILE_PATH_NAME << "=<path>] \\\n"
        << "  [--" << FILE_MIME_NAME << "=<mime_type>] \\\n"
        << "  [--" << FILE_SIZE_NAME << "=<size>] \\\n"
        << "  [--" << CRDATE_NAME << "=<create_time>]\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_FILE, name, name##_DESC, type, callable, visible}

const struct options
FileClient::m_opts[] = {
    ADDOPT(FILE_LIST_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(FILE_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    add_ID,
    add_NAME,
    ADDOPT(FILE_TYPE_NAME, TYPE_INT, false, false),
    ADDOPT(FILE_PATH_NAME, TYPE_STRING, false, false),
    ADDOPT(FILE_MIME_NAME, TYPE_STRING, false, false),
    ADDOPT(FILE_SIZE_NAME, TYPE_INT, false, false),
    add_CRDATE,
};

#undef ADDOPT

int 
FileClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;


