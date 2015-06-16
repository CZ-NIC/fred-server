/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  operation context
 */

#ifndef OPCONTEXT_H_514FF01C2C974C899314DD0B8DE0E372//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define OPCONTEXT_H_514FF01C2C974C899314DD0B8DE0E372

#include "util/log/log.h"
#include "src/fredlib/db_settings.h"

#include <string>
#include <boost/utility.hpp>

/**
 * What's need in Fred.
 */
namespace Fred
{

class OperationContext;
class OperationContextCreator;

/**
 * Database connection used in Fred operations.
 */
class OperationContextDatabasePart
{
public:
    typedef Database::StandaloneConnection Connection;///< typename alias
    /**
     * Obtain database connection with running transaction.
     * @return database connection object reference
     * @throw std::runtime_error if no transaction in progress
     */
    Connection& get_conn()const;
    /**
     * Database transaction string identification which will use in first phase of two-phase commit.
     * @throw std::runtime_error if no transaction_id was set in the past
     */
    std::string get_transaction_id()const;
private:
    OperationContextDatabasePart();
    OperationContextDatabasePart(const std::string &_transaction_id);
    const std::string transaction_id_;
    typedef std::auto_ptr< Connection > ConnectionPtr;
    ConnectionPtr conn_;
    friend class OperationContext;
    friend class OperationContextCreator;
};

/**
 * Logging object used in Fred operations.
 */
class OperationContextLoggingPart
{
public:
    /**
     * Obtain logging object.
     * @return logging object reference
     */
    Logging::Log& get_log() { return log_; }
private:
    OperationContextLoggingPart();
    Logging::Log &log_;
    friend class OperationContext;
};

/**
 * Common objects needed in Fred operations. It consists of two parts, database and logging.
 */
class OperationContext
:   public OperationContextDatabasePart,
    public OperationContextLoggingPart
{
private:
    OperationContext() { }
    OperationContext(const std::string &_transaction_id):OperationContextDatabasePart(_transaction_id) { }
    friend class OperationContextCreator;
};

/**
 * Makes accessible OperationContext instance and offers commit_transaction() method.
 */
class OperationContextCreator
:   private boost::noncopyable,
    public OperationContext
{
public:
    /**
     * Creates logging object and starts database transaction without two-phase commit ability.
     */
    OperationContextCreator() { }
    /**
     * Creates logging object and starts database transaction prepared for two-phase commit.
     * @param _transaction_id database transaction string identification usable in second phase commit/rollback
     * @throw std::runtime_error in case of empty _transaction_id
     * @note Calling commit_transaction() will process first phase of two-phase commit.
     */
    OperationContextCreator(const std::string &_transaction_id)
    :   OperationContext(_transaction_id) { }
    /**
     * Processes database transaction rollback if transaction wasn't commited.
     */
    ~OperationContextCreator();
    /**
     * Commits database transaction.
     * @throw std::runtime_error if no transaction in progress
     * @note If transaction_id was set it will process first phase of two-phase commit.
     */
    void commit_transaction();
};

/**
 * Processes second phase of two-phase database transaction commit on standalone database connection.
 * @param _transaction_id database transaction string identification used in first phase of two-phase commit
 * @throw std::runtime_error if _transaction_id empty
 */
void commit_transaction(const std::string &_transaction_id);
/**
 * Drops two-phase database transaction on standalone database connection.
 * @param _transaction_id database transaction string identification used in first phase of two-phase commit
 * @throw std::runtime_error if _transaction_id empty
 */
void rollback_transaction(const std::string &_transaction_id);

}//namespace Fred

#endif //OPCONTEXT_H_514FF01C2C974C899314DD0B8DE0E372
