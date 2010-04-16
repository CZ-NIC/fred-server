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

#include "simple.h"
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

#define INVOICE_ZONE_NAME_NAME          "zone_fqdn"
#define INVOICE_ZONE_NAME_NAME_DESC     "zone fully qualified domain name  (e.g. cz, com)"

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

#define INVOICE_ADD_PREFIX_NAME         "invoice_add_prefix"
#define INVOICE_ADD_PREFIX_NAME_DESC    "add row into the ``invoice_prefix'' table"
#define INVOICE_ADD_PREFIX_HELP_NAME        "invoice_add_prefix_help"
#define INVOICE_ADD_PREFIX_HELP_NAME_DESC   "help for ``add_prefix''"
#define INVOICE_PREFIX_TYPE_NAME        "type"
#define INVOICE_PREFIX_TYPE_NAME_DESC   "type"
#define INVOICE_PREFIX_YEAR_NAME        "year"
#define INVOICE_PREFIX_YEAR_NAME_DESC   "year"
#define INVOICE_PREFIX_PREFIX_NAME      "prefix"
#define INVOICE_PREFIX_PREFIX_NAME_DESC "prefix"

#define INVOICE_CREATE_INVOICE_NAME     "create_invoice"
#define INVOICE_CREATE_INVOICE_NAME_DESC "create invoice for given payment"

#define INVOICE_PAYMENT_ID_NAME         "payment_id"
#define INVOICE_PAYMENT_ID_NAME_DESC    "payment id"

namespace Admin {

class InvoiceClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    Database::ID getRegistrarId(std::string handle);
    /*
    void factoring(Register::Invoicing::Manager *man, 
            Database::ID zoneId, std::string zoneName, 
            Database::ID regId, std::string regName, 
            Database::Date toDate, Database::Date taxDate);
            */

    static const struct options m_opts[];
public:
    InvoiceClient()
    { }
    InvoiceClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~InvoiceClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void list_filters();
    void archive();
    void add_invoice_prefix();

    void credit();
    void factoring();
    void create_invoice();

    void list_help();
    void archive_help();
    void credit_help();
    void factoring_help();
    void pair_invoices_help();
    void add_invoice_prefix_help();
    void create_invoice_help();

    // added in order to make it work with old invoicing
private:
    bool factoring_all(const char *database, const char *zone_fqdn,
      const char *taxdateStr, const char *todateStr);
    int factoring(const char *database, const char *registrarHandle,
      const char *zone_fqdn, const char *taxdateStr, const char *todateStr);
    void filter_reload_invoices(Register::Invoicing::Manager *invMan, Register::Invoicing::List *invList);

}; // class InvoiceClient

} // namespace Admin;

#endif // _INVOICECLIENT_H_

