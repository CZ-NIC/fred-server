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
 *  @bank_params.h
 *  header of bank client implementation
 */

#ifndef BANK_PARAMS_H_
#define BANK_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class ImportXMLArgs
 * \brief admin client import_xml params
 */
struct ImportXMLArgs
{
    optional_string bank_xml;//BANK_XML_FILE_NAME
    bool cr_credit_invoice;//BANK_CREATE_CREDIT_INVOICE_NAME
    optional_string bank_statement_file;//BANK_XML_FILE_STATEMENT_NAME
    optional_string bank_statement_file_mimetype;//BANK_XML_FILE_STATEMENT_MIME_NAME

    ImportXMLArgs()
    : cr_credit_invoice(false)
    {}//ctor
    ImportXMLArgs(
             const optional_string& _bank_xml
             , bool _cr_credit_invoice
             , const optional_string& _bank_statement_file
             , const optional_string& _bank_statement_file_mimetype
            )
    : bank_xml(_bank_xml)
    , cr_credit_invoice(_cr_credit_invoice)
    , bank_statement_file(_bank_statement_file)
    , bank_statement_file_mimetype(_bank_statement_file_mimetype)
    {}//init ctor
};//struct ImportXMLArgs

/**
 * \class AddAccountArgs
 * \brief admin client bank_add_account params
 */

struct AddAccountArgs
{
    std::string bank_account_number;//BANK_ACCOUNT_NUMBER_NAME
    std::string bank_code;//BANK_BANK_CODE_NAME
    optional_string account_name;//BANK_ACCOUNT_NAME_NAME
    optional_string zone_fqdn;//BANK_ZONE_NAME_NAME
    AddAccountArgs(const std::string& _bank_account_number
            , const std::string& _bank_code
            , const optional_string& _account_name
            , const optional_string& _zone_fqdn)
    : bank_account_number(_bank_account_number)
    , bank_code(_bank_code)
    , account_name(_account_name)
    , zone_fqdn(_zone_fqdn)
    {}//init ctor
    AddAccountArgs()
    {}//ctor
};//struct AddAccountArgs

#endif // BANK_PARAMS_H_
