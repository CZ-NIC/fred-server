/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/record_statement/impl/factory.hh"

#include <stdexcept>

namespace Fred {
namespace RecordStatement {
namespace Impl {

Factory::Factory()
{ }

Factory::~Factory()
{ }

Factory::RegisteredProducers& Factory::get_registered_producers()
{
    static RegisteredProducers static_instance;
    return static_instance;
}

void Factory::register_producer(const std::string& _key, Producer _producer)
{
    if (get_registered_producers().find(_key) != get_registered_producers().end())
    {
        throw std::runtime_error("Producer registered already.");
    }
    get_registered_producers().insert(std::make_pair(_key, _producer));
}

Factory::Product Factory::produce(
        const std::string& _handle_of_timezone,
        const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
        const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager)
{
    const RegisteredProducers::const_iterator producers_itr = get_registered_producers().find(_handle_of_timezone);
    if (producers_itr == get_registered_producers().end())
    {
        throw std::runtime_error("Unknown producer.");
    }
    return producers_itr->second(_doc_manager, _mailer_manager);
}

}//namespace Fred::RecordStatement::Impl
}//namespace Fred::RecordStatement
}//namespace Fred

