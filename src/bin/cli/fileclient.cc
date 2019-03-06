/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/fileclient.hh"

namespace Admin {


void
FileClient::runMethod()
{
    if (file_list) {
        list();
    }
}


void
FileClient::list()
{

    // init file manager
    CorbaClient corba_client(0, 0, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
            );
    FileManagerClient fm_client(corba_client.getNS());
    LibFred::File::ManagerPtr file_manager(LibFred::File::Manager::create(&fm_client));

    std::unique_ptr<LibFred::File::List> fileList(file_manager->createList());

    Database::Filters::File *fileFilter;
    fileFilter = new Database::Filters::FileImpl(true);

    if (file_list_params.id.is_value_set()//m_conf.hasOpt("id")
            )
        fileFilter->addId().setValue(Database::ID(file_list_params.id.get_value()
                ));

    if (file_list_params.name_name.is_value_set()//m_conf.hasOpt("name_name")
            )
        fileFilter->addName().setValue(file_list_params.name_name.get_value()//m_conf.get<std::string>("name_name")
                );

    if (file_list_params.crdate.is_value_set()//m_conf.hasOpt("crdate")
            ) {
       fileFilter->addCreateTime().setValue(
               *parseDateTime(file_list_params.crdate.get_value()//m_conf.get<std::string>("crdate")
                       ));
    }

    if (file_list_params.path.is_value_set()//m_conf.hasOpt(FILE_PATH_NAME)
            )
        fileFilter->addPath().setValue(
                file_list_params.path.get_value()//m_conf.get<std::string>(FILE_PATH_NAME)
                );
    if (file_list_params.mime.is_value_set()// m_conf.hasOpt(FILE_MIME_NAME)
            )
        fileFilter->addPath().setValue(
                file_list_params.mime.get_value()//m_conf.get<std::string>(FILE_MIME_NAME)
                );
    if (file_list_params.size.is_value_set()//m_conf.hasOpt(FILE_SIZE_NAME)
            )
        fileFilter->addSize().setValue(
                file_list_params.size.get_value()//m_conf.get<int>(FILE_SIZE_NAME)
                );
    if (file_list_params.file_type.is_value_set()//m_conf.hasOpt(FILE_TYPE_NAME)
            )
        fileFilter->addType().setValue(
                file_list_params.file_type.get_value()//m_conf.get<int>(FILE_TYPE_NAME)
                );

    Database::Filters::Union *unionFilter;
    unionFilter = new Database::Filters::Union();

    unionFilter->addFilter(fileFilter);
    fileList->setLimit(file_list_params.limit//m_conf.get<unsigned int>(LIMIT_NAME)
            );

    fileList->reload(*unionFilter);

    std::cout << "<object>\n";
    for (unsigned int i = 0; i < fileList->getSize(); i++) {
        LibFred::File::File *file = fileList->get(i);
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

} // namespace Admin;


