#include <string>
#include <iostream>
#include <algorithm>

#include "bank_payment_list_impl.h"
#include "bank_statement_list_impl.h"
#include "bank_manager.h"
#include "bank_common.h"
#include "invoice_manager.h"

namespace Register {
namespace Banking {


class ManagerImpl : virtual public Manager
{
private:
    /**
     *  core method for pairing payment and statement - no error checking
     *  just statement update
     */
    void _pairPaymentWithStatement(const Database::ID &payment,
                                   const Database::ID &statement)
    {
        if (payment == 0) {
            throw std::runtime_error("payment id not valid");
        }

        Database::Query uquery;
        uquery.buffer() << "UPDATE bank_item SET statement_id = ";
        if (statement == 0) {
            uquery.buffer() << "NULL";
        }
        else {
            uquery.buffer() << Database::Value(statement);
        }
        uquery.buffer() << " WHERE id = " << Database::Value(payment);

        Database::Connection conn = Database::Manager::acquire();
        Database::Result uresult = conn.exec(uquery);

        LOGGER(PACKAGE).info(
                boost::format("Pair payment id=%1% with statement id=%2%: "
                              "success!") % payment % statement);
    }


public:
    StatementList* createStatementList() const
    {
        return new StatementListImpl();
    }

    PaymentList* createPaymentList() const
    {
        return new PaymentListImpl();
    }

    bool importStatementXml(std::istream &in,
                            const std::string &path,
                            const bool &createCreditInvoice = false)
    {
        TRACE("[CALL] Register::Banking::Manager::importStatementXml(...)");
        try {
            /* load stream to string */
            in.seekg(0, std::ios::beg);
            std::ostringstream stream;
            stream << in.rdbuf();
            std::string xml = stream.str();

            /* parse */
            XMLparser parser;
            if (!parser.parse(xml)) {
                throw std::runtime_error("parser error");
            }

            XMLnode root = parser.getRootNode();
            if (root.getName().compare(STATEMENTS_ROOT) != 0) {
                throw std::runtime_error(str(boost::format(
                            "root element name is not `<%1%>'")
                            % STATEMENTS_ROOT));
            }

            /* start transaction for saving */
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            for (int i = 0; i < root.getChildrenSize(); i++) {
                /* xml to statement */
                XMLnode snode = root.getChild(i);
                StatementImplPtr statement(parse_xml_statement_part(snode));

                /* check if statement is valid or was processed in past */
                bool statement_conflict;
                bool statement_valid = statement->isValid();
                if (statement_valid) {
                    Database::ID conflict_sid(0);
                    if ((conflict_sid = statement->getConflictId()) == 0) {
                        statement_conflict = false;
                        /* XXX don't processs statements so far
                        statement->save();
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "new statement imported (id=%1% account_id=%2% "
                                "number=%3%)")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum());
                        */
                    }
                    else {
                        statement_conflict = true;
                        statement->setId(conflict_sid);
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "conflict statement found (id=%1% account_id=%2% "
                                "number=%3%) -- checking payments")
                                % statement->getId()
                                % statement->getAccountId()
                                % statement->getNum());
                    }
                }
                else {
                    statement->setId(0);
                    LOGGER(PACKAGE).info("Bank statement XML import: "
                            "no statement -- importing only payments");

                }
                unsigned long long sid = statement->getId();

                XMLnode pnodes = snode.getChild(STATEMENT_ITEMS);
                for (int n = 0; n < pnodes.getChildrenSize(); n++) {
                    /* xml to payment */
                    XMLnode pnode = pnodes.getChild(n);
                    PaymentImplPtr payment(parse_xml_payment_part(pnode));
                    payment->setAccountId(statement->getAccountId());
                    if (sid != 0) {
                        payment->setStatementId(sid);
                    }

                    Database::ID conflict_pid(0);
                    if ((conflict_pid = payment->getConflictId()) == 0) {
                        payment->save();
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "payment imported (id=%1% account=%2%/%3% "
                                "evid=%4%)")
                                % payment->getId()
                                % payment->getAccountNumber()
                                % payment->getBankCode()
                                % payment->getAccountEvid());
                    }
                    else {
                        payment->setId(conflict_pid);
                        LOGGER(PACKAGE).info(boost::format(
                                "Bank statement XML import: "
                                "conflict payment found "
                                "(id=%1% account=%2%/%3% evid=%4%)")
                                % payment->getId()
                                % payment->getAccountNumber()
                                % payment->getBankCode()
                                % payment->getAccountEvid());
                        /* XXX don't process statements so far
                        if (statement_valid) {
                            if (!statement_conflict) {
                                // payment exists and statement was saved
                                // need to pair payment with statement
                                _pairPaymentWithStatement(payment->getId(), statement->getId());
                            }
                            else {
                                // payment exists and statement was existing also
                                // is it paired already?
                            }
                        }
                        */
                    }
                }
            }

            /* upload file via file manager and update statement */

            // if (createCreditInvoice) {
            //     std::auto_ptr<Invoicing::Manager>
            //         invMan(Invoicing::Manager::create());
            //     return invMan->pairInvoices();
            // }

            tx.commit();
            return true;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("Bank statement XML import: %1%")
                                    % ex.what());
            return false;
        }
        catch (...) {
            LOGGER(PACKAGE).error("Bank statement XML import: an error occured");
            return false;
        }
    }

    bool insertBankAccount(const std::string &zone,
                           const std::string &account_number,
                           const std::string &account_name,
                           const std::string &bank_code)
    {
        TRACE("[CALL] Register::Banking::Manager::insertBankAccount(zone_fqdn, ...)");
        Database::Query query;
        query.buffer()
            << "SELECT id FROM zone WHERE fqdn=" << Database::Value(zone);
        try {
            Database::Connection conn = Database::Manager::acquire();
            Database::Result res = conn.exec(query);
            if (res.size() == 0) {
                LOGGER(PACKAGE).error(boost::format("Unknown zone (%1%)") % zone);
                return false;
            }
            Database::ID zoneId = res[0][0];
            return insertBankAccount(zoneId, account_number, account_name, bank_code);
        } catch (...) {
            LOGGER(PACKAGE).error("An exception was catched");
            return false;
        }
        return false;
    }

    bool insertBankAccount(const Database::ID &zone,
                           const std::string &account_number,
                           const std::string &account_name,
                           const std::string &bank_code)
    {
        TRACE("[CALL] Register::Banking::Manager::insertBankAccount(zone_id, ...)");
        Database::InsertQuery insertAccount("bank_account");
        insertAccount.add("zone", zone);
        insertAccount.add("account_number", account_number);
        insertAccount.add("account_name", account_name);
        insertAccount.add("bank_code", bank_code);
        Database::Connection conn = Database::Manager::acquire();
        try {
            conn.exec(insertAccount);
        } catch (...) {
            LOGGER(PACKAGE).error("Cannot insert new account into the ``bank_account'' table");
            return false;
        }
        return true;
    }

    bool moveItemToPayment(
        const Database::ID &payment,
        const Database::ID &statement,
        bool force = false)
    {
        return pairPaymentWithStatement(payment, statement, force);
    }

    bool pairPaymentWithStatement(const Database::ID &payment,
                                  const Database::ID &statement,
                                  bool force = false)
    {
        TRACE("[CALL] Register::Banking::Manager::pairPaymentWithStatement(...)");
        if (payment == 0) {
            throw std::runtime_error("payment id not valid");
        }

        Database::Connection conn = Database::Manager::acquire();
        try {
            Database::Query pquery;
            pquery.buffer() << "SELECT id, statement_id FROM bank_item WHERE id = "
                            << Database::Value(payment);

            Database::Result presult = conn.exec(pquery);
            if (presult.size() == 0) {
                throw std::runtime_error("payment doesn't exist");
            }

            if (statement > 0) {
                Database::Query squery;
                squery.buffer() << "SELECT id FROM bank_head WHERE id = "
                                << Database::Value(statement);

                Database::Result sresult = conn.exec(squery);
                if (sresult.size() == 0) {
                    throw std::runtime_error("statement doesn't exist");
                }
            }

            unsigned long long payment_sid = presult[0][1];
            if (payment_sid > 0) {
                if (force) {
                    LOGGER(PACKAGE).info(
                            boost::format("Pair payment id=%1% with statement id=%2%: "
                                          "payment is listed on statement id=%3% (FORCING)")
                                           % payment % statement % payment_sid);
                }
                else {
                    throw std::runtime_error(str(boost::format(
                                    "payment is listed on statement id=%3% "
                                    "(try run with `force' parameter)")
                                    % payment % statement % payment_sid));
                }
            }

            /* update payment */
            _pairPaymentWithStatement(payment, statement);
            return true;
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(
                    boost::format("Cannot pair payment id=%1% with statement id=%2%: %3%")
                                  % payment % statement % ex.what());
            return false;
         }
        catch (...) {
            LOGGER(PACKAGE).error(
                    boost::format("Cannot pair payment id=%1% with statement id=%2%: "
                                  "exception caught") % payment % statement);
            return false;
        }
    }
};

Manager* Manager::create()
{
    return new ManagerImpl();
}

} // namespace Banking
} // namespace Register

