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

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <sstream>

namespace Admin {

namespace {

enum struct InvoiceType {
  deposit,
  account
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
    std::string country_code;
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

// this relies on PaymentActionType (operation_id - 1) in invoice_add_action
enum PaymentActionType
{
    create_domain = 0,
    renew_domain = 1,
    requests_over_limit = 2,
    administrative_fee = 3,
    fee = 4,
    monthly_fee = 5
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
    boost::optional<boost::uuids::uuid> file_pdf_uuid;
    boost::optional<boost::uuids::uuid> file_xml_uuid;
    std::string var_symbol;
    Subject client;
    std::vector<PaymentSource> sources;
    std::vector<PaymentAction> actions;
    std::vector<Payment> paid; ///< list of paid vat rates
};

std::string format_money(Money _value)
{
    return _value.get_string(".2f");
}

std::string to_string(PaymentActionType _payment_action)
{
    switch (_payment_action)
    {
        case PaymentActionType::create_domain: return "RREG";
        case PaymentActionType::renew_domain: return "RUDR";
        case PaymentActionType::requests_over_limit: return "REPP";
        case PaymentActionType::administrative_fee: return "RPOA";
        case PaymentActionType::fee: return "RPOP";
        case PaymentActionType::monthly_fee: return "RPOP";
    }
    return "RUNK";
}

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
 "GROUP BY 1,2";
// clang-format on

LibTypist::Struct make_subject_context(const Subject& _subject)
{
    return LibTypist::Struct{
            {LibTypist::StructKey{"id"}, LibTypist::StructValue{std::to_string(_subject.id)}},
            {LibTypist::StructKey{"organization"}, LibTypist::StructValue{_subject.fullname}},
            {LibTypist::StructKey{"address"},
                    LibTypist::StructValue{LibTypist::Struct{
                            {LibTypist::StructKey{"street"}, LibTypist::StructValue{_subject.street}},
                            {LibTypist::StructKey{"postal_code"}, LibTypist::StructValue{_subject.zip}},
                            {LibTypist::StructKey{"city"}, LibTypist::StructValue{_subject.city}},
                            {LibTypist::StructKey{"country_code"}, LibTypist::StructValue{_subject.country_code}}}}},
            {LibTypist::StructKey{"vat_identification_number"}, LibTypist::StructValue{_subject.vat_number}},
            {LibTypist::StructKey{"company_registration_number"}, LibTypist::StructValue{_subject.ico}}};
}

LibTypist::Struct make_invoice_context(const Invoice& _invoice)
{
    LOGGER.debug("make_invoice_context");
    if (_invoice.crtime.date().is_special())
    {
        throw std::runtime_error("make_invoice_context: invoice crtime date is special");
    }
    if (_invoice.tax_date.is_special())
    {
        throw std::runtime_error("make_invoice_context: invoice taxdate is special");
    }
    if (_invoice.type == InvoiceType::account)
    {
        if (_invoice.account_period.begin().is_special())
        {
            throw std::runtime_error("make_invoice_context: invoice period_from is special");
        }
        if (_invoice.account_period.end().is_special())
        {
            throw std::runtime_error("make_invoice_context: invoice period_to is special");
        }
    }

    LibTypist::Struct context;
    const auto buyer_context = make_subject_context(_invoice.client);

    auto invoice_context =
            LibTypist::Struct{
                    {LibTypist::StructKey{"number"}, LibTypist::StructValue{std::to_string(_invoice.number)}},
                    {LibTypist::StructKey{"variable_symbol"}, LibTypist::StructValue{_invoice.var_symbol}},
                    {LibTypist::StructKey{"date"}, LibTypist::StructValue{to_iso_extended_string(_invoice.crtime.date())}},
                    {LibTypist::StructKey{"tax_date"}, LibTypist::StructValue{to_iso_extended_string(_invoice.tax_date)}}};

    if (_invoice.type == InvoiceType::account)
    {
        invoice_context[LibTypist::StructKey{"period_from"}] = LibTypist::StructValue{to_iso_extended_string(_invoice.account_period.begin())};
        invoice_context[LibTypist::StructKey{"period_to"}] = LibTypist::StructValue{to_iso_extended_string(_invoice.account_period.end())};
    }

    struct MoneyRecord
    {
        Money price_without_vat;
        Money price_with_vat;
        Money vat;
    };

    if (_invoice.type == InvoiceType::account)
    {
        LOGGER.debug("make_invoice_context: account invoice");
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
            const Database::Result db_result =
                    conn.exec_params(
                            invoice_query,
                            Database::query_param_list(_invoice.id));

            if (db_result.size() < 1)
            {
                throw std::runtime_error("make_invoice_context account query failed");
            }

            const auto row = db_result[0];
            account_invoice_record.vat_rate = Decimal(static_cast<std::string>(row[0]));
            account_invoice_record.self.price_without_vat = Money(static_cast<std::string>(row[1]));
            account_invoice_record.self.vat = Money(static_cast<std::string>(row[2]));
            account_invoice_record.self.price_with_vat = account_invoice_record.self.price_without_vat + account_invoice_record.self.vat;
            account_invoice_record.operations_price = Decimal(static_cast<std::string>(row[3]));
        }

        const Database::Result db_result =
                conn.exec_params(
                        account_invoice_with_annual_partitioning_query,
                        Database::query_param_list(_invoice.id));

        if (db_result.size() > 0)
        {

            for (const auto& row : db_result)
            {
                const auto year = static_cast<unsigned>(row[0]);
                const auto vat_rate = Decimal(static_cast<std::string>(row[1]));
                account_invoice_record.records[vat_rate][year].price_without_vat = Money(static_cast<std::string>(row[2]));
                account_invoice_record.records[vat_rate][year].price_with_vat = Money(static_cast<std::string>(row[3]));
                account_invoice_record.records[vat_rate][year].vat = Money(static_cast<std::string>(row[4]));
            }

            std::vector<LibTypist::StructValue> vat_rates;

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
                    vat_rates_entry_context[LibTypist::StructKey{"total_base"}] = LibTypist::StructValue{format_money(account_invoice_record.self.price_without_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_vat"}] = LibTypist::StructValue{format_money(account_invoice_record.self.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"price"}] = LibTypist::StructValue{format_money(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"price_vat"}] = LibTypist::StructValue{format_money(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid"}] = LibTypist::StructValue{format_money(all_years_money_record.price_with_vat - account_invoice_record.self.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{format_money(all_years_money_record.vat - account_invoice_record.self.vat)};
                }
                else
                {
                    vat_rates_entry_context[LibTypist::StructKey{"vat_rate"}] = LibTypist::StructValue{boost::lexical_cast<std::string>(vat_rate)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_base"}] = LibTypist::StructValue{format_money(all_years_money_record.price_without_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"total_vat"}] = LibTypist::StructValue{format_money(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"price"}] = LibTypist::StructValue{format_money(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"price_vat"}] = LibTypist::StructValue{format_money(all_years_money_record.vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid"}] = LibTypist::StructValue{format_money(all_years_money_record.price_with_vat)};
                    vat_rates_entry_context[LibTypist::StructKey{"paid_vat"}] = LibTypist::StructValue{format_money(all_years_money_record.vat)};
                }

                std::vector<LibTypist::StructValue> deferred_income_context;
                for (const auto& year_record : account_invoice_record.records[vat_rate])
                {
                    const auto year = year_record.first;
                    const auto year_money_record = year_record.second;

                    const LibTypist::Struct year_context{
                            {LibTypist::StructKey{"year"}, LibTypist::StructValue{std::to_string(year)}},
                            {LibTypist::StructKey{"base"}, LibTypist::StructValue{format_money(year_money_record.price_without_vat)}},
                            {LibTypist::StructKey{"vat"}, LibTypist::StructValue{format_money(year_money_record.vat)}},
                            {LibTypist::StructKey{"total"}, LibTypist::StructValue{format_money(year_money_record.price_with_vat)}}};
                    deferred_income_context.push_back(LibTypist::StructValue{year_context});
                }
                vat_rates_entry_context[LibTypist::StructKey{"deferred_income"}] = LibTypist::StructValue{deferred_income_context};
                vat_rates.push_back(LibTypist::StructValue{vat_rates_entry_context});
            }
            if (!vat_rates.empty())
            {
                context[LibTypist::StructKey{"vat_rates"}] = LibTypist::StructValue{vat_rates};
            }
        }

        invoice_context[LibTypist::StructKey{"price_base"}] = LibTypist::StructValue{format_money(account_invoice_record.operations_price)};
        invoice_context[LibTypist::StructKey{"paid_base"}] = LibTypist::StructValue{format_money(account_invoice_record.self.price_without_vat - account_invoice_record.operations_price)};
        invoice_context[LibTypist::StructKey{"total"}] = LibTypist::StructValue{format_money(account_invoice_record.self.price_with_vat)};
    }
    else // deposit
    {
        LOGGER.debug("make_invoice_context: deposit (advance) invoice");
        struct AdvanceInvoiceRecord
        {
            MoneyRecord self;
            Money operations_price;
        };

        Database::Connection conn = Database::Manager::acquire();
        Database::Result db_result =
                conn.exec_params(
                        invoice_query,
                        Database::query_param_list(_invoice.id));

        if (db_result.size() != 1)
        {
            throw std::runtime_error("make_invoice_context: deposit query failed");
        }

        AdvanceInvoiceRecord advance_invoice_record;

        const auto row = db_result[0];
        const auto vat_rate = Decimal(static_cast<std::string>(row[0]));
        advance_invoice_record.self.price_without_vat = Money(static_cast<std::string>(row[1]));
        advance_invoice_record.self.vat = Money(static_cast<std::string>(row[2]));
        advance_invoice_record.self.price_with_vat = advance_invoice_record.self.price_without_vat + advance_invoice_record.self.vat;
        advance_invoice_record.operations_price = Decimal(static_cast<std::string>(row[3]));

        LibTypist::Struct vat_rates_entry_context{
                {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(vat_rate)}},
                {LibTypist::StructKey{"total_base"}, LibTypist::StructValue{format_money(advance_invoice_record.self.price_without_vat)}},
                {LibTypist::StructKey{"total_vat"}, LibTypist::StructValue{format_money(advance_invoice_record.self.vat)}},
                {LibTypist::StructKey{"total"}, LibTypist::StructValue{format_money(advance_invoice_record.self.price_with_vat)}},
                {LibTypist::StructKey{"price"}, LibTypist::StructValue{format_money(advance_invoice_record.self.price_with_vat)}},
                {LibTypist::StructKey{"price_vat"}, LibTypist::StructValue{format_money(advance_invoice_record.self.vat)}},
                {LibTypist::StructKey{"paid"}, LibTypist::StructValue{format_money(advance_invoice_record.self.price_with_vat)}},
                {LibTypist::StructKey{"paid_vat"}, LibTypist::StructValue{format_money(advance_invoice_record.self.vat)}}};

        context[LibTypist::StructKey{"vat_rates"}] =
                LibTypist::StructValue{std::vector<LibTypist::StructValue>{{LibTypist::StructValue{vat_rates_entry_context}}}};

        invoice_context[LibTypist::StructKey{"price_base"}] = LibTypist::StructValue{format_money(Money{""})}; // legacy reasons, to keep empty total
        invoice_context[LibTypist::StructKey{"paid_base"}] = LibTypist::StructValue{format_money(Money{"0"})};
        invoice_context[LibTypist::StructKey{"total"}] = LibTypist::StructValue{format_money(Money{"0"})};
    }

    std::vector<LibTypist::StructValue> advance_invoices;
    if (_invoice.sources.size())
    {
        for (const auto payment_source : _invoice.sources)
        {
            advance_invoices.push_back(LibTypist::StructValue{LibTypist::Struct{
                    {LibTypist::StructKey{"number"}, LibTypist::StructValue{std::to_string(payment_source.number)}},
                    {LibTypist::StructKey{"consumed_base"}, LibTypist::StructValue{format_money(payment_source.price)}},
                    {LibTypist::StructKey{"balance"}, LibTypist::StructValue{format_money(payment_source.credit)}},
                    {LibTypist::StructKey{"consumed_vat"}, LibTypist::StructValue{format_money(payment_source.vat)}},
                    {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_source.vat_rate)}},
                    {LibTypist::StructKey{"consumed"}, LibTypist::StructValue{format_money(payment_source.price + payment_source.vat)}},
                    {LibTypist::StructKey{"total_base"}, LibTypist::StructValue{format_money(payment_source.total_price)}},
                    {LibTypist::StructKey{"total_vat"}, LibTypist::StructValue{format_money(payment_source.total_vat)}},
                    {LibTypist::StructKey{"total"}, LibTypist::StructValue{format_money(payment_source.total_price + payment_source.total_vat)}},
                    {LibTypist::StructKey{"datetime"}, LibTypist::StructValue{to_iso_extended_string(payment_source.crtime)}}
            }});
        }
        if (!advance_invoices.empty())
        {
            context[LibTypist::StructKey{"advance_invoices"}] = LibTypist::StructValue{advance_invoices};
        }
    }
    std::vector<LibTypist::StructValue> items;
    if (_invoice.actions.size())
    {
        for (const auto payment_action : _invoice.actions)
        {
            LibTypist::Struct item_context{
                    {LibTypist::StructKey{"subject"}, LibTypist::StructValue{payment_action.object_name}},
                    {LibTypist::StructKey{"code"}, LibTypist::StructValue{to_string(payment_action.action)}},
                    {LibTypist::StructKey{"datetime"}, LibTypist::StructValue{to_iso_extended_string(payment_action.action_time)}},
                    {LibTypist::StructKey{"count"}, LibTypist::StructValue{std::to_string(payment_action.units_count)}},
                    {LibTypist::StructKey{"price"}, LibTypist::StructValue{format_money(payment_action.price_per_unit)}},
                    {LibTypist::StructKey{"total"}, LibTypist::StructValue{format_money(payment_action.price)}},
                    {LibTypist::StructKey{"vat_rate"}, LibTypist::StructValue{boost::lexical_cast<std::string>(payment_action.vat_rate)}}};
            if (!payment_action.exdate.is_special())
            {
                item_context[LibTypist::StructKey{"expiration_date"}] = LibTypist::StructValue{to_iso_extended_string(payment_action.exdate)};
            };
            items.push_back(LibTypist::StructValue{item_context});
        }
        if (!items.empty())
        {
           context[LibTypist::StructKey{"items"}] = LibTypist::StructValue{items};
        }
    }

    context[LibTypist::StructKey{"buyer"}] = LibTypist::StructValue{buyer_context};
    context[LibTypist::StructKey{"invoice"}] = LibTypist::StructValue{invoice_context};

    return LibTypist::Struct{{LibTypist::StructKey{"context"}, LibTypist::StructValue{context}}};
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

        sql_params.push_back(std::to_string(_invoice_id));
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
        const Invoice& _invoice,
        bool _debug_context)
{
    LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: invoice id %1% number %2% type %3%") % _invoice.id % _invoice.number % (_invoice.type == InvoiceType::account ? "account" : "advance")));
    const auto invoice_context = make_invoice_context(_invoice);

    LibTypist::Connection secretary_connection{
            LibTypist::Connection::ConnectionString{_secretary_args.endpoint}};

    const auto template_name_html =
            LibTypist::TemplateName{std::string{
                    (_invoice.type == InvoiceType::deposit ? "advance-invoice" : "invoice")} +
                    (_invoice.client.country_code == "CZ" ? "-cs" : "-en") + ".html"};

    if (_debug_context)
    {
        LOGGER.debug("invoice_export: debug_context");
        std::cout << LibTypist::to_json_string(invoice_context) << std::endl;
        return;
    }

    try
    {
        std::stringstream file_pdf_rawdata;
        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: render pdf using template %1%") % *template_name_html));
        LibTypist::render(
                secretary_connection,
                template_name_html,
                LibTypist::ContentType{"application/pdf"},
                invoice_context,
                file_pdf_rawdata);

        const auto template_name_xml =
                LibTypist::TemplateName{_invoice.type == InvoiceType::deposit ? "advance-invoice.xml" : "invoice.xml"};

        std::stringstream file_xml_rawdata;
        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: render xml using template %1%") % *template_name_xml));
        LibTypist::render(
                secretary_connection,
                template_name_xml,
                LibTypist::ContentType{"text/html"},
                invoice_context,
                file_xml_rawdata);

        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: connecting to fileman at %1%") % _fileman_args.endpoint));
        LibFiled::Connection<LibFiled::Service::File> fileman_connection{
                LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
                        _fileman_args.endpoint}};

        LOGGER.debug("export_and_archive_invoice: archiving pdf file to fileman");
        const auto file_pdf_uuid =
                LibFiled::File::create(
                        fileman_connection,
                        LibFiled::File::FileName{std::to_string(_invoice.number) + ".pdf"},
                        file_pdf_rawdata,
                        LibFiled::File::FileMimeType{"application/pdf"});

        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: pdf file uuid: %1%") % boost::uuids::to_string(*file_pdf_uuid)));

        LOGGER.debug("export_and_archive_invoice: archiving xml file to fileman");
        const auto file_xml_uuid =
                LibFiled::File::create(
                        fileman_connection,
                        LibFiled::File::FileName{std::to_string(_invoice.number) + ".xml"},
                        file_xml_rawdata,
                        LibFiled::File::FileMimeType{"application/xml"});

        LOGGER.debug(boost::str(boost::format("export_and_archive_invoice: xml file uuid: %1%") % boost::uuids::to_string(*file_xml_uuid)));

        LOGGER.debug("export_and_archive_invoice: update invoice table - add files");
        invoice_save_file_uuid(_invoice.id, *file_pdf_uuid, *file_xml_uuid);
    }
    catch (const LibTypist::HttpResponseResult& e)
    {
        if (e.result() == 502)
        {
            LOGGER.debug("export_and_archive_invoice: got network exception - Bad Gateway. Skipping this invoice for now.");
            return;
        }
        throw;
    }
}

enum struct ArchiveFilter
{
    ignore,
    set,
    unset
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
        Database::Result db_result = conn.exec("SELECT vat, koef, valid_to FROM price_vat");
        for (const auto& row : db_result)
        {
            vatList.push_back(Vat{
                    Decimal(std::string(row[0])),
                    Decimal(std::string(row[1])),
                    boost::gregorian::date(row[2].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(row[2]))});
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
    }
    else {
        const Vat *v = get_vat(vat_rate);
        Decimal coef = v ? v->koef : Decimal("0");
        vat = (price_without_vat * coef / (Decimal("1") - coef)).round_half_up(2);
    }
    return vat;
}

void invoice_add_action(Invoice& invoice, const Database::Row& _col) {
    std::string _price = std::string(_col[1]);
    std::string _vat_rate = std::string(_col[2]);
    std::string object_name = _col[3];
    Database::DateTime action_time = _col[4];
    Database::Date fromdate = _col[5];
    Database::Date exdate = _col[6];
    int operation_id = _col[7];
    PaymentActionType type = PaymentActionType(operation_id - 1);

    unsigned units = _col[8];
    std::string _price_per_unit = std::string(_col[9]);
    Database::ID id = _col[10];

    Money price(_price);
    Decimal vat_rate(_vat_rate);
    Money price_per_unit(_price_per_unit);

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

void invoice_add_source(Invoice& invoice, const Database::Row& _col)
{
    std::string _price = std::string(_col[1]);
    std::string _vat_rate = std::string(_col[2]);
    unsigned long long number = _col[3];
    std::string _credit = std::string(_col[4]);
    Database::ID id = _col[5];
    std::string _total_price = std::string(_col[6]);
    std::string _total_vat = std::string(_col[7]);
    Database::DateTime crtime = _col[8];

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
    std::string registrar_email; ///< address to deliver email
    boost::uuids::uuid registrarUuid;
    boost::gregorian::date from; ///< start of invoicing period 
    boost::gregorian::date to; ///< end of invoicing period 
    boost::optional<boost::uuids::uuid> file_pdf_uuid;
    boost::optional<boost::uuids::uuid> file_xml_uuid;
    unsigned long long generation; ///< filled if source is invoice generation
    unsigned long long invoice; ///< filled if successful generation or advance invoice
    std::string zone_fqdn;
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
    LOGGER.debug("send_invoices");
    typedef std::vector<MailItem> MailItems; ///< type for notification list
    MailItems mail_items; ///< list of notifications to send

    // load

    Database::Connection conn = Database::Manager::acquire();

    std::stringstream sql;

    sql << "SELECT r.email, "
                  "r.uuid, "
                  "g.fromdate, "
                  "g.todate, "
                  "i.file_uuid, "
                  "i.file_xml_uuid, "
                  "g.id, "
                  "i.id, "
                  "z.fqdn "
             "FROM registrar r, "
                  "invoice i "
             "LEFT JOIN invoice_generation g "
               "ON g.invoice_id = i.id "
             "LEFT JOIN invoice_mails im "
               "ON im.invoiceid = i.id "
             "LEFT JOIN zone z "
               "ON z.id = i.zone_id "
            "WHERE i.registrar_id = r.id "
              "AND im.mailid IS NULL "
              "AND NOT(r.email IS NULL OR TRIM(r.email) = '') "

            "UNION "

           "SELECT r.email, "
                  "r.uuid, "
                  "g.fromdate, "
                  "g.todate, "
                  "NULL, "
                  "NULL, "
                  "g.id, "
                  "NULL, "
                  "z.fqdn "
             "FROM registrar r, "
                  "invoice_generation g "
             "LEFT JOIN invoice_mails im "
               "ON im.genid = g.id "
             "LEFT JOIN zone z "
               "ON z.id = g.zone_id "
             "WHERE g.registrar_id = r.id "
               "AND g.invoice_id IS NULL "
               "AND im.mailid IS NULL "
               "AND NOT(r.email IS NULL OR TRIM(r.email) = '')";

    Database::Result db_result = conn.exec(sql.str());
    for (const auto& row : db_result)
    mail_items.push_back(MailItem{
          row[0],
          boost::uuids::string_generator()(static_cast<std::string>(row[1])),
          (boost::gregorian::date(row[2].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(static_cast<std::string>(row[2])))),
          (boost::gregorian::date(row[3].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(static_cast<std::string>(row[3])))),
          row[4].isnull() ? boost::none : boost::optional<boost::uuids::uuid>{boost::uuids::string_generator()(static_cast<std::string>(row[4]))},
          row[5].isnull() ? boost::none : boost::optional<boost::uuids::uuid>{boost::uuids::string_generator()(static_cast<std::string>(row[5]))},
          row[6].isnull() ? 0 : boost::lexical_cast<unsigned long long>(static_cast<std::string>(row[6])),
          row[7].isnull() ? 0 : boost::lexical_cast<unsigned long long>(static_cast<std::string>(row[7])),
          row[8]
      });

    LOGGER.debug(boost::str(boost::format("send_invoices: found %1% items to send") % mail_items.size()));

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
            email_template_params[LibHermes::StructKey{"zone"}] = LibHermes::StructValue{mail_item.zone_fqdn};

            auto email =
                    LibHermes::Email::make_minimal_email(
                            {{LibHermes::Email::RecipientEmail{mail_item.registrar_email}, {}}},
                            get_template_name_subject(mail_item.generation, mail_item.invoice),
                            get_template_name_body(mail_item.generation, mail_item.invoice));
            email.type = get_mail_type_name(mail_item.generation, mail_item.invoice);
            email.context = email_template_params;
            if (mail_item.file_pdf_uuid != boost::none && mail_item.file_xml_uuid != boost::none)
            {
                email.attachments = {
                        LibHermes::Email::AttachmentUuid{*mail_item.file_pdf_uuid},
                        LibHermes::Email::AttachmentUuid{*mail_item.file_xml_uuid}};
            }

            LOGGER.debug("send_invoices: sending email");

            const auto email_uid =
                    LibHermes::Email::send(
                            connection,
                            email,
                            LibHermes::Email::Archive{_messenger_args.archive},
                            {LibHermes::Reference{
                                    LibHermes::Reference::Type{"registrar"},
                                    LibHermes::Reference::Value{boost::uuids::to_string(mail_item.registrarUuid)}}});

            LOGGER.debug(boost::str(boost::format("send_invoices: email uid: %1%") % *email_uid));

            {
                LOGGER.debug("send_invoices: store");
                const auto sql =
                        std::string{"INSERT INTO invoice_mails (invoiceid, genid, mailid) VALUES ("} +
                        (mail_item.invoice ? std::to_string(mail_item.invoice) : std::string{"NULL"}) + ", " +
                        (mail_item.generation ? std::to_string(mail_item.generation) : std::string{"NULL"}) + ", "
                        "0)";
                        //"'" + *email_uuid + "'::TEXT)";
                Database::Connection conn = Database::Manager::acquire();
                conn.exec(sql);
            }
 
        }
        catch (const LibHermes::Email::SendFailed& e)
        {
            LOGGER.error(
                    std::string(" Error while send mail in Mails class email: ") + mail_item.registrar_email +
                    " invoice id: " + std::to_string(mail_item.invoice));
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

enum struct ExportedStatePolicy
{
    ignore,
    unexported,
    exported
};

std::vector<Invoice> get_invoices(
        unsigned long long _invoice_id,
        boost::optional<boost::gregorian::date> _taxdate_from,
        boost::optional<boost::gregorian::date> _taxdate_to,
        int _limit,
        ExportedStatePolicy _export_state_policy)
{
    std::vector<Invoice> invoices;

    LOGGER.debug("invoice_export: connection");
    Database::Connection conn = Database::Manager::acquire();

    Database::QueryParams sql_params;
    sql_params.reserve(20);
    std::stringstream sql;
    std::stringstream from;
    std::stringstream where;

    // clang format-off
     sql << "SELECT DISTINCT i.id "
              "INTO TEMPORARY tmp_invoice_filter_result ";
      from << "FROM invoice i ";
    where << "WHERE 1=1 ";
    // clang format-on
    if (_invoice_id)
    {
        sql_params.push_back(std::to_string(_invoice_id));
        where << "AND " << "i.id" << "= $" << sql_params.size() << "::BIGINT ";
    }
    if (_export_state_policy == ExportedStatePolicy::unexported)
    {
        where << "AND i.file_uuid IS NULL "; // file_xml_uuid can be null for legacy invoices
    }
    else if (_export_state_policy == ExportedStatePolicy::exported)
    {
        where << "AND i.file_uuid IS NOT NULL ";
        if (!_invoice_id)
        {
            if (_taxdate_from != boost::none)
            {
                where << "AND " << "i.taxdate" << " >= '" << to_iso_extended_string(*_taxdate_from) <<  "'::TIMESTAMP ";
            }
            else {
                where << "AND " << "i.taxdate" << " >= DATE_TRUNC('month', CURRENT_DATE - INTERVAL '1' month) ";
            }
            if (_taxdate_to != boost::none)
            {
                where << "AND " << "i.taxdate" << " < '" << to_iso_extended_string(*_taxdate_to) << "'::TIMESTAMP ";
            }
            else
            {
                where << "AND " << "i.taxdate" << " <  DATE_TRUNC('month', CURRENT_DATE) ";
            }
        }
    }
    sql << from.rdbuf() << where.rdbuf();

    if (_limit > 0)
    {
        std::stringstream limit;
        limit << "LIMIT " << std::to_string(_limit);
        sql << limit.rdbuf();
    }

    LOGGER.debug("invoice_export: select tmp");
    conn.exec("DROP TABLE IF EXISTS tmp_invoice_filter_result ");
    conn.exec_params(sql.str(), sql_params);
    conn.exec("ANALYZE tmp_invoice_filter_result");

    LOGGER.debug("invoice_export: select invoice");
    Database::Result db_result = conn.exec(
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
                   "i.file_uuid, "
                   "i.file_xml_uuid, "
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

    for (const auto& row : db_result)
    {
        LOGGER.debug(
            boost::format(
            "db_result i.id %1% i.zone_id %2% i.typ %3% i.prefix %4% i.registrar_id %5% i.balance %6% i.operations_price %7% i.vat %8% i.total %9% i.totalvat %10%")
            % std::string(row[0])
            % std::string(row[1])
            % (static_cast<int>(row[6]) == 0 ? "deposit" : "account")
            % std::string(row[7])
            % std::string(row[8])
            % std::string(row[9])
            % std::string(row[10])
            % std::string(row[11])
            % std::string(row[12])
            % std::string(row[13])
        );

        Database::ID id = row[0];
        Database::ID zone = row[1];
        Database::DateTime create_time = (boost::posix_time::ptime(row[2].isnull() ? boost::posix_time::ptime(not_a_date_time) : time_from_string(row[2])));
        Database::Date tax_date = (boost::gregorian::date(row[3].isnull() ? boost::gregorian::date(not_a_date_time) : from_string(row[3])));
        Database::Date from_date = (boost::gregorian::date(row[4].isnull() ? boost::gregorian::date(neg_infin) : from_string(row[4])));
        Database::Date to_date = (boost::gregorian::date(row[5].isnull() ? boost::gregorian::date(pos_infin) : from_string(row[5])));
        InvoiceType type = (static_cast<int>(row[6]) == 0 ? InvoiceType::deposit : InvoiceType::account);
        unsigned long long number = row[7];
        Database::ID registrar_id = row[8];
        Money credit = std::string(row[9]);
        Money price = std::string(row[10]);
        std::string vat_rate = row[11];
        Money total = std::string(row[12]);
        Money total_vat = std::string(row[13]);
        boost::optional<boost::uuids::uuid> file_pdf_uuid = row[14].isnull() ? boost::none : boost::optional<boost::uuids::uuid>{boost::uuids::string_generator()(static_cast<std::string>(row[14]))};
        boost::optional<boost::uuids::uuid> file_xml_uuid = row[15].isnull() ? boost::none : boost::optional<boost::uuids::uuid>{boost::uuids::string_generator()(static_cast<std::string>(row[15]))};
        std::string client_organization = row[16];
        std::string client_street1 = row[17];
        std::string client_city = row[18];
        std::string client_postal_code = row[19];
        std::string client_ico = row[20];
        std::string client_dic = row[21];
        std::string client_var_symb = row[22];
        std::string client_handle = row[23];
        bool client_vat = row[24];
        unsigned long long client_id = row[25];
        std::string fqdn = row[26];
        std::string client_country_code = row[27];

        boost::gregorian::date_period account_period(from_date, to_date);
        Subject client{
                client_id,
                client_handle,
                "",
                client_organization,
                client_street1,
                client_city,
                client_postal_code,
                client_country_code,
                client_ico,
                client_dic,
                "",
                "",
                "",
                "",
                "",
                "",
                client_vat};

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
                        file_pdf_uuid,
                        file_xml_uuid,
                        client_var_symb,
                        client,
                        {}, {}, {}});

    }

    LOGGER.debug(boost::str(boost::format("got %1% invoices") % invoices.size()));
    return invoices;
}

void add_actions(std::vector<Invoice>& _invoices)
{
    LOGGER.debug("add_actions");
    /* append list of actions to all selected invoices
     * it handle situation when action come from source advance invoices
     * with different vat rates by grouping
     * this is ignored on partial load
     */
    Database::Connection conn = Database::Manager::acquire();
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

    Database::Result db_result_actions = conn.exec(action_query);
    LOGGER.debug(boost::str(boost::format("add_actions: found %1% actions") % std::to_string(db_result_actions.size())));
    for (const auto& action : db_result_actions)
    {
        Database::ID invoice_id = action[0];

        const auto invoice = find_if(_invoices.begin(), _invoices.end(), [&invoice_id](const Invoice& i){ return i.id == invoice_id; });
        if (invoice != _invoices.end())
        {
            invoice_add_action(*invoice, action);
        }
    }
}

void add_sources(std::vector<Invoice>& _invoices)
{
    LOGGER.debug("add_sources");
    Database::Connection conn = Database::Manager::acquire();
    //append list of sources to all selected invoices
    Database::SelectQuery source_query;
    // clang-format off
    source_query.select() << "tmp.id, "
                             "icm.credit, "
                             "sri.vat, "
                             "sri.prefix, "
                             "icm.balance, "
                             "sri.id, "
                             "sri.total, "
                             "sri.totalvat, "
                             "sri.crdate";
      source_query.from() << "tmp_invoice_filter_result tmp "
                        "JOIN invoice_credit_payment_map icm "
                          "ON tmp.id = icm.ac_invoice_id "
                        "JOIN invoice sri "
                          "ON icm.ad_invoice_id = sri.id ";
    source_query.order_by() << "tmp.id";
    // clang-format on

    Database::Result db_result_sources = conn.exec(source_query);
    LOGGER.debug(boost::str(boost::format("add_sources: found %1% sources") % std::to_string(db_result_sources.size())));
    for (const auto& source : db_result_sources)
    {
        Database::ID invoice_id = source[0];

        const auto invoice = find_if(_invoices.begin(), _invoices.end(), [&invoice_id](const Invoice& i) { return i.id == invoice_id; });
        if (invoice != _invoices.end())
        {
            invoice_add_source(*invoice, source);
        }
    }
    conn.exec("DROP TABLE tmp_invoice_filter_result "); // FIXME
}

} // namespace

void invoice_export(
        const MessengerArgs& _messenger_args,
        const FilemanArgs& _fileman_args,
        const SecretaryArgs& _secretary_args,
        bool _invoice_dont_send,
        unsigned long long _invoice_id,
        int _limit,
        bool _debug_context)
{
    Logging::Context ctx("invoice export");
    try
    {
        const auto taxdate_from = boost::none;
        const auto taxdate_to = boost::none;
        std::vector<Invoice> invoices = get_invoices(_invoice_id, taxdate_from, taxdate_to, _limit, _debug_context ? ExportedStatePolicy::ignore : ExportedStatePolicy::unexported);
        add_actions(invoices);
        add_sources(invoices);

        for (const auto& invoice : invoices)
        {
            export_and_archive_invoice(_secretary_args, _fileman_args, invoice, _debug_context);
        }

        if (_debug_context)
        {
            return;
        }

        if (!_invoice_dont_send)
        {
            send_invoices(_messenger_args);
        }
        else
        {
            LOGGER.debug("invoice_export: invoice_dont_send");
        }
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
    catch (const std::exception& ex)
    {
        LOGGER.error(boost::format("%1%") % ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("Exception in invoice_export.");
        throw;
    }
}

void invoice_export_list(
        const FilemanArgs& _fileman_args,
        unsigned long long _invoice_id,
        boost::optional<boost::gregorian::date> _taxdate_from,
        boost::optional<boost::gregorian::date> _taxdate_to,
        int _limit)
{
    LOGGER.debug("invoice_export_list");
    try
    {
        std::vector<Invoice> invoices = get_invoices(_invoice_id, _taxdate_from, _taxdate_to, _limit, ExportedStatePolicy::exported);
        const auto needs_xml_list_tag = invoices.size() > 1;
        std::cout << "<?xml version='1.0' encoding='utf-8'?>" << std::endl;
        if (needs_xml_list_tag)
        {
            std::cout << "<list>";
        }
        LibFiled::Connection<LibFiled::Service::File> fileman_connection{
                LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
                        _fileman_args.endpoint}};
        std::for_each(rbegin(invoices), rend(invoices), [&](auto&& invoice)
        {
            LOGGER.debug(boost::str(boost::format("invoice_export_list: invoice id %1%") % invoice.id));
            LOGGER.debug(boost::str(boost::format("invoice_export_list: invoice_xml_uuid %1%") % *invoice.file_xml_uuid));
            std::ostringstream buffer;
            LibFiled::File::read(
                    fileman_connection,
                    LibFiled::File::FileUuid{*invoice.file_xml_uuid},
                    buffer);
            std::string strbuf{buffer.rdbuf()->str()};
            if (!strbuf.empty() && strbuf.substr(0, 6) == "<?xml ")
            {
                auto pos = strbuf.find('>');
                if (pos != std::string::npos)
                {
                    strbuf.erase(0, pos + 1);
                }
            }
            boost::algorithm::trim(strbuf);
            std::cout << strbuf;
        });
        if (needs_xml_list_tag)
        {
            std::cout << "</list>";
        }
        if (!invoices.empty())
        {
            std::cout << std::endl;
        }
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
    catch (const std::exception& ex)
    {
        LOGGER.error(std::string("Exception in invoice_export_list: ") + ex.what());
        throw;
    }
    catch (...)
    {
        LOGGER.error("Exception in invoice_export_list.");
        throw;
    }
}

} // namespace Admin

