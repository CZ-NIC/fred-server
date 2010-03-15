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
        if(static_cast<unsigned long long>(res[0][6]) != mf1.getFileTypeId()) ret+=64;

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
    std::string query = str(boost::format("UPDATE files SET name = E'', path = E'', mimetype = E'',"
            " crdate = '2000-01-01 00:00:01', filesize = 80000, fileType = 1 WHERE id = %1%") % mf1.getId() );
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
    std::string query = str(boost::format("DELETE FROM files WHERE id = %1%") % mf1.getId() );
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



bool check_std_exception_nodatafound(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.find(std::string("No data found")) != std::string::npos);
}


#endif // TESTMODEL_H_
