/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/record_statement/impl/factory.hh"

#include <stdexcept>

namespace Fred {
namespace Backend {
namespace RecordStatement {
namespace Impl {

Factory::Factory()
{
}

Factory::~Factory()
{
}

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
        const std::shared_ptr<LibFred::Document::Manager>& _doc_manager,
        const std::shared_ptr<LibFred::Mailer::Manager>& _mailer_manager)
{
    const RegisteredProducers::const_iterator producers_itr = get_registered_producers().find(_handle_of_timezone);
    if (producers_itr == get_registered_producers().end())
    {
        throw std::runtime_error("Unknown producer.");
    }
    return producers_itr->second(_doc_manager, _mailer_manager);
}

} // namespace Fred::Backend::RecordStatement::Impl
} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred

