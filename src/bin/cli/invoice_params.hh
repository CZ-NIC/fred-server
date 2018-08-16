/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @invoice_params.h
 *  header of invoice client implementation
 */

#ifndef INVOICE_PARAMS_HH_6969EEC06CBA4AECBFE57EEEF62DDA21
#define INVOICE_PARAMS_HH_6969EEC06CBA4AECBFE57EEEF62DDA21

#include "src/util/types/optional.hh"

/**
 * \class InvoiceListArgs
 * \brief admin client invoice_list params
 */
struct InvoiceListArgs
{
    optional_id invoice_id;
    optional_id zone_id;
    optional_string zone_fqdn;
    optional_ulonglong type;
    optional_string number;
    optional_string crdate;
    optional_string taxdate;
    optional_id registrar_id;
    optional_string registrar_handle;
    optional_id invoice_file_pdf;
    optional_id invoice_file_xml;
    optional_string invoice_file_name;
    optional_ulonglong limit;

    InvoiceListArgs()
    {}//ctor

    InvoiceListArgs(
            const optional_id& _invoice_id
            , const optional_id& _zone_id
            , const optional_string& _zone_fqdn
            , const optional_ulonglong& _type
            , const optional_string& _number
            , const optional_string& _crdate
            , const optional_string& _taxdate
            , const optional_id& _registrar_id
            , const optional_string& _registrar_handle
            , const optional_id& _invoice_file_pdf
            , const optional_id& _invoice_file_xml
            , const optional_string& _invoice_file_name
            , const optional_ulonglong& _limit
            )
    : invoice_id(_invoice_id)
    , zone_id(_zone_id)
    , zone_fqdn(_zone_fqdn)
    , type(_type)
    , number(_number)
    , crdate(_crdate)
    , taxdate(_taxdate)
    , registrar_id(_registrar_id)
    , registrar_handle(_registrar_handle)
    , invoice_file_pdf(_invoice_file_pdf)
    , invoice_file_xml(_invoice_file_xml)
    , invoice_file_name(_invoice_file_name)
    , limit(_limit)
    {}//init ctor
};//struct InvoiceListArgs

/**
 * \class InvoiceCreditArgs
 * \brief admin client invoice_credit params
 */
struct InvoiceCreditArgs
{
    optional_id zone_id;
    optional_id registrar_id;
    optional_string price;
    optional_string taxdate;

    InvoiceCreditArgs()
    {}//ctor

    InvoiceCreditArgs(
        const optional_id& _zone_id
        , const optional_id& _registrar_id
        , const optional_string& _price
        , const optional_string& _taxdate
        )
    : zone_id(_zone_id)
    , registrar_id(_registrar_id)
    , price(_price)
    , taxdate(_taxdate)
    {}
};//struct InvoiceCreditArgs

/**
 * \class InvoiceBillingArgs
 * \brief admin client invoice_billing params
 */
struct InvoiceBillingArgs
{
    optional_string zone_fqdn;
    optional_string registrar_handle;
    optional_string fromdate;
    optional_string todate;
    optional_string taxdate;
    optional_string invoicedate;


    InvoiceBillingArgs()
    {}//ctor

    InvoiceBillingArgs(
        const optional_string& _zone_fqdn
        , const optional_string& _registrar_handle
        , const optional_string& _fromdate
        , const optional_string& _todate
        , const optional_string& _taxdate
        , const optional_string& _invoicedate
        )
    : zone_fqdn(_zone_fqdn)
    , registrar_handle(_registrar_handle)
    , fromdate(_fromdate)
    , todate(_todate)
    , taxdate(_taxdate)
    , invoicedate(_invoicedate)
    {}
};//struct InvoiceBillingArgs

/**
 * \class InvoicePrefixArgs
 * \brief admin client invoice_add_prefix params
 */
struct InvoicePrefixArgs
{
    optional_ulong type;
    optional_ulong year;
    optional_ulonglong prefix;
    optional_id zone_id;
    optional_string zone_fqdn;

    InvoicePrefixArgs()
    {}//ctor

    InvoicePrefixArgs(
            const optional_ulong& _type
            , const optional_ulong& _year
            , const optional_ulonglong& _prefix
            , const optional_id& _zone_id
            , const optional_string& _zone_fqdn
        )
    : type(_type)
    , year(_year)
    , prefix(_prefix)
    , zone_id(_zone_id)
    , zone_fqdn(_zone_fqdn)
    {}
};//struct InvoicePrefixArgs


/**
 * \class CreateInvoicePrefixesArgs
 * \brief admin client create_invoice_prefixes params
 */
struct CreateInvoicePrefixesArgs
{
    bool for_current_year;

    CreateInvoicePrefixesArgs()
    : for_current_year(false)
    {}//ctor

    CreateInvoicePrefixesArgs(
        bool _for_current_year
        )
    : for_current_year(_for_current_year)
    {}
};//struct CreateInvoicePrefixesArgs

/**
 * \class AddInvoiceNumberPrefixArgs
 * \brief admin client add_invoice_number_prefix params
 */
struct AddInvoiceNumberPrefixArgs
{
    optional_ulong prefix;
    optional_string zone_fqdn;
    optional_string invoice_type_name;

    AddInvoiceNumberPrefixArgs()
    {}//ctor

    AddInvoiceNumberPrefixArgs(
            const optional_ulong& _prefix
            , const optional_string& _zone_fqdn
            , const optional_string& _invoice_type_name
        )
    : prefix(_prefix)
    , zone_fqdn(_zone_fqdn)
    , invoice_type_name(_invoice_type_name)
    {}

};//struct AddInvoiceNumberPrefixArgs

#endif
