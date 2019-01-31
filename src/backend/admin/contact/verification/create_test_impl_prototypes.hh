/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  push back contact verification checks to queue up to it's at maximal length
 */

#ifndef CREATE_TEST_IMPL_PROTOTYPES_HH_9627A790EC304B26805677091D2BE100
#define CREATE_TEST_IMPL_PROTOTYPES_HH_9627A790EC304B26805677091D2BE100

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"

#include <map>
#include <memory>
#include <string>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * @return mapping of testnames to configured test implementation instances
 */
std::map<std::string, std::shared_ptr<Test>> create_test_impl_prototypes(
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::Document::Manager> _document_manager,
        std::shared_ptr<LibFred::Messages::Manager> _message_manager,
        const std::string& _cz_address_dataset_path);


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
