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

#ifndef TESTMODEL_H_
#define TESTMODEL_H_

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <boost/format.hpp>
#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"
#include <boost/test/included/unit_test.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include "model_files.h"
#include "model_bank_payment.h"

ModelFiles mf1, mf2;

unsigned model_insert_test()
{
    unsigned ret=0;
    //insert data
    mf1.setName("dummy_test_name");
    mf1.setPath("~/log");
    //mf1.setMimeType("application/octet-stream");//default
    mf1.setCrDate(Database::DateTime("2010-03-10 12:00:01"));
    mf1.setFilesize(50000);
    mf1.setFileTypeId(2);//invoice xml
    mf1.insert();

    Database::Connection conn = Database::Manager::acquire();
    std::string query = str(boost::format(
            "select id, name, path, mimetype, crdate, filesize, filetype "
            "from files WHERE id = %1%") % mf1.getId() );
    Database::Result res = conn.exec( query );
    if ((res.size() > 0) && (res[0].size() == 7))
    {    //check data inserted by model
        if(static_cast<unsigned long long>(res[0][0]) != mf1.getId()) ret+=1;
        if(mf1.getName().compare(res[0][1])) ret+=2;
        if(mf1.getPath().compare(res[0][2])) ret+=4;
        if(std::string("application/octet-stream").compare(res[0][3])) ret+=8;
        if(mf1.getCrDate().to_string()
            .compare(Database::DateTime(std::string(res[0][4])).to_string()))
                ret+=16;
        if(static_cast<int>(res[0][5]) != mf1.getFilesize()) ret+=32;
        if(static_cast<unsigned long long>(res[0][6]) != mf1.getFileTypeId())
            ret+=64;
    }//if res size
    else ret+=128;

    if (ret != 0 ) std::cerr << "model_insert_test ret: "<< ret << std::endl;

    return ret;
}

unsigned model_reload_test()
{
    unsigned ret=0;
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    std::string query
        = str(boost::format("UPDATE files SET name = E'', path = E'', mimetype = E'',"
        " crdate = '2000-01-01 00:00:01', filesize = 80000, fileType = 1 WHERE id = %1%")
        % mf1.getId() );
    conn.exec( query );
    tx.commit();

    mf2.setId(mf1.getId());
    mf2.reload();

    //check data from UPDATE query after reload
    if(mf2.getId() != mf1.getId()) ret+=1;
    if(mf2.getName().compare("")) ret+=2;
    if(mf2.getPath().compare("")) ret+=4;
    if(mf2.getMimeType().compare("")) ret+=8;
    if(Database::DateTime(mf2.getCrDate()).to_string().compare(
        Database::DateTime("2000-01-01 00:00:01").to_string() ))    ret+=16;
    if(mf2.getFilesize() != 80000 ) ret+=32;
    if(mf2.getFileTypeId() != 1 ) ret+=64;

    if (ret != 0 ) std::cerr << "model_reload_test ret: "<< ret << std::endl;
    return ret;
}


unsigned model_update_test()
{
    unsigned ret=0;

    mf1.setFilesize(80000);
    mf1.update();

    mf1.reload();

    //compare mf1 and mf2, it should be same,  ret=0 is OK

    if(mf1.getId() != mf2.getId()) ret+=1;
    if(mf1.getName() != mf2.getName())
    {
        std::cerr << mf1.getName() << std::endl;
        std::cerr << mf2.getName() << std::endl;
        ret+=2;
    }

    if(mf1.getPath() != mf2.getPath())
    {
        std::cerr << mf1.getPath() << std::endl;
        std::cerr << mf2.getPath() << std::endl;

        ret+=4;
    }

    if(mf1.getMimeType() != mf2.getMimeType()) ret+=8;
    if(mf1.getCrDate() != mf2.getCrDate())
    {
        std::cerr << mf1.getCrDate() << std::endl;
        std::cerr << mf2.getCrDate() << std::endl;
        ret+=16;
    }

    if(mf1.getFilesize() != mf2.getFilesize()) ret+=32;
    if(mf1.getFileTypeId() != mf2.getFileTypeId())
    {
        std::cerr << mf1.getFileTypeId() << std::endl;
        std::cerr << mf2.getFileTypeId() << std::endl;

        ret+=64;
    }


    if(ret !=0 ) std::cerr << "model_update_test ret: "<< ret << std::endl;

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    std::string query
        = str(boost::format("DELETE FROM files WHERE id = %1%")
        % mf1.getId() );
    conn.exec( query );
    tx.commit();

    return ret;

}

unsigned model_nodatareload_test()
{
    unsigned ret=0;
    mf2.reload();
    return ret;
}

unsigned model_nodataupdate_test()
{
    unsigned ret=0;
    mf2.setName("x");
    mf2.update();
    return ret;
}

///////////mbp

struct mbp_insert_data
{
    unsigned long long id; //filled by model
    unsigned statement_id;//fk bank_statement (id) - none
    unsigned account_id; //fk bank_account (id) - num 1-6
    unsigned invoice_id; //fk invoice (id) - none
    std::string account_number;//17 numletters
    std::string bank_code;//4 numletters
    int operation_code; // num 1-5
    int transfer_type; // num 1-5
    int payment_status;// num 1-6
    std::string konstsym;// 10 numletters
    std::string varsymb;// 10 numletters
    std::string specsymb;// 10 numletters
    int price;//int
    std::string account_evid;//20 numletters
    Database::Date account_date; //some date
    std::string account_memo; //64 chars
    std::string account_name; //64 chars
    Database::DateTime crtime;//timestamp
};

unsigned mbp_insert_test(ModelBankPayment& mbp1, mbp_insert_data& insert_data)
{
    unsigned ret=0;
    try
    {

        //insert data
        mbp1.setAccountId(insert_data.account_id);
        mbp1.setAccountNumber(insert_data.account_number);
        mbp1.setBankCodeId(insert_data.bank_code);
        mbp1.setCode(insert_data.operation_code);
        mbp1.setType(insert_data.transfer_type);
        mbp1.setStatus(insert_data.payment_status);
        mbp1.setKonstSym(insert_data.konstsym);
        mbp1.setVarSymb(insert_data.varsymb);
        mbp1.setSpecSymb(insert_data.specsymb);
        mbp1.setPrice(insert_data.price);
        mbp1.setAccountEvid(insert_data.account_evid);
        mbp1.setAccountDate(insert_data.account_date);
        mbp1.setAccountMemo(insert_data.account_memo);
        mbp1.setAccountName(insert_data.account_name);
        mbp1.setCrTime(insert_data.crtime);
        mbp1.insert();

        Database::Connection conn = Database::Manager::acquire();
        std::string query = str(boost::format(
                "select id, statement_id, account_id, account_number, bank_code " //0-4
                ", code, \"type\" ,status, konstsym, varsymb, specsymb, price " //5-11
                ", account_evid, account_date, account_memo, invoice_id " //12-15
                ", account_name, crtime " //16-17
                " from bank_payment WHERE id = %1%") % mbp1.getId() );
        //save id - this should not change
        insert_data.id = mbp1.getId();
        insert_data.statement_id = 0;
        insert_data.invoice_id = 0;
        Database::Result res = conn.exec( query );
        if ((res.size() > 0) && (res[0].size() == 18))
        {    //check data inserted by model
            if(insert_data.id != static_cast<unsigned long long>(res[0][0]) ) ret+=1;
            if(insert_data.statement_id
                    != static_cast<unsigned long long>(res[0][1])) ret+=2;
            if(insert_data.account_id
                    != static_cast<unsigned long long>(res[0][2])) ret+=4;
            if(insert_data.account_number.compare(res[0][3])) ret+=8;
            if(insert_data.bank_code.compare(res[0][4])) ret+=16;
            if(insert_data.operation_code != static_cast<int>(res[0][5])) ret+=32;
            if(insert_data.transfer_type != static_cast<int>(res[0][6])) ret+=64;
            if(insert_data.payment_status != static_cast<int>(res[0][7])) ret+=128;
            if(insert_data.konstsym.compare(res[0][8])) ret+=256;
            if(insert_data.varsymb.compare(res[0][9])) ret+=512;
            if(insert_data.specsymb.compare(res[0][10])) ret+=1024;
            if(Database::Money(insert_data.price)
                != Database::Money(static_cast<std::string>(res[0][11]))) ret+=2048;
            if(insert_data.account_evid.compare(res[0][12])) ret+=4096;
            if(insert_data.account_date.to_string()
                .compare(Database::Date(std::string(res[0][13])).to_string()))
                    ret+=8192;
            if(insert_data.account_memo.compare(res[0][14])) ret+=16384;
            if(insert_data.invoice_id
                    != static_cast<unsigned long long>(res[0][15])) ret+=32768;
            if(insert_data.account_name.compare(res[0][16])) ret+=65536;
            if(insert_data.crtime.to_string()
                .compare(Database::DateTime(std::string(res[0][17])).to_string()))
                    ret+=131072;
        }//if res size
        else ret+=262144;
        if (ret != 0 ) std::cerr << "mbp_insert_test ret: "<< ret << std::endl;

    }
    catch(std::exception& ex)
    {
        std::cerr << "mbp_insert_test exception reason: "<< ex.what() << std::endl;
        ret+=524288;
        throw;
    }
    catch(...)
    {
        std::cerr << "mbp_insert_test exception returning"<< std::endl;
        ret+=1048576;
        if (ret != 0 ) std::cerr << "mbp_insert_test ret: "<< ret << std::endl;
    }

    return ret;
}

unsigned mbp_reload_test(ModelBankPayment& mbp1, ModelBankPayment& mbp2)
{
    unsigned ret=0;
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        std::string query = str(boost::format("UPDATE bank_payment SET"
            " statement_id=null, account_id=null, account_number=E'', bank_code=E''"
            ", code=0, \"type\"=0, status=2, konstsym=E'', varsymb=E'', specsymb=E''"
            ", price='12345.00', account_evid=E'', account_date='2000-01-01', account_memo=E''"
            ", invoice_id=null, account_name=E'', crtime='2000-01-01 00:00:01'"
            " WHERE id = %1%") % mbp1.getId() );
        conn.exec( query );
        tx.commit();

        mbp2.setId(mbp1.getId());
        mbp2.reload();

        //check data from UPDATE query after reload
        if(mbp2.getId() != mbp1.getId()) ret+=1;
        if(mbp2.getStatementId() != 0) ret+=2;
        if(mbp2.getAccountId() != 0) ret+=4;
        if(mbp2.getAccountNumber().compare("")) ret+=8;
        if(mbp2.getBankCodeId().compare("")) ret+=16;
        if(mbp2.getCode() != 0) ret+=32;
        if(mbp2.getType() != 0) ret+=64;
        if(mbp2.getStatus() != 2) ret+=128;
        if(mbp2.getKonstSym().compare("")) ret+=256;
        if(mbp2.getVarSymb().compare("")) ret+=512;
        if(mbp2.getSpecSymb().compare("")) ret+=1024;
        if(mbp2.getPrice() != Database::Money("12345.00")) ret+=2048;
        if(mbp2.getAccountEvid().compare("")) ret+=4096;
        if(mbp2.getAccountDate() != Database::Date("2000-01-01")) ret+=8192;
        if(mbp2.getAccountMemo().compare("")) ret+=16384;
        if(mbp2.getInvoiceId() != 0) ret+=32768;
        if(mbp2.getAccountName().compare("")) ret+=65536;
        if(Database::DateTime(mbp2.getCrTime()).to_string().compare(
                Database::DateTime("2000-01-01 00:00:01").to_string() )) ret+=131072;

        if (ret != 0 ) std::cerr << "model_reload_test ret: "<< ret << std::endl;

    }
    catch(std::exception& ex)
    {
        std::cerr << "model_reload_test exception reason: "<< ex.what() << std::endl;
        ret+=262144;
        throw;
    }
    catch(...)
    {
        std::cerr << "model_reload_test exception returning"<< std::endl;
        ret+=524288;
        if (ret != 0 ) std::cerr << "model_reload_test ret: "<< ret << std::endl;
    }

    return ret;


}

unsigned mbp_update_test(ModelBankPayment& mbp1, ModelBankPayment& mbp2)
{
    unsigned ret=0;
    try
    {
        //mbp1.setAccountId(mbp2.getAccountId());
        mbp1.setStatus(mbp2.getStatus());
        mbp1.setPrice(Database::Money("12345.00"));
        mbp1.update();
        mbp1.reload();

        //compare mbp1 and mbp2, it should be same,  ret=0 is OK
        if(mbp2.getId() != mbp1.getId()) ret+=1;
        if(mbp2.getStatementId() != mbp1.getStatementId()) ret+=2;
        //if(0 != mbp1.getAccountId()) ret+=4; //unable to work with null
        if(mbp2.getAccountNumber().compare(mbp1.getAccountNumber())) ret+=8;
        if(mbp2.getBankCodeId().compare(mbp1.getBankCodeId())) ret+=16;
        if(mbp2.getCode() != mbp1.getCode()) ret+=32;
        if(mbp2.getType() != mbp1.getType()) ret+=64;
        if(mbp2.getStatus() != mbp1.getStatus()) ret+=128;
        if(mbp2.getKonstSym().compare(mbp1.getKonstSym())) ret+=256;
        if(mbp2.getVarSymb().compare(mbp1.getVarSymb())) ret+=512;
        if(mbp2.getSpecSymb().compare(mbp1.getSpecSymb())) ret+=1024;
        if(mbp2.getPrice() != mbp1.getPrice()) ret+=2048;
        if(mbp2.getAccountEvid().compare(mbp1.getAccountEvid())) ret+=4096;
        if(mbp2.getAccountDate() != mbp1.getAccountDate()) ret+=8192;
        if(mbp2.getAccountMemo().compare(mbp1.getAccountMemo())) ret+=16384;
        if(mbp2.getInvoiceId() != mbp1.getInvoiceId()) ret+=32768;
        if(mbp2.getAccountName().compare(mbp1.getAccountName())) ret+=65536;
        if(Database::DateTime(mbp2.getCrTime()).to_string().compare(
                Database::DateTime(mbp1.getCrTime()).to_string() )) ret+=131072;

        if(ret !=0 ) std::cerr << "model_update_test ret: "<< ret << std::endl;

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        std::string query = str(boost::format("DELETE FROM bank_payment WHERE id = %1%") % mbp1.getId() );
        conn.exec( query );
        tx.commit();
    }
    catch(std::exception& ex)
    {
        std::cerr << "model_update_test exception reason: "<< ex.what() << std::endl;
        ret+=262144;
        throw;
    }
    catch(...)
    {
        std::cerr << "model_update_test exception returning"<< std::endl;
        ret+=524288;
        if(ret !=0 ) std::cerr << "model_update_test ret: "<< ret << std::endl;
    }

    return ret;
}

unsigned mbp_nodatareload_test()
{
    unsigned ret=0;
    mf2.reload();
    return ret;
}

unsigned mbp_nodataupdate_test()
{
    unsigned ret=0;
    mf2.setName("x");
    mf2.update();
    return ret;
}


////////////mbp




bool check_std_exception_nodatafound(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("No data found")) != std::string::npos);
}

//http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;

#if ( BOOST_VERSION > 103401 )
    boost::condition_variable
#else
    boost::condition
#endif
        the_condition_variable;
public:
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    std::size_t size()
    {
        boost::mutex::scoped_lock lock(the_mutex);
        std::size_t ret = the_queue.size();
        lock.unlock();
        the_condition_variable.notify_one();
        return ret;
    }


    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }

        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }

        popped_value=the_queue.front();
        the_queue.pop();
    }
};




#endif // TESTMODEL_H_
