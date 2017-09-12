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

#ifndef FACTORY_HH_4BFAD236EAC59AEF87FC6037F0106ABF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define FACTORY_HH_4BFAD236EAC59AEF87FC6037F0106ABF

#include "src/record_statement/record_statement.hh"

#include <map>

namespace Fred {
namespace RecordStatement {
namespace Impl {

class Factory
{
public:
    typedef boost::shared_ptr<Registry::RecordStatement::RecordStatementImpl::WithExternalContext> Product;
    typedef Product (*Producer)(const boost::shared_ptr<Fred::Document::Manager>&,
                                const boost::shared_ptr<Fred::Mailer::Manager>&);
    static void register_producer(const std::string& _key, Producer _producer);
    static Product produce(
            const std::string& _handle_of_timezone,
            const boost::shared_ptr<Fred::Document::Manager>& _doc_manager,
            const boost::shared_ptr<Fred::Mailer::Manager>& _mailer_manager);
private:
    Factory();
    Factory(const Factory&);
    ~Factory();
    Factory& operator=(const Factory&);
    typedef std::map<std::string, Producer> RegisteredProducers;
    static RegisteredProducers& get_registered_producers();
};

}//namespace Fred::RecordStatement::Impl
}//namespace Fred::RecordStatement
}//namespace Fred

#endif//FACTORY_HH_4BFAD236EAC59AEF87FC6037F0106ABF
