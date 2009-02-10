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
#include "baseclient.h"


#define INVOICE_SHOW_OPTS_NAME          "invoice_show_opts"
#define INVOICE_SHOW_OPTS_NAME_DESC     "show all invoice command line options"
#define INVOICE_LIST_NAME               "invoice_list"
#define INVOICE_LIST_NAME_DESC          "list of all invoices (via buildin ``exportXML()'' function)"
#define INVOICE_LIST_FILTERS_NAME       "invoice_list_filters"
#define INVOICE_LIST_FILTERS_NAME_DESC  "list of all invoices (via filters)"
#define INVOICE_LIST_PLAIN_NAME         "invoice_list_plain"
#define INVOICE_LIST_PLAIN_NAME_DESC    "list of all invoices (via ccReg)"
#define INVOICE_ARCHIVE_NAME            "invoice_archive"
#define INVOICE_ARCHIVE_NAME_DESC       "Archive unarchived invoices"

#define INVOICE_LIST_HELP_NAME          "invoice_list_help"
#define INVOICE_LIST_HELP_NAME_DESC     "invoice_list help"
#define INVOICE_LIST_PLAIN_HELP         "invoice_list_plain_help"
#define INVOICE_LIST_PLAIN_HELP_DESC    "invoice plain list help"
#define INVOICE_ARCHIVE_HELP_NAME       "invoice_archive_help"
#define INVOICE_ARCHIVE_HELP_NAME_DESC  "invoice archive help"

#define INVOICE_TYPE_NAME               "type"
#define INVOICE_TYPE_NAME_DESC          "invoice type (1=advanced, 2=account)"
#define INVOICE_VAR_SYMBOL_NAME         "var_symbol"
#define INVOICE_VAR_SYMBOL_NAME_DESC    "variable symbol of payment"
#define INVOICE_NUMBER_NAME             "number"
#define INVOICE_NUMBER_NAME_DESC        "invoice number"
#define INVOICE_TAXDATE_NAME            "taxdate"
#define INVOICE_TAXDATE_NAME_DESC       "tax date (type ``./fred-admin --help_dates'' for further date&time information)"
#define INVOICE_TODATE_NAME             "todate"
#define INVOICE_TODATE_NAME_DESC        "to date (type ``./fred-admin --help_dates'' for further date&time information)"
#define INVOICE_ARCHIVED_NAME           "archived"
#define INVOICE_ARCHIVED_NAME_DESC      "archived flag"
#define INVOICE_OBJECT_ID_NAME          "invoice_object_id"
#define INVOICE_OBJECT_ID_NAME_DESC     "object id"
#define INVOICE_OBJECT_NAME_NAME        "object_name"
#define INVOICE_OBJECT_NAME_NAME_DESC   "object name"
#define INVOICE_ADV_NUMBER_NAME         "adv_number"
#define INVOICE_ADV_NUMBER_NAME_DESC    "advance number"
#define INVOICE_DONT_SEND_NAME          "invoice_dont_send"
#define INVOICE_DONT_SEND_NAME_DESC     "don't send mails with invoices during archivation"

#define INVOICE_FILE_ID_NAME            "invoice_file_id"
#define INVOICE_FILE_ID_NAME_DESC       "invoice file id"
#define INVOICE_FILE_NAME_NAME          "invoice_file_name"
#define INVOICE_FILE_NAME_NAME_DESC     "invoice file name"

#define INVOICE_FILE_PDF_NAME           "invoice_file_pdf"
#define INVOICE_FILE_PDF_NAME_DESC      "invoice_file_pdf"

#define INVOICE_FILE_XML_NAME           "invoice_file_xml"
#define INVOICE_FILE_XML_NAME_DESC      "invoice_file_xml"

#define INVOICE_CREDIT_NAME             "invoice_credit"
#define INVOICE_CREDIT_NAME_DESC        "create credit invoice for registrar"

#define INVOICE_CREDIT_HELP_NAME        "invoice_credit_help"
#define INVOICE_CREDIT_HELP_NAME_DESC   "invoice credit help"

#define INVOICE_ZONE_ID_NAME            "zone_id"
#define INVOICE_ZONE_ID_NAME_DESC       "zone identifier number"

#define INVOICE_ZONE_NAME_NAME          "zone_name"
#define INVOICE_ZONE_NAME_NAME_DESC     "zone name (e.g. cz, com)"

#define INVOICE_REGISTRAR_ID_NAME       "registrar_id"
#define INVOICE_REGISTRAR_ID_NAME_DESC  "registrar identifier number"

#define INVOICE_REGISTRAR_HANDLE_NAME   "registrar_handle"
#define INVOICE_REGISTRAR_HANDLE_NAME_DESC "registrar handle"

#define INVOICE_PRICE_NAME              "price"
#define INVOICE_PRICE_NAME_DESC         "price"

#define INVOICE_FACTORING_NAME          "invoice_factoring"
#define INVOICE_FACTORING_NAME_DESC     "invoice factoring"

#define INVOICE_FACTORING_HELP_NAME     "invoice_factoring_help"
#define INVOICE_FACTORING_HELP_NAME_DESC "invoice factoring help"

#define INVOICE_MAKE_PAIRS_NAME         "invoice_make_pairs"
#define INVOICE_MAKE_PAIRS_NAME_DESC    "pair invoices with statements"

namespace Admin {

class InvoiceClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;

    Database::ID getRegistrarId(std::string handle);
    void factoring(Register::Invoicing::Manager *man, 
            Database::ID zoneId, std::string zoneName, 
            Database::ID regId, std::string regName, 
            Database::Date toDate, Database::Date taxDate);
public:
    InvoiceClient();
    InvoiceClient(std::string connstring,
            std::string nsAddr);
    ~InvoiceClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void show_opts() const;
    void list();
    void list_filters();
    int archive();

    void list_help();
    void archive_help();
    void credit_help();
    void factoring_help();

    void credit();
    void factoring();
    void pair_invoices();
};

} // namespace Admin;

#endif // _INVOICECLIENT_H_

