/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INVOICECLIENT_H_
#define _INVOICECLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "register/register.h"
#include "old_utils/dbsql.h"

#define INVOICE_LIST_NAME               "invoice-list"
#define INVOICE_LIST_NAME_DESC          "list of all invoices (via filters)"
#define INVOICE_LIST_PLAIN              "invoice-list-plain"
#define INVOICE_LIST_PLAIN_DESC         "list of all invoices"
#define INVOICE_LIST_PLAIN_NAME         "invoice-list-plain"
#define INVOICE_LIST_PLAIN_NAME_DESC    "list of all invoices (via ccReg)"
#define INVOICE_ARCHIVE_NAME            "invoice-archive"
#define INVOICE_ARCHIVE_NAME_DESC       "Archive unarchived invoices"

#define INVOICE_LIST_HELP_NAME          "invoice-list-help"
#define INVOICE_LIST_HELP_NAME_DESC     "invoice list help"
#define INVOICE_LIST_PLAIN_HELP         "invoice-list-plain-help"
#define INVOICE_LIST_PLAIN_HELP_DESC    "invoice plain list help"
#define INVOICE_ARCHIVE_HELP_NAME       "invoice-archive-help"
#define INVOICE_ARCHIVE_HELP_NAME_DESC  "invoice archive help"

#define INVOICE_TYPE_NAME               "type"
#define INVOICE_VAR_SYMBOL_NAME         "var-symbol"
#define INVOICE_NUMBER_NAME             "number"
#define INVOICE_CRDATE_FROM_NAME        "crdate-from"
#define INVOICE_CRDATE_TO_NAME          "crdate-to"
#define INVOICE_TAXDATE_FROM_NAME       "taxdate-from"
#define INVOICE_TAXDATE_TO_NAME         "taxdate-to"
#define INVOICE_ARCHIVED_NAME           "archived"
#define INVOICE_OBJECT_ID_NAME          "object-id"
#define INVOICE_OBJECT_NAME_NAME        "object-name"
#define INVOICE_ADV_NUMBER_NAME         "adv-number"
#define INVOICE_DONT_SEND_NAME          "dont-send"

namespace Admin {

class InvoiceClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    InvoiceClient();
    InvoiceClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~InvoiceClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void list();
    int archive();

    void list_help();
    void archive_help();
};

} // namespace Admin;

#endif // _INVOICECLIENT_H_

