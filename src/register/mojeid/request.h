#ifndef MOJEID_REQUEST_H_
#define MOJEID_REQUEST_H_


#include "register/db_settings.h"

namespace MojeID {

class Request
{
private:
    unsigned int request_code_;
    unsigned long long registrar_id_;
    std::string clienttrid_;

    /* id to action table - temporary */
    unsigned long long id_;
    /* id to logger request table */
    unsigned long long request_id_;

    int status_;

    std::auto_ptr<Database::Transaction> tx_;
    std::string servertrid_;

public:
    Database::Connection conn;

    Request(const unsigned int &_request_code,
            const unsigned long long &_registrar_id,
            const unsigned long long &_request_id,
            const std::string &_clienttrid = std::string())
        : request_code_(_request_code),
          registrar_id_(_registrar_id),
          clienttrid_(_clienttrid),
          id_(0),
          request_id_(_request_id),
          status_(0),
          tx_(0),
          conn(Database::Manager::acquire())
    {
        try {
            if (request_id_ == 0) {
                LOGGER(PACKAGE).error("cannot create mojeid request without"
                        "logger request_id");
                throw;
            }
            Database::Result rnext = conn.exec(
                    "SELECT nextval('action_id_seq'::regclass)");
            if (rnext.size() != 1 || (id_ = rnext[0][0]) == 0) {
                throw;
            }

            servertrid_ = str(boost::format("MojedID-%010d") % id_);

            conn.exec_params(
                    "INSERT INTO action"
                    " (id, clientid, action, clienttrid, servertrid)"
                    " VALUES ($1::integer, NULL, $2::integer, $3::text, $4::text)",
                    Database::query_param_list
                        (id_)
                        (request_code_)
                        (clienttrid_)
                        (servertrid_));
        }
        catch (...) {
            throw std::runtime_error("unable to create request log (action)");
        }

        tx_.reset(new Database::Transaction(conn));
    }


    ~Request()
    {
        end_failure();
    }


    const unsigned long long& get_id() const
    {
        return id_;
    }


    const std::string& get_servertrid() const
    {
        return servertrid_;
    }


    const unsigned long long& get_registrar_id() const
    {
        return registrar_id_;
    }


    const unsigned long long get_request_id() const
    {
        return request_id_;
    }


    void end_prepare(const std::string &_id)
    {
        if (status_ == 0) {
            tx_->prepare(_id);
            status_ = 3;
        }
    }


    void end_success()
    {
        if (status_ == 0) {
            tx_->commit();
            status_ = 2;
            conn.exec_params(
                    "UPDATE action SET response = 1000, enddate = now()"
                    " WHERE id = $1::integer",
                    Database::query_param_list(id_));
        }
    }


    void end_failure()
    {
        if (status_ == 0) {
            tx_->rollback();
            status_ = 1;
            conn.exec_params(
                    "UPDATE action SET response = 2400, enddate = now()"
                    " WHERE id = $1::integer",
                    Database::query_param_list(id_));
        }
    }

};


}

#endif /*MOJEID_REQUEST_H_*/

