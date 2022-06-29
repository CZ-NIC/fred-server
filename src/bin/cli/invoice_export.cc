/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/bin/cli/invoice_export.hh"

#include "src/deprecated/libfred/invoicing/invoice.hh"

#include "libfiled/libfiled.hh"
#include "libhermes/libhermes.hh"
#include "libtypist/libtypist.hh"
#include "src/deprecated/util/dbsql.hh"
#include "util/db/result.hh"
#include "libfred/opcontext.hh"
#include "util/db/query_param.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "util/decimal/decimal.hh"

#include <boost/optional.hpp>

namespace Admin {

namespace {

enum InvoiceType {
  IT_DEPOSIT = 0,
  IT_ACCOUNT = 1
};

struct Subject
{
    unsigned long long id;
    std::string handle;
    std::string name;
    std::string fullname;
    std::string street;
    std::string city;
    std::string zip;
    std::string country;
    std::string ico;
    std::string vat_number;
    std::string registration;
    std::string reclamation;
    std::string email;
    std::string url;
    std::string phone;
    std::string fax;
    bool vat_apply;
};

struct Payment
{
    Money price;
    Decimal vat_rate;
    Money vat;
};

struct PaymentSource
{
    Money price;
    Decimal vat_rate;
    Money vat;

    unsigned long long number;
    Money credit;
    unsigned long long id;
    Money total_price;
    Money total_vat;
    boost::posix_time::ptime crtime;
};

//this relies on PaymentActionType (operation_id - 1) in invoice_add_action
enum PaymentActionType {
        PAT_CREATE_DOMAIN
        , PAT_RENEW_DOMAIN
        , PAT_REQUESTS_OVER_LIMIT
        , PAT_ADMINISTRATIVE_FEE
        , PAT_FEE 
        , PAT_MONTHLY_FEE
};

struct PaymentAction
{
    Money price;
    Decimal vat_rate;
    Money vat;

    std::string object_name;
    boost::posix_time::ptime action_time;
    boost::gregorian::date from_date;
    boost::gregorian::date exdate;
    PaymentActionType action;
    unsigned units_count;
    Money price_per_unit;
    unsigned long long object_id;
};

struct Invoice
{
    //DB *dbc;
    unsigned long long id;
    unsigned long long zone;
    std::string zone_name;
    boost::posix_time::ptime crtime;
    boost::gregorian::date tax_date;
    boost::gregorian::date_period account_period;
    InvoiceType type;
    unsigned long long number;
    unsigned long long registrar;
    Money credit;
    Money price;
    Decimal vat_rate;
    Money total;
    Money total_vat;
    //unsigned long long filePDF;
    //unsigned long long fileXML;
    boost::optional<boost::uuids::uuid> file_pdf_uuid;
    boost::optional<boost::uuids::uuid> file_xml_uuid;
    std::string var_symbol;
    Subject client;
    //static Subject supplier;
    std::vector<PaymentSource> sources;
    std::vector<PaymentAction> actions;
    std::vector<Payment> paid; ///< list of paid vat rates
};

static const constexpr char* invoice_query =
        "SELECT vat, total, totalvat, operations_price FROM invoice WHERE id=$1::BIGINT";

static const constexpr char* account_invoice_with_annual_partitioning_query =
        // clang-format off
 "SELECT moo.year, "
       "moo.chi_vat_rate, "
       "SUM(moo.chi_price_novat) AS chi_price_novat, "
       "SUM(moo.chi_price_vat) AS chi_price_vat, "
       "SUM(moo.chi_price_vat) - SUM(moo.chi_price_novat) AS chi_vat "
  "FROM ( "
        "SELECT baz.year, "
               "baz.chi_vat_rate, "
               "baz.chi_vat_alg, "
               "baz.chi_price_novat, "
               "CASE WHEN baz.chi_vat_alg = 'coef' "
                    "THEN baz.chi_price_novat * (1 / (1 - (SELECT koef FROM price_vat WHERE vat = baz.chi_vat_rate))) "
                    "WHEN baz.chi_vat_alg = 'math' "
                    "THEN baz.chi_price_novat * (1 + (baz.chi_vat_rate / 100)) "
                    "WHEN baz.chi_vat_alg = 'novat' "
                    "THEN baz.chi_price_novat "
               "END AS chi_price_vat "
          "FROM ( "
                "SELECT bar.year, bar.chi_vat_rate, bar.chi_vat_alg, SUM(bar.chi_price_novat) AS chi_price_novat "
                  "FROM ( "
                        "SELECT foo.invoice_operation_id, "
                               "foo.ad_invoice_id, "
                               "foo.chi_vat_rate, "
                               "foo.chi_vat_alg, "
                               "foo.year, "
                               "foo.price * (foo.days_per_year / (SUM(foo.days_per_year) OVER (PARTITION BY foo.invoice_operation_id, foo.ad_invoice_id))) AS chi_price_novat "
                          "FROM ( "
                                "SELECT iocm.invoice_operation_id, "
                                       "iocm.invoice_id AS ad_invoice_id, "
                                       "iocm.price, "
                                       "chi.vat AS chi_vat_rate, "
                                       "CASE WHEN chi.vat > 0 "
                                            "THEN CASE WHEN chi.crdate < (('2019-10-01 00:00:00' AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague') "
                                                           "or abs(((chi.total + chi.totalvat) * (1 - 1/(1 + chi.vat/100)))::NUMERIC(10,2) - chi.totalvat) > 0.01 "
                                                      "THEN 'coef' "
                                                      "ELSE 'math' "
                                                 "END "
                                            "ELSE 'novat' "
                                       "END AS chi_vat_alg, "
                                       "EXTRACT(YEAR FROM generate_series(io.date_from + '1 day'::INTERVAL, io.date_to, '1 day'::INTERVAL)) AS year, "
                                       "COUNT(*) AS days_per_year "
                                  "FROM invoice_operation io "
                                  "JOIN invoice_operation_charge_map iocm ON iocm.invoice_operation_id = io.id "
                                  "JOIN invoice chi ON chi.id = iocm.invoice_id " // chi can be advance or account invoice
                                  "JOIN invoice_prefix ip ON chi.invoice_prefix_id = ip.id "
                                 "WHERE io.ac_invoice_id = $1::BIGINT "
                                 "GROUP BY 1,2,3,4,5,6 "
                                ") AS foo "
                       ") AS bar "
                   "GROUP BY 1,2,3 "
                   "ORDER BY 1,2,3 "
               ") AS baz "
       ") AS moo "
 "group by 1,2";
// clang-format on

//void export_invoice(const Invoice& _invoice, std::ostream _out)
//{
//    LibFiled::Connection<LibFiled::Service::File> fileman_connection{
//            LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
//                    _fileman_args.endpoint}};
//
//    LibFiled::File::read(
//            fileman_connection,
//            LibFiled::File::FileUuid{_invoice.file_xml_uuid},
//            _out);
//}

LibTypist::Struct make_subject_context(const Subject& _subject)
{
    return LibTypist::Struct{
            {LibTypist::StructKey{"organization"}, LibTypist::StructValue{_subject.fullname}},
            {LibTypist::StructKey{"address"},
                    LibTypist::StructValue{LibTypist::Struct{
                            {LibTypist::StructKey{"street"}, LibTypist::StructValue{_subject.street}},
                            {LibTypist::StructKey{"postal_code"}, LibTypist::StructValue{_subject.zip}},
                            {LibTypist::StructKey{"city"}, LibTypist::StructValue{_subject.city}},
                            {LibTypist::StructKey{"country"}, LibTypist::StructValue{_subject.country}}}}},
            {LibTypist::StructKey{"vat_identification_number"}, LibTypist::StructValue{_subject.vat_number}},
            {LibTypist::StructKey{"company_registration_number"}, LibTypist::StructValue{_subject.ico}}};
    // TODO getRegistration?
    // TODO getReclamation?
}

LibTypist::Struct make_invoice_context(const Invoice& _invoice)
{
    if (_invoice.crtime.date().is_special())
    {
        throw std::runtime_error("doExport invoice crtime date is special");
    }
    if (_invoice.tax_date.is_special())
    {
        throw std::runtime_error("doExport invoice taxdate is special");
    }
    if (_invoice.type == IT_ACCOUNT)
    {
        if (_invoice.account_period.begin().is_special())
        {
            throw std::runtime_error("doExport invoice period_from is special");
        }
        if (_invoice.account_period.end().is_special())
        {
            throw std::runtime_error("doExport invoice period_to is special");
        }
    }
    // setting locale for proper date and time format
    // do not use system locale - locale("") because of
    // unpredictable formatting behavior
    //_out.imbue(std::locale(std::locale(_out.getloc(), new time_facet("%Y-%m-%d %T")), new boost::gregorian::date_facet("%Y-%m-%d")));

    const auto buyer_context = make_subject_context(_invoice.client);
    //const auto supplier_context = make(_invoice.supplier); // obsolete

    auto invoice_context =
            LibTypist::Struct{
                    {LibTypist::StructKey{"number"}, LibTypist::StructValue{boost::lexical_cast<std::string>(_invoice.number)}},
                    {LibTypist::StructKey{"variable_symbol"}, LibTypist::StructValue{_invoice.var_symbol}},
                    {LibTypist::StructKey{"date"}, LibTypist::StructValue{to_iso_extended_string(_invoice.crtime.date())}},
                    {LibTypist::StructKey{"tax_date"}, LibTypist::StructValue{to_iso_extended_string(_invoice.tax_date)}}};

    if (_invoice.type == IT_ACCOUNT)
    {
        invoice_context[LibTypist::StructKey{"period_from"}] = LibTypist::StructValue{to_iso_extended_string(_invoice.account_period.begin())};
        invoice_context[LibTypist::StructKey{"period_to"}] = LibTypist::StructValue{to_iso_extended_string(_invoice.account_period.end())};
    }

    std::vector<LibTypist::StructValue> vat_rates;

    struct MoneyRecord
    {
        Money price_without_vat;
        Money price_with_vat;
        Money vat;
    };

    if (_invoice.type == IT_ACCOUNT)
    {
        struct AccountInvoiceRecord
        {
            Decimal vat_rate;
            MoneyRecord self;
            Money operations_price;
            using YearRecord = std::map<unsigned, MoneyRecord>;
            using VatrateRecord = std::map<Decimal, YearRecord>;
            VatrateRecord records;
        };

        Database::Connection conn = Database::Manager::acquire();
        AccountInvoiceRecord account_invoice_record;
        {
            const Database::Result result =
                    conn.exec_params(
                            invoice_query,
                            Database::query_param_list(_invoice.id));

            if (result.size() < 1)
            {
                throw std::runtime_error("ExporterArchiver::doExport IT_ACCOUNT query failed");
            }
            constexpr auto row = 0;
            account_invoice_record.vat_rate = Decimal(static_cast<std::string>(result[row][0]));
            account_invoice_record.self.price_without_vat = Money(static_cast<std::string>(result[row][1]));
            account_invoice_record.self.vat = Money(static_cast<std::string>(result[row][2]));
            account_invoice_record.self.price_with_vat = account_invoice_record.self.price_without_vat + account_invoice_record.self.vat;
            account_invoice_record.operations_price = Decimal(static_cast<std::string>(result[row][3]));
        }

        const Database::Result result =
                conn.exec_params(
                        account_invoice_with_annual_partitioning_query,
                        Database::query_param_list(_invoice.id));

        if (result.size() > 0)
        {

            for (std::size_t row = 0; row < result.size(); ++row)
            {
                const auto year = static_cast<unsigned>(result[row][0]);
                const auto vat_rate = Decimal(static_cast<std::string>(result[row][1]));
                account_invoice_record.records[vat_rate][year].price_without_vat = Money(static_cast<std::string>(result[row][2]));
                account_invoice_record.records[vat_rate][year].price_with_vat = Money(static_cast<std::string>(result[row][3]));
                account_invoice_record.records[vat_rate][year].vat = Money(static_cast<std::string>(result[row][4]));
            }

            for (const auto& vat_rate_record : account_invoice_record.records)
            {
                const auto vat_rate = vat_rate_record.first;

                MoneyRecord all_years_money_record{Money("0"), Money("0"), Money("0")};
                for (const auto& year_record : vat_rate_record.second)
                {
                    const auto year_money_record = year_record.second;
                    all_years_money_record.price_without_vat += year_money_record.price_without_vat;
                    all_years_money_record.price_with_vat += year_money_record.price_with_vat;
                    all_years_money_record.vat += year_money_record.vat;
                }

                LibTypist::Struct vat_rates_entry_context;

                if (account_invoice_record.vat_rate == vat_rate) // add this account invoice to vat details
                {
                    vat_rates_entry_context[LibTypist::StructKey{"vat_rate"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(account_invoice_record.vat_rate)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_base"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(account_invoice_record.self.price_without_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(account_invoice_record.self.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.price_with_vat - account_invoice_record.self.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.vat - account_invoice_record.self.vat)};
                }
                else
                {
                    vat_rates_entry_context[LibTypist::StructKey{"vat_rate"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(vat_rate)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_base"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.price_without_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(all_years_money_record.vat)};
                }

                std::vector<LibTypist::StructValue> deffered_income_context;
                for (const auto& year_record : account_invoice_record.records[vat_rate])
                {
                    const auto year = year_record.first;
                    const auto year_money_record = year_record.second;

                    const LibTypist::Struct year_context{
                            {LibTypist::StructKey{"year"}, LibTypist::StructValue{std::to_string(year)}},
                            {LibTypist::StructKey{"base"}, LibTypist::StructValue{boost::lexical_cast<std::string>(year_money_record.price_without_vat)}},
                            {LibTypist::StructKey{"vat"}, LibTypist::StructValue{boost::lexical_cast<std::string>(year_money_record.vat)}},
                            {LibTypist::StructKey{"total"}, LibTypist::StructValue{boost::lexical_cast<std::string>(year_money_record.price_with_vat)}}};
                    deffered_income_context.push_back(LibTypist::StructValue{year_context});
                }
                vat_rates_entry_context[LibTypist::StructKey{"deffered_income"}] = LibTypist::StructValue{deffered_income_context};
                vat_rates.push_back(LibTypist::StructValue{vat_rates_entry_context});
            }
        }

        invoice_context[LibTypist::StructKey{"total"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(account_invoice_record.operations_price)};
    }
    else // IT_DEPOSIT
    {
        struct AdvanceInvoiceRecord
        {
            MoneyRecord self;
            Money operations_price;
        };

        Database::Connection conn = Database::Manager::acquire();
        Database::Result result =
                conn.exec_params(
                        invoice_query,
                        Database::query_param_list(_invoice.id));

        if (result.size() != 1)
        {
            throw std::runtime_error("ExporterArchiver::doExport IT_DEPOSIT query failed");
        }

        AdvanceInvoiceRecord advance_invoice_record;

        constexpr auto row = 0;
        const auto vat_rate = Decimal(static_cast<std::string>(result[row][0]));
        advance_invoice_record.self.price_without_vat = Money(static_cast<std::string>(result[row][1]));
        advance_invoice_record.self.vat = Money(static_cast<std::string>(result[row][2]));
        advance_invoice_record.self.price_with_vat = advance_invoice_record.self.price_without_vat + advance_invoice_record.self.vat;
        advance_invoice_record.operations_price = Decimal(static_cast<std::string>(result[row][3]));

        LibTypist::Struct vat_rates_entry_context{
                {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(vat_rate)}},
                {LibTypist::StructKey{"base"}, LibTypist::StructValue{boost::lexical_cast<std::string>(advance_invoice_record.self.price_without_vat)}},
                {LibTypist::StructKey{"vat"}, LibTypist::StructValue{boost::lexical_cast<std::string>(advance_invoice_record.self.vat)}},
                {LibTypist::StructKey{"total"}, LibTypist::StructValue{boost::lexical_cast<std::string>(advance_invoice_record.self.price_with_vat)}}};

        vat_rates.push_back(LibTypist::StructValue{vat_rates_entry_context});
    }

    std::vector<LibTypist::StructValue> advance_invoices;
    if (_invoice.sources.size())
    {
        for (unsigned source_idx = 0; source_idx < _invoice.sources.size(); source_idx++)
        {
            const auto payment_source = _invoice.sources.at(source_idx);
            advance_invoices.push_back(LibTypist::StructValue{LibTypist::Struct{
                    {LibTypist::StructKey{"number"}, LibTypist::StructValue{std::to_string(payment_source.number)}},
                    {LibTypist::StructKey{"consumed_base"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.price)}},
                    {LibTypist::StructKey{"balance"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.credit)}},
                    {LibTypist::StructKey{"consumed_vat"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.vat)}},
                    {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.vat_rate)}},
                    {LibTypist::StructKey{"consumed"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.price + payment_source.vat)}},
                    {LibTypist::StructKey{"total_base"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.total_price)}},
                    {LibTypist::StructKey{"total_vat"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.total_vat)}},
                    {LibTypist::StructKey{"total"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.total_price + payment_source.total_vat)}},
                    {LibTypist::StructKey{"datetime"}, LibTypist::StructValue{to_iso_extended_string(payment_source.crtime)}}
            }});
        }
    }
    std::vector<LibTypist::StructValue> items_context;
    if (_invoice.actions.size())
    {
        for (unsigned action_idx = 0; action_idx < _invoice.actions.size(); action_idx++)
        {
            const auto payment_action = _invoice.actions.at(action_idx);
            LibTypist::Struct item_context{
                    {LibTypist::StructKey{"subject"}, LibTypist::StructValue{payment_action.object_name}},
                    // clang-format off
                    {LibTypist::StructKey{"code"}, LibTypist::StructValue{std::string{
                            (payment_action.action == PAT_CREATE_DOMAIN ? "RREG"
                            : (payment_action.action == PAT_RENEW_DOMAIN ? "RUDR"
                              : (payment_action.action == PAT_REQUESTS_OVER_LIMIT ? "REPP"
                                : (payment_action.action == PAT_ADMINISTRATIVE_FEE ? "RPOA"
                                  : (payment_action.action == PAT_FEE ? "RPOP"
                                    : (payment_action.action == PAT_MONTHLY_FEE ? "RPOP"
                                      : "RUNK")
                                    )
                                  )
                                )
                              )
                            )}
                    }},
                    // clang-format on
                    {LibTypist::StructKey{"datetime"}, LibTypist::StructValue{to_iso_extended_string(payment_action.action_time)}},
                    {LibTypist::StructKey{"count"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_action.units_count)}},
                    {LibTypist::StructKey{"price"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_action.price_per_unit)}},
                    {LibTypist::StructKey{"total"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_action.price)}},
                    {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_action.vat_rate)}}};
            if (!payment_action.exdate.is_special())
            {
                item_context[LibTypist::StructKey{"expiration_date"}] = LibTypist::StructValue{to_iso_extended_string(payment_action.exdate)};
            };
            items_context.push_back(LibTypist::StructValue{item_context});
        }
    }

    return LibTypist::Struct{
            {LibTypist::StructKey{"context"},
                    LibTypist::StructValue{LibTypist::Struct{
                            {LibTypist::StructKey{"buyer"}, LibTypist::StructValue{buyer_context}},
                            //{LibTypist::StructKey{"supplier"}, // LibTypist::StructValue{supplier_context}},
                            {LibTypist::StructKey{"vat_ratest"}, LibTypist::StructValue{vat_rates}},
                            {LibTypist::StructKey{"invoice"}, LibTypist::StructValue{invoice_context}},
                            {LibTypist::StructKey{"delivery"}, LibTypist::StructValue{vat_rates}},
                            {LibTypist::StructKey{"advance_invoices"}, LibTypist::StructValue{advance_invoices}},
                            {LibTypist::StructKey{"items"}, LibTypist::StructValue{items_context}}}}}};
}

void invoice_save_file_uuid(unsigned long long _invoice_id, const boost::uuids::uuid& _file_pdf_uuid, const boost::uuids::uuid& _file_xml_uuid)
{
    Logging::Context ctx("invoice_save_file_uuid");
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::QueryParams sql_params;
        std::stringstream sql;

        sql_params.push_back(boost::lexical_cast<std::string>(_file_pdf_uuid));
        sql << "UPDATE invoice SET file_uuid=$" << sql_params.size() << "::UUID ";

        sql_params.push_back(boost::lexical_cast<std::string>(_file_xml_uuid));
        sql << ",file_xml_uuid=$" << sql_params.size() << "::UUID ";

        sql_params.push_back(boost::lexical_cast<std::string>(_invoice_id));
        sql << " WHERE id=$" << sql_params.size() << "::BIGINT ";

        conn.exec_params(sql.str(), sql_params);

      }
      catch (std::exception& ex)
      {
          LOGGER.error(boost::format("invoice_save_file_uuid failed: %1% ") % ex.what());
          throw std::runtime_error(std::string("invoice_save_file_uuid failed: ") + ex.what());
      }
      catch (...)
      {
          LOGGER.error("invoice_save_file_uuid failed.");
          throw std::runtime_error("invoice_save_file_uuid failed");
      }
}

void export_and_archive_invoice(
        const SecretaryArgs& _secretary_args,
        const FilemanArgs& _fileman_args,
        const Invoice& _invoice)
{
    try
    {
        const auto invoice_context = make_invoice_context(_invoice);

        LibTypist::Connection secretary_connection{
                LibTypist::Connection::ConnectionString{_secretary_args.endpoint}};

        const auto template_name_html =
                LibTypist::TemplateName{std::string{
                        (_invoice.type == IT_DEPOSIT ? "advance-invoice" : "invoice")} +
                        (_invoice.client.country == "CZ" ? "-cs" : "-en") + ".html"};

        std::stringstream file_pdf_rawdata;
LOGGER.debug("DEBUG2 render1 "+*template_name_html);
        LibTypist::render(
                secretary_connection,
                template_name_html,
                LibTypist::ContentType{"application/pdf"},
                invoice_context,
                file_pdf_rawdata);
std::ofstream output_pdf(std::to_string(_invoice.number) + ".pdf", std::ofstream::out);
output_pdf << file_pdf_rawdata.str();
output_pdf.close();

        const auto template_name_xml =
                LibTypist::TemplateName{_invoice.type == IT_DEPOSIT ? "advance-invoice.xml" : "invoice.xml"};

        std::stringstream file_xml_rawdata;
LOGGER.debug("DEBUG3 render2");
        LibTypist::render(
                secretary_connection,
                template_name_xml,
                LibTypist::ContentType{"text/html"},
                invoice_context,
                file_xml_rawdata);
std::ofstream output_xml(std::to_string(_invoice.number) + ".xml", std::ofstream::out);
output_xml << file_xml_rawdata.str();
output_xml.close();


LOGGER.debug("DEBUG3: "+_fileman_args.endpoint);
        LibFiled::Connection<LibFiled::Service::File> fileman_connection{
                LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
                        _fileman_args.endpoint}};

LOGGER.debug("DEBUG4 create file pdf");
        const auto file_pdf_uuid =
                LibFiled::File::create(
                        fileman_connection,
                        LibFiled::File::FileName{std::to_string(_invoice.number) + ".pdf"},
                        file_pdf_rawdata,
                        LibFiled::File::FileMimeType{"application/pdf"});

        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: pdf file uuid: %1% ") % boost::uuids::to_string(*file_pdf_uuid)));

LOGGER.debug("DEBUG4 create file xml");
        const auto file_xml_uuid =
                LibFiled::File::create(
                        fileman_connection,
                        LibFiled::File::FileName{std::to_string(_invoice.number) + ".xml"},
                        file_xml_rawdata,
                        LibFiled::File::FileMimeType{"application/xml"});

        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: xml file uuid: %1% ") % boost::uuids::to_string(*file_xml_uuid)));

        invoice_save_file_uuid(_invoice.id, *file_pdf_uuid, *file_xml_uuid);
    }
    catch (...)
    {
        LOGGER.error("Exception in export_and_archive_invoice");
        throw;
    }
}

//void load_invoice_xml(const Invoice& _invoice, std::ostream& _out)
//{
//    LibFiled::Connection<LibFiled::Service::File> fileman_connection{
//            LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
//                    _fileman_args.endpoint}};
//
//    LibFiled::File::read(
//            fileman_connection,
//            LibFiled::File::FileUuid{invoice.file_xml_uuid},
//            out);
//}

enum ArchiveFilter
{
    AF_IGNORE,
    AF_SET,
    AF_UNSET
};

bool do_use_coef(boost::posix_time::ptime crtime, Decimal vat_rate, Money total_without_vat, Money total_vat)
{
      static const auto use_coef_before =
              boost::posix_time::ptime(
                      boost::gregorian::date(2019, boost::gregorian::Oct, 1),
                      boost::posix_time::hours(-2)); // CEST

      if (crtime < use_coef_before)
      {
          return true;
      }

      const auto total_possibly_using_coef = total_without_vat + total_vat;
      const auto total_using_math = total_without_vat + total_without_vat * (vat_rate / Decimal("100"));
      static const auto round_err_max = Money("0.01");
      const auto total_definitely_used_coef = (total_possibly_using_coef - total_using_math).abs() > round_err_max;

      return total_definitely_used_coef;
}

struct Vat
{
    Decimal vat_rate;
    Decimal koef;
    boost::gregorian::date validity;
};

std::vector<Vat> vatList;

void init_vat_list()
{
    if (vatList.empty())
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec("SELECT vat, koef, valid_to FROM price_vat");
        for (unsigned i = 0; i < res.size(); ++i)
        {
            vatList.push_back(Vat{
                    Decimal(std::string(res[i][0])),
                    Decimal(std::string(res[i][1])),
                    boost::gregorian::date(res[i][2].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(res[i][2]))});
        }
    }
}

const Vat * get_vat(Decimal rate) {
    // late initialization would brake constness
    init_vat_list();
    std::vector<Vat>::const_iterator ci = find_if(vatList.begin(), vatList.end(), [&rate](const Vat& v){ return v.vat_rate == rate; });
    return ci == vatList.end() ? NULL : &(*ci);
}

Money count_vat(Money price_without_vat, Decimal vat_rate, bool use_coef) {
    Money vat;
    if (!use_coef) {
        vat = price_without_vat * (vat_rate / Decimal("100"));
        LOGGER.debug(
            std::string("count_vat: ")+ vat.get_string()
            + " price: " + price_without_vat.get_string()+ " vat_rate: " + vat_rate.get_string()
            + " base: true use_coef: false");
    }
    else {
        const Vat *v = get_vat(vat_rate);
        Decimal coef = v ? v->koef : Decimal("0");
        vat = (price_without_vat * coef / (Decimal("1") - coef)).round_half_up(2);
        LOGGER.debug(
            std::string("count_vat: ")+ vat.get_string()
            + " price: " + price_without_vat.get_string()+ " vat_rate: " + vat_rate.get_string()
            + " base: true use_coef: true coef: " + coef.get_string());
    }
    return vat;
}

void invoice_add_action(Invoice& invoice, Database::Row::Iterator& _col) {
    std::string _price = std::string(*_col);
    std::string _vat_rate = std::string(*(++_col));
    std::string object_name = *(++_col);
    Database::DateTime action_time = *(++_col);
    Database::Date fromdate = *(++_col);
    Database::Date exdate = *(++_col);
    int operation_id = *(++_col);
    PaymentActionType type = PaymentActionType(operation_id - 1); //PaymentActionType = id -1

    unsigned units = *(++_col);
    std::string _price_per_unit = std::string(*(++_col));
    Database::ID id = *(++_col);

    Money price(_price);
    Decimal vat_rate(_vat_rate);
    Money price_per_unit(_price_per_unit);

    LOGGER.debug(
        boost::format(
        "invoice_add_action _price %1% _vat_rate %2% object_name %3% PaymentActionType %4%"
        " units %5% _price_per_unit %6% id %7%")
        % _price
        % _vat_rate
        % object_name
        % type
        % units
        % _price_per_unit
        % id
    );

    const constexpr bool use_coef = false;
    invoice.actions.push_back(
            PaymentAction{
                    price,
                    vat_rate,
                    count_vat(price, vat_rate, use_coef),
                    object_name,
                    action_time,
                    fromdate,
                    exdate,
                    type,
                    units,
                    price_per_unit,
                    id});
  }

void invoice_add_source(Invoice& invoice, Database::Row::Iterator& _col)
{
    std::string _price = std::string(*_col);
    std::string _vat_rate = std::string(*(++_col));
    unsigned long long number = *(++_col);
    std::string _credit = std::string(*(++_col));
    Database::ID id = *(++_col);
    std::string _total_price = std::string(*(++_col));
    std::string _total_vat = std::string(*(++_col));
    Database::DateTime crtime = *(++_col);

    Money price(_price);
    Decimal vat_rate(_vat_rate);
    Money credit(_credit);
    Money total_price(_total_price);
    Money total_vat(_total_vat);

    const bool use_coef = do_use_coef(crtime.get(), vat_rate, total_price, total_vat);

    LOGGER.debug(
        boost::format(
        "invoice_add_source _price %1% _vat_rate %2% number %3% _credit %4%"
        " _total_price %5% _total_vat %6% id %7% use_coef %8%")
        % _price
        % _vat_rate
        % number
        % _credit
        % _total_price
        % _total_vat
        % id
        % use_coef
    );

    const auto vat = count_vat(price, vat_rate, use_coef);

    invoice.sources.push_back(
            PaymentSource{
                    price,
                    vat_rate,
                    vat,
                    number,
                    credit,
                    id,
                    total_price,
                    total_vat,
                    crtime});

    // init vat groups, if vat rate exists, add it, otherwise create new
    const auto payment = find_if(invoice.paid.begin(), invoice.paid.end(), [&vat_rate](const Payment& p){ return p.vat_rate == vat_rate; });
    if (payment != invoice.paid.end())
    {
        payment->price += price;
        payment->vat += vat;
    }
    else
    {
        invoice.paid.push_back(Payment{price, vat_rate, vat});
    }
}

struct MailItem {
    std::string registrarEmail; ///< address to deliver email
    boost::uuids::uuid registrarUuid;
    boost::gregorian::date from; ///< start of invoicing period 
    boost::gregorian::date to; ///< end of invoicing period 
    boost::optional<boost::uuids::uuid> file_pdf_uuid;
    boost::optional<boost::uuids::uuid> file_xml_uuid;
    unsigned long long generation; ///< filled if source is invoice generation
    unsigned long long invoice; ///< filled if successful generation or advance invoice
    std::string zoneFqdn;
};

LibHermes::Email::Type get_mail_type_name(bool _generation, bool _invoice) {
    if (!_generation) {
        return LibHermes::Email::Type{std::string{"invoice_deposit"}};
    };
    if (!_invoice) {
        return LibHermes::Email::Type{std::string{"invoice_noaudit"}};
    };
    return LibHermes::Email::Type{std::string{"invoice_audit"}};
}

LibHermes::Email::SubjectTemplate get_template_name_subject(bool _generation, bool _invoice)
{
    if (!_generation)
    {
        return LibHermes::Email::SubjectTemplate{std::string{"invoice-deposit-subject.txt"}};
    }
    if (!_invoice)
    {
        return LibHermes::Email::SubjectTemplate{std::string{"invoice-subject.txt"}};
    }
    return LibHermes::Email::SubjectTemplate{std::string{"invoice-subject.txt"}};
}

LibHermes::Email::BodyTemplate get_template_name_body(bool _generation, bool _invoice)
{
    if (!_generation)
    {
        return LibHermes::Email::BodyTemplate{std::string{"invoice-deposit-body.txt"}};
    }
    if (!_invoice)
    {
        return LibHermes::Email::BodyTemplate{std::string{"invoice-noaudit-body.txt"}};
    }
    return LibHermes::Email::BodyTemplate{std::string{"invoice-audit-body.txt"}};
}


void send_invoices(const MessengerArgs& _messenger_args)
{
LOGGER.debug("DEBUG0 send");
    typedef std::vector<MailItem> MailItems; ///< type for notification list
    MailItems mail_items; ///< list of notifications to send

        // load

        Database::Connection conn = Database::Manager::acquire();

        std::stringstream sql;

        sql << "SELECT r.email, r.uuid, g.fromdate, g.todate, "
        << "i.file_uuid, i.file_xml_uuid, g.id, i.id, z.fqdn "
        << "FROM registrar r, invoice i "
        << "LEFT JOIN invoice_generation g ON (g.invoice_id=i.id) "
        << "LEFT JOIN invoice_mails im ON (im.invoiceid=i.id) "
        << "LEFT JOIN zone z ON (z.id = i.zone_id) "
        << "WHERE i.registrar_id=r.id "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')"
        << "UNION "
        << "SELECT r.email, r.uuid, g.fromdate, g.todate, NULL, NULL, g.id, "
        << "NULL, z.fqdn "
        << "FROM registrar r, invoice_generation g "
        << "LEFT JOIN invoice_mails im ON (im.genid=g.id) "
        << "LEFT JOIN zone z ON (z.id = g.zone_id) "
        << "WHERE g.registrar_id=r.id AND g.invoice_id ISNULL "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')";

        Database::Result res = conn.exec(sql.str());
        for (unsigned i=0; i < res.size(); ++i)
        mail_items.push_back(MailItem{
              res[i][0],
              boost::uuids::string_generator()(static_cast<std::string>(res[i][1])),
              (boost::gregorian::date(res[i][2].isnull()? boost::gregorian::date(not_a_date_time) : from_string(static_cast<std::string>(res[i][2])))),
              (boost::gregorian::date(res[i][3].isnull()? boost::gregorian::date(not_a_date_time) : from_string(static_cast<std::string>(res[i][3])))),
              res[i][4].isnull() ? boost::optional<boost::uuids::uuid>{} : boost::uuids::string_generator()(static_cast<std::string>(res[i][4])),
              res[i][5].isnull() ? boost::optional<boost::uuids::uuid>{} : boost::uuids::string_generator()(static_cast<std::string>(res[i][5])),
              res[i][6],
              res[i][7],
              res[i][8]
          });

        // send

        LibHermes::Connection<LibHermes::Service::EmailMessenger> connection{
                LibHermes::Connection<LibHermes::Service::EmailMessenger>::ConnectionString{
                        _messenger_args.endpoint}};

        for (const auto& mail_item : mail_items)
        {
            try
            {
                LibHermes::Struct email_template_params;
                std::stringstream dateBuffer;
                dateBuffer.imbue(std::locale(dateBuffer.getloc(), new date_facet("%d.%m.%Y")));
                dateBuffer << mail_item.from;
                email_template_params[LibHermes::StructKey{"fromdate"}] = LibHermes::StructValue{dateBuffer.str()};
                dateBuffer.str("");
                dateBuffer << mail_item.to;
                email_template_params[LibHermes::StructKey{"todate"}] = LibHermes::StructValue{dateBuffer.str()};
                email_template_params[LibHermes::StructKey{"zone"}] = LibHermes::StructValue{mail_item.zoneFqdn};

                auto email =
                        LibHermes::Email::make_minimal_email(
                                {{LibHermes::Email::RecipientEmail{mail_item.registrarEmail}, {}}},
                                get_template_name_subject(mail_item.generation, mail_item.invoice),
                                get_template_name_body(mail_item.generation, mail_item.invoice));
                email.type = get_mail_type_name(mail_item.generation, mail_item.invoice);
                email.context = email_template_params;
                email.attachments = {
                        LibHermes::Email::AttachmentUuid{*mail_item.file_pdf_uuid},
                        LibHermes::Email::AttachmentUuid{*mail_item.file_xml_uuid},
                };
                const auto email_uuid =
                        LibHermes::Email::send(
                                connection,
                                email,
                                LibHermes::Email::Archive{_messenger_args.archive},
                                {LibHermes::Reference{
                                        LibHermes::Reference::Type{"registrar"},
                                        LibHermes::Reference::Value{boost::uuids::to_string(mail_item.registrarUuid)}}});

                // store
                //{
LOGGER.debug("DE//BUG0 store");
                //      const auto sql =
                //              std::string{"INSERT INTO invoice_mails (invoiceid, genid, mailid) VALUES ("} +
                //             (mail_item.invoice ? std::to_string(mail_item.invoice) : std::string{"NULL"}) + ", " +
                //             (mail_item.generation ? std::to_string(mail_item.generation) : std::string{"NULL"}) + ", "
                //             "'" + *email_uuid + "'::UUID)";
                //      Database::Connection conn = Database::Manager::acquire();
                //      conn.exec(sql);
                //}

            }
            catch (const LibHermes::Email::SendFailed& e)
            {
                LOGGER.error(
                        std::string(" Error while send mail in Mails class email: ") + mail_item.registrarEmail +
                        " pdf file uuid: " + boost::lexical_cast<std::string>(*mail_item.file_pdf_uuid) +
                        " xml file uuid: " + boost::lexical_cast<std::string>(*mail_item.file_xml_uuid) +
                        " invoice id: " + boost::lexical_cast<std::string>(mail_item.invoice) +
                        " zone fqdn: " + mail_item.zoneFqdn);
                LOGGER.info(boost::str(boost::format("gRPC exception caught while sending email: gRPC error code: %1% error message: %2% grpc_message_json: %3%") %
                            e.error_code() % e.error_message() % e.grpc_message_json()));
                throw std::runtime_error("failed to send email to recipient");
            }
            catch (const std::exception& e)
            {
                LOGGER.info(boost::str(boost::format("std::exception caught while sending email: %1%") % e.what()));
                throw std::runtime_error("failed to send email to recipient");
            }
            catch (...)
            {
                LOGGER.info("exception caught while sending email");
                throw std::runtime_error("failed to send email to recipient");
            }
        }

}

} // namespace

void invoice_export(
        const MessengerArgs& _messenger_args,
        const FilemanArgs& _fileman_args,
        const SecretaryArgs& _secretary_args,
        bool _invoice_dont_send)
{
    const unsigned long long idFilter = 0;
    const unsigned long long registrarFilter = 0;
    const std::string registrarHandleFilter;
    const unsigned long long zoneFilter = 0;
    const unsigned typeFilter = 0;
    const std::string varSymbolFilter;
    const std::string numberFilter;
    const boost::posix_time::time_period crDateFilter = boost::posix_time::time_period(boost::posix_time::ptime(neg_infin), boost::posix_time::ptime(pos_infin));
    const boost::posix_time::time_period taxDateFilter = boost::posix_time::time_period(boost::posix_time::ptime(neg_infin), boost::posix_time::ptime(pos_infin));
    const ArchiveFilter archiveFilter = AF_IGNORE;
    const unsigned long long objectIdFilter = 0;
    const std::string objectNameFilter;
    const std::string advanceNumberFilter;

    LOGGER.debug("invoice_export: invoice_export");
    std::vector<Invoice> invoices;
    try
    {

        Logging::Context ctx("invoice export");
        try
        {
            LOGGER.debug("invoice_export: connection");
            Database::Connection conn = Database::Manager::acquire();

            Database::QueryParams sql_params;
            sql_params.reserve(20);
            std::stringstream sql;

            sql << "SELECT DISTINCT i.id "
            << "INTO TEMPORARY tmp_invoice_filter_result ";
            std::stringstream from;
            from << "FROM invoice i ";
            std::stringstream where;
            where << "WHERE 1=1 ";
            if (idFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(idFilter));
                where << "AND " << "i.id" << "= $" << sql_params.size() << "::BIGINT ";
            }
            if (registrarFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(registrarFilter));
                where << "AND " << "i.registrar_id" << "= $" << sql_params.size() << "::BIGINT ";
            }
            if (zoneFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(zoneFilter));
                where << "AND " << "i.zone_id" << "= $" << sql_params.size() << "::BIGINT ";
            }
            if (typeFilter && typeFilter <= 2)
            {
                from << ", invoice_prefix ip ";
                where << "AND i.invoice_prefix_id=ip.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(typeFilter-1));
                where << "AND " << "ip.typ" << "=$" << sql_params.size() << "::INTEGER ";
            }
            if (!varSymbolFilter.empty() || !registrarHandleFilter.empty())
            {
                from << ", registrar r ";
                where << "AND i.registrar_id=r.id ";
                if (!varSymbolFilter.empty())
                {
                    sql_params.push_back(boost::lexical_cast<std::string>(varSymbolFilter));
                    where << "AND "
                           << "TRIM(r.varsymb)" << " ILIKE TRANSLATE($" << sql_params.size() << "::TEXT,'*?','%_') ";
                }
                if (!registrarHandleFilter.empty())
                {
                    sql_params.push_back(boost::lexical_cast<std::string>(registrarHandleFilter));
                    where << "AND "
                           << "r.handle" << " ILIKE TRANSLATE($" << sql_params.size() << "::TEXT,'*?','%_') ";
                }
            }
            if (!numberFilter.empty())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(numberFilter));
                where << "AND "
                    << "(i.prefix)::TEXT" << " ILIKE TRANSLATE($" << sql_params.size() << "::TEXT,'*?','%_') ";
            }
            if (!crDateFilter.begin().is_special())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(crDateFilter.begin().date())));
                 where << "AND " << "i.crdate" << ">=$"
                   <<  sql_params.size() << "::TIMESTAMP ";
            }
            if (!crDateFilter.end().is_special())
            {
                sql_params.push_back(
                    to_iso_extended_string(crDateFilter.end().date() + boost::gregorian::days(1))
                    );
                where << "AND " << "i.crdate" << " < $"
                        <<  sql_params.size() << "::TIMESTAMP ";
            }
            if ((!taxDateFilter.begin().is_special()))
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                    to_iso_extended_string(taxDateFilter.begin())));
               where << "AND " << "i.taxdate" << ">=$"
                 <<  sql_params.size() << "::TIMESTAMP ";
            }
            if ((!taxDateFilter.end().is_special()))
            {
                sql_params.push_back(
                    to_iso_extended_string(taxDateFilter.end().date() + boost::gregorian::days(1)));
                where << "AND " << "i.taxdate" << " < $"
                    << sql_params.size() << "::TIMESTAMP ";
            }
            switch (archiveFilter)
            {
                case AF_IGNORE: break;
                case AF_SET: where << "AND NOT(i.file ISNULL) "; break;
                case AF_UNSET: where << "AND i.file ISNULL "; break;
                default: break;
            }
            if (objectIdFilter)
            {
                from << ", invoice_operation io ";
                where << "AND i.id=io.ac_invoice_id ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectIdFilter));
                where << "AND " << "io.object_id" << "=$" << sql_params.size() << "::BIGINT ";
            }
            if (!objectNameFilter.empty())
            {
                from << ", invoice_operation ioh, object_registry obr ";
                where << "AND i.id=ioh.ac_invoice_id AND obr.id=ioh.object_id ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectNameFilter));
                where << "AND "
                       << "obr.name" << " ILIKE TRANSLATE($" << sql_params.size() << "::TEXT,'*?','%_') ";
            }

            if (!advanceNumberFilter.empty())
            {
                from << ", invoice_operation io2 "
                << ", invoice_operation_charge_map iocm "
                << ", invoice advi ";
                where << "AND i.id=io2.ac_invoice_id "
                << "AND iocm.invoice_operation_id=io2.id AND iocm.invoice_id=advi.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(advanceNumberFilter));
                where << "AND "
                       << "(advi.prefix)::TEXT" << " ILIKE TRANSLATE($" << sql_params.size() << "::TEXT,'*?','%_') ";
            }
            sql << from.rdbuf() << where.rdbuf();

            LOGGER.debug("invoice_export: select tmp");
            Database::Result res1 = conn.exec_params(sql.str(), sql_params);
            Database::Result res2 = conn.exec("ANALYZE tmp_invoice_filter_result");

            LOGGER.debug("invoice_export: select invoice");
            Database::Result res3 = conn.exec(
                    // clang-format off
                    "SELECT i.id, "
                           "i.zone_id, "
                           "i.crdate::TIMESTAMPTZ AT TIME ZONE 'Europe/Prague', "
                           "i.taxdate, "
                           "ig.fromdate, "
                           "ig.todate, "
                           "ip.typ, "
                           "i.prefix, "
                           "i.registrar_id, "
                           "i.balance, "
                           "i.operations_price, "
                           "i.vat, "
                           "i.total, "
                           "i.totalvat, "
                           "i.file, "    // i.file_uuid FIXME
                           "i.fileXML, " // i.file_xml_uuid
                           "r.organization, "
                           "r.street1, "
                           "r.city, "
                           "r.postalcode, "
                           "TRIM(r.ico), "
                           "TRIM(r.dic), "
                           "TRIM(r.varsymb), "
                           "r.handle, "
                           "r.vat, "
                           "r.id, "
                           "z.fqdn, "
                           "r.country "
                      "FROM tmp_invoice_filter_result it "
                      "JOIN invoice i "
                        "ON it.id = i.id "
                      "JOIN zone z "
                        "ON i.zone_id = z.id "
                      "JOIN registrar r "
                        "ON i.registrar_id = r.id "
                      "JOIN invoice_prefix ip "
                        "ON ip.id = i.invoice_prefix_id "
                      "LEFT JOIN invoice_generation ig "
                        "ON i.id = ig.invoice_id "
                     "ORDER BY it.id"
                    // clang-format on
            );

            for (unsigned i = 0; i < res3.size(); ++i)
            {
                LOGGER.debug(
                    boost::format(
                    "res3 i.id %1% i.zone_id %2% i.typ %3% i.prefix %4%"
                    " i.registrar_id %5% i.balance %6% i.operations_price %7% i.vat %8%"
                    " i.total %9% i.totalvat %10%")
                    % std::string(res3[i][0])
                    % std::string(res3[i][1])
                    % (int(res3[i][6]) == 0 ? IT_DEPOSIT : IT_ACCOUNT)
                    % std::string(res3[i][7])
                    % std::string(res3[i][8])
                    % std::string(res3[i][9])
                    % std::string(res3[i][10])
                    % std::string(res3[i][11])
                    % std::string(res3[i][12])
                    % std::string(res3[i][13])
                );

                Database::ID id = res3[i][0];
                Database::ID zone = res3[i][1];
                std::string fqdn = res3[i][26];
                Database::DateTime create_time = (boost::posix_time::ptime(res3[i][2].isnull() ? boost::posix_time::ptime(not_a_date_time) : time_from_string(res3[i][2])));
                Database::Date tax_date = (boost::gregorian::date(res3[i][3].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(res3[i][3])));
                Database::Date from_date = (boost::gregorian::date(res3[i][4].isnull() ? boost::gregorian::date(neg_infin) : from_string(res3[i][4])));
                Database::Date to_date = (boost::gregorian::date(res3[i][5].isnull() ? boost::gregorian::date(pos_infin) : from_string(res3[i][5])));
                InvoiceType type = (int(res3[i][6]) == 0 ? IT_DEPOSIT : IT_ACCOUNT);
                unsigned long long number = res3[i][7];
                Database::ID registrar_id = res3[i][8];
                Money credit = std::string(res3[i][9]);
                Money price = std::string(res3[i][10]);
                std::string vat_rate = res3[i][11];
                Money total = std::string(res3[i][12]);
                Money total_vat = std::string(res3[i][13]);
                //Database::ID filePDF = res3[i][14]; // file_uuid TODO trigger
                //Database::ID fileXML = res3[i][15]; // file_xml_uuid
                std::string c_organization = res3[i][16];
                std::string c_street1 = res3[i][17];
                std::string c_city = res3[i][18];
                std::string c_postal_code = res3[i][19];
                std::string c_ico = res3[i][20];
                std::string c_dic = res3[i][21];
                std::string c_var_symb = res3[i][22];
                std::string c_handle = res3[i][23];
                bool c_vat = res3[i][24];
                unsigned long long c_id = res3[i][25];
                std::string c_country = res3[i][27];

                boost::gregorian::date_period account_period(from_date, to_date);
                Subject client{
                        c_id,
                        c_handle,
                        c_organization,
                        "",
                        c_street1,
                        c_city,
                        c_postal_code,
                        c_country,
                        c_ico,
                        c_dic,
                        "",
                        "",
                        "",
                        "",
                        "",
                        "",
                        c_vat};

                //////Database::SelectQuery source_query;
                //////// clang-format off
                //////  source_query.select() << "tmp.id, "
                //////                           "icm.credit, "
                //////                           "sri.vat, "
                //////                           "sri.prefix, "
                //////                           "icm.balance, "
                //////                           "sri.id, "
                //////                           "sri.total, "
                //////                           "sri.totalvat, "
                //////                           "sri.crdate";
                //////    source_query.from() << "tmp_invoice_filter_result tmp "
                //////                      "JOIN invoice_credit_payment_map icm "
                //////                        "ON tmp.id = icm.ac_invoice_id "
                //////                      "JOIN invoice sri "
                //////                        "ON icm.ad_invoice_id = sri.id ";
                //////source_query.order_by() << "tmp.id";
                //////// clang-format on

                //////Database::Result result_sources = conn.exec(source_query);
                //////LOGGER.debug("invoice_export: found " + result_sources.size() + " sources");
                //////for (Database::Result::Iterator it = result_sources.begin(); it != result_sources.end(); ++it)
                //////{
                //////    Database::Row::Iterator col = (*it).begin();
                //////    Database::ID invoice_id = *col;

                //////    const auto invoice = find_if(invoices.begin(), invoices.end(), [&invoice_id](const Invoice& i) { return i.id == invoice_id; });
                //////    if (invoice != invoices.end())
                //////    {
                //////        invoice_add_source(*invoice, ++col);
                //////    }
                //////}

                invoices.push_back(
                        Invoice{id,
                                zone,
                                fqdn,
                                create_time,
                                tax_date,
                                account_period,
                                type,
                                number,
                                registrar_id,
                                credit,
                                price,
                                Decimal(vat_rate),
                                total,
                                total_vat,
                                boost::none,
                                boost::none,
                                c_var_symb,
                                client,
                                {}, {}, {}});

            }//for res3

            /* append list of actions to all selected invoices
             * it handle situation when action come from source advance invoices
             * with different vat rates by grouping
             * this is ignored on partial load
             */
            {
                Database::SelectQuery action_query;
                action_query.select() << "tmp.id, SUM(icm.price), i.vat, o.name, "
                                      << "io.crdate::TIMESTAMPtz AT TIME ZONE 'Europe/Prague', "
                                      << "io.date_from, io.date_to, io.operation_id, io.quantity, "
                                      << "CASE "
                                      << "  WHEN io.quantity = 0 THEN 0 "
                                      << "  ELSE SUM(icm.price) / io.quantity END, "
                                      << "o.id";
                action_query.from() << "tmp_invoice_filter_result tmp "
                                    << "JOIN invoice_operation io ON (tmp.id = io.ac_invoice_id) "
                                    << "JOIN invoice_operation_charge_map icm ON (io.id = icm.invoice_operation_id) "
                                    << "JOIN invoice i ON (icm.invoice_id = i.id) "
                                    << "LEFT JOIN object_registry o ON (io.object_id = o.id) ";
                action_query.group_by() << "tmp.id, o.name, io.crdate, io.date_from, io.date_to, "
                                        << "io.operation_id, io.quantity, o.id, i.vat";
                action_query.order_by() << "tmp.id";

                Database::Result result_actions = conn.exec(action_query);
LOGGER.debug(std::string{"invoice_export: found "} + std::to_string(result_actions.size()) + " actions");
                for (Database::Result::Iterator it = result_actions.begin(); it != result_actions.end(); ++it)
                {
                    Database::Row::Iterator col = (*it).begin();
                    Database::ID invoice_id = *col;

                    const auto invoice = find_if(invoices.begin(), invoices.end(), [&invoice_id](const Invoice& i){ return i.id == invoice_id; });
                    if (invoice != invoices.end())
                    {
                        invoice_add_action(*invoice, ++col);
                    }
                }
            }

            //append list of sources to all selected invoices
            {
                Database::SelectQuery source_query;
                source_query.select() << "tmp.id, icm.credit, sri.vat, sri.prefix, "
                                      << "icm.balance, sri.id, sri.total, "
                                      << "sri.totalvat, sri.crdate";
                source_query.from() << "tmp_invoice_filter_result tmp "
                                    << "JOIN invoice_credit_payment_map icm ON (tmp.id = icm.ac_invoice_id) "
                                    << "JOIN invoice sri ON (icm.ad_invoice_id = sri.id) ";
                source_query.order_by() << "tmp.id";

                Database::Result result_sources = conn.exec(source_query);
LOGGER.debug(std::string{"invoice_export: found "} + std::to_string(result_sources.size()) + " sources");
                for (Database::Result::Iterator it = result_sources.begin(); it != result_sources.end(); ++it)
                {
                    Database::Row::Iterator col = (*it).begin();
                    Database::ID invoice_id = *col;

                    const auto invoice = find_if(invoices.begin(), invoices.end(), [&invoice_id](const Invoice& i) { return i.id == invoice_id; });
                    if (invoice != invoices.end())
                    {
                        invoice_add_source(*invoice, ++col);
                    }
                }
            }
            // delete temporary table
            conn.exec("DROP TABLE tmp_invoice_filter_result ");
        }
        catch (Database::Exception& ex)
        {
            std::string message = ex.what();
            if (message.find(Database::Connection::getTimeoutString()) != std::string::npos)
            {
                LOGGER.error("timeout");
            }
            else
            {
                LOGGER.error(boost::format("%1%") % ex.what());
            }
            throw;
        }
        catch (std::exception& ex)
        {
            LOGGER.error(boost::format("%1%") % ex.what());
            throw;
        }


        for (const auto& invoice : invoices)
        {
            export_and_archive_invoice(_secretary_args, _fileman_args, invoice);
        }

LOGGER.debug("DEBUGX dontsend:"+_invoice_dont_send?"dont":"DO");
        if (!_invoice_dont_send)
        {
            send_invoices(_messenger_args);
        }
    }
    catch (const std::exception& ex)
    {
        LOGGER.error(std::string("Exception in archiveInvoices: ") + ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("Exception in archiveInvoices.");
        throw;
    }
}

} // namespace Admin
