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

//#include "simple.h"
#include "corba/admin/admin_impl.h"
#include "fredlib/registry.h"
//#include "old_utils/dbsql.h"
#include "baseclient.h"

#include "invoice_params.h"

namespace Admin {

class InvoiceClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;
    std::string nameservice_context;
    InvoiceListArgs params;
    bool invoice_list;
    bool invoice_list_filters;
    bool invoice_archive;
    bool invoice_credit;
    bool invoice_billing;
    bool invoice_add_prefix;
    bool invoice_create;
    bool invoice_show_opts;

    optional_string docgen_path;
    optional_string docgen_template_path;
    optional_string fileclient_path;

    bool invoice_dont_send;

    InvoiceCreditArgs credit_params;
    InvoiceBillingArgs billing_params;
    InvoicePrefixArgs prefix_params;
    InvoiceCreateArgs create_params;

    Database::ID getRegistrarId(std::string handle);

    static const struct options m_opts[];
public:
    InvoiceClient()
    : invoice_list(false)
    , invoice_list_filters(false)
    , invoice_archive(false)
    , invoice_credit(false)
    , invoice_billing(false)
    , invoice_add_prefix(false)
    , invoice_create(false)
    , invoice_show_opts(false)
    , invoice_dont_send(false)
    { }
    InvoiceClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , const InvoiceListArgs& _params
            , bool _invoice_list
            , bool _invoice_list_filters
            , bool _invoice_archive
            , bool _invoice_credit
            , bool _invoice_billing
            , bool _invoice_add_prefix
            , bool _invoice_create
            , bool _invoice_show_opts
            , const optional_string& _docgen_path
            , const optional_string& _docgen_template_path
            , const optional_string& _fileclient_path
            , bool _invoice_dont_send
            , const InvoiceCreditArgs& _credit_params
            , const InvoiceBillingArgs& _billing_params
            , const InvoicePrefixArgs& _prefix_params
            , const InvoiceCreateArgs& _create_params
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , params(_params)
    , invoice_list(_invoice_list)
    , invoice_list_filters(_invoice_list_filters)
    , invoice_archive(_invoice_archive)
    , invoice_credit(_invoice_credit)
    , invoice_billing(_invoice_billing)
    , invoice_add_prefix(_invoice_add_prefix)
    , invoice_create(_invoice_create)
    , invoice_show_opts(_invoice_show_opts)
    , docgen_path(_docgen_path)
    , docgen_template_path(_docgen_template_path)
    , fileclient_path(_fileclient_path)
    , invoice_dont_send(_invoice_dont_send)
    , credit_params(_credit_params)
    , billing_params(_billing_params)
    , prefix_params(_prefix_params)
    , create_params(_create_params)
    {
        m_db = connect_DB(connstring
                , std::runtime_error("InvoiceClient db connection failed"));
    }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void list_filters();
    void archive();
    void add_invoice_prefix();

    void credit();
    void billing();
    void create_invoice();

    // added in order to make it work with old invoicing
private:
    int billing(const char *registrarHandle,
      const char *zone_fqdn, const char *taxdateStr, const char *todateStr);
    void filter_reload_invoices(Fred::Invoicing::Manager *invMan, Fred::Invoicing::List *invList);

}; // class InvoiceClient

void create_invoice_prefixes(CreateInvoicePrefixesArgs params);
void add_invoice_number_prefix(AddInvoiceNumberPrefixArgs params);

} // namespace Admin;

#endif // _INVOICECLIENT_H_

