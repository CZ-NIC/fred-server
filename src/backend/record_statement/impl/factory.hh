/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
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
#ifndef FACTORY_HH_11FF9EFF1490482CBE6092537EE29473
#define FACTORY_HH_11FF9EFF1490482CBE6092537EE29473

#include "src/backend/record_statement/record_statement.hh"

#include "util/factory.hh"


namespace Fred {
namespace Backend {
namespace RecordStatement {
namespace Impl {

using Product = std::shared_ptr<RecordStatementImpl::WithExternalContext>;

struct Producer
{
    virtual Product operator()(const std::shared_ptr<LibFred::Document::Manager>&,
                               const std::shared_ptr<LibFred::Mailer::Manager>&) = 0;
};

using Factory = Util::Factory<Producer>;

template <typename> void register_producer(Factory&);

const Factory& get_default_factory();

} // namespace Fred::Backend::RecordStatement::Impl
} // namespace Fred::Backend::RecordStatement
} // namespace Fred::Backend
} // namespace Fred

#endif//FACTORY_HH_11FF9EFF1490482CBE6092537EE29473
