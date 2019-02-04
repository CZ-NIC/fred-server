/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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
 *  @file manager_tss.hh
 *  \brief Storing Connection as a thread specific data
 */

#ifndef MANAGER_TSS_HH_BF0788B58F294031ACB5E438E48E058E
#define MANAGER_TSS_HH_BF0788B58F294031ACB5E438E48E058E

#include "src/util/db/sequence.hh"
#include "src/util/db/statement.hh"

#include "util/db/result.hh"
#include "util/db/connection.hh"
#include "util/db/psql/psql_connection.hh"
#include "util/db/transaction.hh"

#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include <string>

namespace Database {

class TerribleHack;

template <typename T>
struct ConnectionDriverTraits
{
    using TransactionType = typename T::transaction_type;
    static constexpr std::string (*getTimeoutString)() = T::getTimeoutString;
};

template <>
struct ConnectionDriverTraits<PSQLConnection>
{
    class TransactionType
    {
    public:
        typedef PSQLConnection connection_type;

        TransactionType() { }
        ~TransactionType() { }

        static std::string start() { return "START TRANSACTION  ISOLATION LEVEL READ COMMITTED"; }
        static std::string rollback() { return "ROLLBACK TRANSACTION"; }
        static std::string commit() { return "COMMIT TRANSACTION"; }
        static std::string prepare(const std::string& _id) { return "PREPARE TRANSACTION '" + _id + "'"; }
    };

    static std::string getTimeoutString()
    {
        return "statement timeout";
    }
};

/**
 * \class  ConnectionBase_
 */
template<class connection_driver, class manager_type>
class ConnectionBase_
{
public:
    typedef connection_driver                        driver_type;
    typedef typename manager_type::transaction_type  transaction_type;
    typedef typename manager_type::result_type       result_type;

    /**
     * Constructor and destructor
     */
    ConnectionBase_(connection_driver* _conn)
        : conn_(_conn) { }

    virtual ~ConnectionBase_() { }

    static const std::string getTimeoutString()
    {
        return ConnectionDriverTraits<connection_driver>::getTimeoutString();
    }
    /**
     * This call is converted to stringize method call
     *
     * @param _query object representing query statement
     * @return       result
     */
    virtual result_type exec(Statement& _stmt) /*throw (ResultFailed)*/
    {
        return this->exec(_stmt.toSql(boost::bind(&ConnectionBase_<connection_driver, manager_type>::escape, this, _1)));
    }

    /**
     * @param _query boost::format representing query statement
     * @return       result
     */
    virtual result_type exec(const boost::format& _fmt) /*throw (ResultFailed)*/
    {
        try
        {
            return exec(_fmt.str());
        }
        catch (const ResultFailed&)
        {
            throw;
        }
        catch (...)
        {
            throw ResultFailed(">>Conversion error<<");
        }
    }

    /**
     * @param _query string representation of query statement
     * @return       result
     */
    virtual result_type exec(const std::string& _stmt) /*throw (ResultFailed)*/
    {
        try
        {
#ifdef HAVE_LOGGER
            LOGGER.debug(boost::format("exec query [%1%]") % _stmt);
#endif
            return result_type(conn_->exec(_stmt));
        }
        catch (const ResultFailed&)
        {
            throw;
        }
        catch (...)
        {
            throw ResultFailed(_stmt);
        }
    }

    /**
     * @param _stmt string representation of query statement with params
     * @param params vector of param data strings
     * @return result
     */
    virtual result_type exec_params(const std::string& _stmt,//one command query
                                    const std::vector<std::string>& params)//parameters data
    {
        try
        {
#ifdef HAVE_LOGGER
            LOGGER.debug(boost::format("exec query [%1%]") % _stmt);
#endif
            return result_type(conn_->exec_params(_stmt,//one command query
                               params));//parameters data
        }
        catch (const ResultFailed&)
        {
            throw;
        }
        catch (...)
        {
            throw ResultFailed(_stmt);
        }
    }

    /**
     * @param _stmt string representation of query statement with params
     * @param params vector of query param data strings or binary
     * @return result
     */
    virtual result_type exec_params(const std::string& _stmt,//one command query
                                    const QueryParams& params)//parameters data
    {
        try
        {
#ifdef HAVE_LOGGER
          if (LOGGER.is_sufficient<Logging::Log::EventImportance::debug>())
          {
              std::string value;
              std::string params_dump;
              std::size_t params_counter =0;
              for (QueryParams::const_iterator i = params.begin(); i != params.end(); ++i)
              {
                  ++params_counter;
                  value = i->is_null() ? "[null]" : "'" + i->print_buffer() + "'";
                  params_dump += " $" + boost::lexical_cast<std::string>(params_counter) + ": " + value;
              }

              LOGGER.debug(boost::format("exec query [%1%] params %2%") % _stmt % params_dump);
          }

#endif
          return result_type(conn_->exec_params(_stmt,//one command query
                             params));//parameters data
        }
        catch (const ResultFailed&)
        {
            throw;
        }
        catch (...)
        {
            throw ResultFailed(_stmt);
        }
    }

    /**
     * Reset connection to state after connect
     */
    virtual void reset()
    {
        conn_->reset();
    }

    /**
     * String escape method by specific connection_driver
     */
    virtual std::string escape(const std::string& _in)
    {
        return conn_->escape(_in);
    }

    /**
     * @return  true if there is active transaction on connection
     *          false otherwise
     */
    virtual bool is_in_transaction() const
    {
        return conn_->is_in_transaction();
    }

    bool in_valid_transaction() const
    {
        return (conn_ != NULL) && conn_->in_valid_transaction();
    }

    virtual void setQueryTimeout(unsigned t)
    {
        conn_->setQueryTimeout(t);
#ifdef HAVE_LOGGER
        LOGGER.debug(boost::format("sql statement timout set to %1%ms") % t);
#endif
    }
protected:
    connection_driver* conn_; /**< connection_driver instance */
private:
    friend class TerribleHack;
};

// HACK! HACK! HACK! (use with construct with old DB connection)
class TerribleHack
{
public:
    template <class M>
    static auto get_internal_psql_connection(const ConnectionBase_<PSQLConnection, M>& encapsulated)
    {
        return get_internal_connection_from(encapsulated.conn_);
    }
private:
    static auto get_internal_connection_from(const PSQLConnection* encapsulated)
    {
        return encapsulated->psql_conn_;
    }
};

/**
 * \class  TSSConnection_
 * \brief  Specialized connection proxy class for use with TSSManager_
 *         (static call in destructor)
 */
template<class connection_driver, class manager_type>
class TSSConnection_ : public ConnectionBase_<connection_driver, manager_type>
{
public:
    typedef ConnectionBase_<connection_driver, manager_type> super;
    typedef typename super::transaction_type transaction_type;

    /**
     * Constructor and destructor
     */
    TSSConnection_(connection_driver* _conn, transaction_type*& _trans)
        : super(_conn)
    {
        this->trans_ = &_trans;
    }

    virtual ~TSSConnection_() { }
protected:
    /**
     * Trasaction support methods
     */
    virtual void setTransaction(transaction_type* _trans)
    {
        *trans_ = _trans;
#ifdef HAVE_LOGGER
        LOGGER.debug(boost::format("(%1%) transaction assigned to (%2%) connection") % trans_ % this->conn_);
#endif
    }

    virtual void unsetTransaction()
    {
#ifdef HAVE_LOGGER
        LOGGER.debug(boost::format("(%1%) transaction released from connection") % trans_);
#endif
        *trans_ = nullptr;
    }


    virtual transaction_type* getTransaction() const
    {
        return *trans_;
    }

    transaction_type** trans_; /**< pointer to active transaction */
private:
    template <class _transaction_type, class _manager_type>
    friend class Transaction_;
};

/**
 * \class TSSManager_
 * \brief Database connection manager for storing and retrieving connections
 *        from thread local data
 *
 * @param connection_factory  connection factory class
 */
template <class connection_factory>
class TSSManager_
{
public:
    typedef typename connection_factory::connection_driver connection_driver;
    typedef TSSConnection_<connection_driver, TSSManager_> connection_type;
    typedef Transaction_<typename ConnectionDriverTraits<connection_driver>::TransactionType, TSSManager_> transaction_type;
    typedef Result_<typename connection_driver::ResultType> result_type;
    typedef Sequence_<connection_type, TSSManager_> sequence_type;
    typedef typename result_type::Row row_type;

    /**
     * Connection factory object initialization
     *
     * @param _conn_factory  factory to be assigned to manager; it has to be already initialized
     */
    static void init(connection_factory* _conn_factory)
    {
        conn_factory_ = _conn_factory;
        init_ = true;
    }

    /**
     * Acquire database connection for actual thread
     *
     * @return  database connection handler pointer
     */
    static connection_type acquire()
    {
        PerThreadData_* const tmp = data_.get();
        if (tmp != nullptr)
        {
            update_data_(tmp);
#ifdef HAVE_LOGGER
            LOGGER.debug(boost::format("[tss] acquire state: conn=%1%  trans=%2%") % tmp->conn % &tmp->trans);
#endif
            return connection_type(tmp->conn, tmp->trans);
        }
        data_.reset(new PerThreadData_());
        return acquire();
    }

    static const std::string& getConnectionString()
    {
        return conn_factory_->getConnectionString();
    }

    /**
     * Explicit release database connection for actual thread
     * back to pool
     *
     * TODO: release policy - 1. do nothing (connection stay in thread until it exits)
     *                        2. factory release (i.e. to pool)
     */
    static void release()
    {
        PerThreadData_* const tmp = data_.get();
        if (tmp != nullptr)
        {
#ifdef HAVE_LOGGER
            LOGGER.debug(boost::format("[tss] release state: conn=%1%  trans=%2%") % tmp->conn % &tmp->trans);
#endif
            if (tmp->conn != nullptr)
            {
                conn_factory_->release(tmp->conn);
                tmp->conn = nullptr;
            }
        }
    }
private:
    TSSManager_() { }
    ~TSSManager_() { }

    /**
     * Stucture to store per thread connection and transaction info
     */
    struct PerThreadData_
    {
        PerThreadData_() : conn(nullptr), trans(nullptr) { }
        ~PerThreadData_()
        {
            if (conn != nullptr)
            {
                conn_factory_->release(conn);
            }
        }
        connection_driver* conn;
        transaction_type* trans;
    };

    /**
     * Helper method for acquiring database connection
     */
    static void update_data_(PerThreadData_* _data)
    {
        if (_data->conn == nullptr)
        {
            _data->conn = conn_factory_->acquire();
            _data->trans = nullptr;
        }
    }

    static connection_factory* conn_factory_; /**< connection factory */
    static boost::thread_specific_ptr<PerThreadData_> data_; /**< data per thread structure */
    static bool init_; /**< data ready initialized internal flag */
};

/**
 * static members initialization
 */
template <class connection_factory>
connection_factory* TSSManager_<connection_factory>::conn_factory_ = nullptr;

template <class connection_factory>
boost::thread_specific_ptr<typename TSSManager_<connection_factory>::PerThreadData_> TSSManager_<connection_factory>::data_;

template <class connection_factory>
bool TSSManager_<connection_factory>::init_ = false;

}//namespace Database

#endif//MANAGER_TSS_HH_BF0788B58F294031ACB5E438E48E058E
