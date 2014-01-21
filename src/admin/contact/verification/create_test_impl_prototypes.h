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

#ifndef ADMIN_CONTACT_VERIFICATION_CREATE_TEST_IMPL_PROTOTYPES_H_224556435
#define ADMIN_CONTACT_VERIFICATION_CREATE_TEST_IMPL_PROTOTYPES_H_224556435

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "admin/contact/verification/test_impl/test_interface.h"

#include "fredlib/mailer.h"
#include "fredlib/documents.h"
#include "fredlib/messages/messages_impl.h"

namespace Admin {

    /**
     * @return mapping of testnames to test implementation instances
     */
    std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> > create_test_impl_prototypes(
        boost::shared_ptr<Fred::Mailer::Manager>   _mailer_manager,
        boost::shared_ptr<Fred::Document::Manager> _document_manager,
        boost::shared_ptr<Fred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    );

}

#endif // #include guard end
