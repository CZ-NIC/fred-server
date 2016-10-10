#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/zone/zone.h"
#include "src/epp/domain/domain_create.h"
#include "src/epp/action.h"
#include "src/epp/conditionally_enqueue_notification.h"
#include "src/epp/domain/domain_create_impl.h"
#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/localization.h"
#include "src/epp/response.h"

#include "util/log/context.h"
#include "util/decimal/decimal.h"

namespace Epp {

    LocalizedCreateDomainResponse domain_create(
        const DomainCreateInputData& _data,
        const unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        const SessionLang::Enum _lang,
        const std::string& _server_transaction_handle,
        const std::string& _client_transaction_handle,
        const bool _epp_notification_disabled,
        const std::string& _client_transaction_handles_prefix_not_to_nofify,
        const bool _rifd_epp_operations_charging
    ) {

    try {
        Logging::Context logging_ctx("rifd");
        Logging::Context logging_ctx2(str(boost::format("clid-%1%") % _registrar_id));
        Logging::Context logging_ctx3(_server_transaction_handle);
        Logging::Context logging_ctx4(str(boost::format("action-%1%") % static_cast<unsigned>( Action::DomainCreate) ) );

        Fred::OperationContextCreator ctx;

        const DomainCreateResult impl_result(
            domain_create_impl(
                ctx,
                _data,
                _registrar_id,
                _logd_request_id
            )
        );

        const LocalizedCreateDomainResponse localized_result(
            create_localized_success_response(
                Response::ok,
                ctx,
                _lang
            ),
            impl_result.crtime,
            impl_result.exdate
        );

        if(_rifd_epp_operations_charging
                && Fred::InfoRegistrarById(_registrar_id).exec(ctx)
                    .info_registrar_data.system.get_value_or(false) == false)
        {
            try {
                //billing tmp impl
                unsigned long long zone_id = Fred::Zone::find_zone_in_fqdn(
                        ctx, Fred::Zone::rem_trailing_dot(_data.fqdn)).id;
                const std::string price_list_query
                    =   "SELECT enable_postpaid_operation, operation_id, price, quantity"
                        " FROM price_list pl "
                            " JOIN enum_operation eo ON pl.operation_id = eo.id "
                            " JOIN zone z ON z.id = pl.zone_id "
                        " WHERE pl.valid_from < $1::timestamp "
                              " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
                        " AND pl.zone_id = $2::bigint AND eo.operation = $3::text "
                        " ORDER BY pl.valid_from DESC "
                        " LIMIT 1 ";

                const std::string lock_registrar_credit_query
                    = "SELECT id, credit "
                        " FROM registrar_credit "
                        " WHERE registrar_id = $1::bigint "
                            " AND zone_id = $2::bigint "
                    " FOR UPDATE ";

                const std::string insert_registrar_credit_query
                    = "INSERT INTO registrar_credit_transaction "
                        " (id, balance_change, registrar_credit_id) "
                        " VALUES (DEFAULT, $1::numeric , $2::bigint) "
                    " RETURNING id ";

                {//create
                    //get_operation_payment_settings
                    Database::Result operation_price_list_result
                        = ctx.get_conn().exec_params(price_list_query
                        , Database::query_param_list(impl_result.crtime)(zone_id)("CreateDomain"));

                    if(operation_price_list_result.size() != 1) throw std::runtime_error("operation not found");

                    bool enable_postpaid_operation = operation_price_list_result[0][0];
                    unsigned long long operation_id = operation_price_list_result[0][1];
                    Decimal price_list_price = std::string(operation_price_list_result[0][2]);
                    Decimal  price_list_quantity = std::string(operation_price_list_result[0][3]);

                    if(price_list_quantity == Decimal("0")) throw std::runtime_error("price_list_quantity == 0");

                    Decimal price =  price_list_price * Decimal("1") / price_list_quantity;//count_price

                    //get_registrar_credit - lock record in registrar_credit table for registrar and zone
                    Database::Result locked_registrar_credit_result
                        = ctx.get_conn().exec_params(lock_registrar_credit_query
                        , Database::query_param_list(_registrar_id)(zone_id));

                    if(locked_registrar_credit_result.size() != 1) throw std::runtime_error("unable to get registrar_credit");

                    unsigned long long registrar_credit_id = locked_registrar_credit_result[0][0];
                    Decimal registrar_credit_balance = std::string(locked_registrar_credit_result[0][1]);

                    if(price != Decimal("0") && registrar_credit_balance < price && !enable_postpaid_operation) throw std::runtime_error("insufficient balance");

                    // save info about debt into credit
                    Database::Result registrar_credit_transaction_result
                        = ctx.get_conn().exec_params(insert_registrar_credit_query
                        , Database::query_param_list(Decimal("0") - price)(registrar_credit_id));

                    if(registrar_credit_transaction_result.size() != 1)
                    {
                        throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
                    }

                    unsigned long long registrar_credit_transaction_id = registrar_credit_transaction_result[0][0];

                    // new record to invoice_operation
                    ctx.get_conn().exec_params(
                        "INSERT INTO invoice_operation "
                        " (id, object_id, registrar_id, operation_id, zone_id" //4
                        " , crdate, quantity, date_from,  date_to "
                        " , registrar_credit_transaction_id) "
                        "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
                        " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, NULL::date "
                        " , $7::bigint) "
                        //" RETURNING id "
                        , Database::query_param_list(impl_result.id)
                        (_registrar_id)(operation_id)(zone_id)
                        ("1")(boost::date_time::c_local_adjustor<ptime>::utc_to_local(impl_result.crtime).date())
                        (registrar_credit_transaction_id)
                        );
                }

                {//renew
                    //get_operation_payment_settings
                    Database::Result operation_price_list_result
                        = ctx.get_conn().exec_params(price_list_query
                        , Database::query_param_list(impl_result.crtime)(zone_id)("RenewDomain"));

                    if(operation_price_list_result.size() != 1) throw std::runtime_error("operation not found");

                    bool enable_postpaid_operation = operation_price_list_result[0][0];
                    unsigned long long operation_id = operation_price_list_result[0][1];
                    Decimal price_list_price = std::string(operation_price_list_result[0][2]);
                    Decimal  price_list_quantity = std::string(operation_price_list_result[0][3]);

                    if(price_list_quantity == Decimal("0")) throw std::runtime_error("price_list_quantity == 0");

                    Decimal price =  price_list_price
                        * Decimal(boost::lexical_cast<std::string>(impl_result.length_of_domain_registration_in_years))
                        / price_list_quantity;//count_price

                    ctx.get_log().debug(boost::format("price_list_price: %1% price_list_quantity: %2% price: %3%")
                    % price_list_price.get_string() % price_list_quantity.get_string() % price.get_string());

                    //get_registrar_credit - lock record in registrar_credit table for registrar and zone
                    Database::Result locked_registrar_credit_result
                        = ctx.get_conn().exec_params(lock_registrar_credit_query
                        , Database::query_param_list(_registrar_id)(zone_id));

                    if(locked_registrar_credit_result.size() != 1) throw std::runtime_error("unable to get registrar_credit");

                    unsigned long long registrar_credit_id = locked_registrar_credit_result[0][0];
                    Decimal registrar_credit_balance = std::string(locked_registrar_credit_result[0][1]);

                    if(price != Decimal("0") && registrar_credit_balance < price && !enable_postpaid_operation) throw std::runtime_error("insufficient balance");

                    // save info about debt into credit
                    Database::Result registrar_credit_transaction_result
                        = ctx.get_conn().exec_params(insert_registrar_credit_query
                        , Database::query_param_list(Decimal("0") - price)(registrar_credit_id));

                    if(registrar_credit_transaction_result.size() != 1)
                    {
                        throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
                    }

                    unsigned long long registrar_credit_transaction_id = registrar_credit_transaction_result[0][0];

                    // new record to invoice_operation
                    ctx.get_conn().exec_params(
                        "INSERT INTO invoice_operation "
                        " (id, object_id, registrar_id, operation_id, zone_id" //4
                        " , crdate, quantity, date_from,  date_to "
                        " , registrar_credit_transaction_id) "
                        "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
                        " , CURRENT_TIMESTAMP::timestamp, $5::integer, $6::date, $7::date "
                        " , $8::bigint) "
                        //" RETURNING id "
                        , Database::query_param_list(impl_result.id)
                        (_registrar_id)(operation_id)(zone_id)
                        (_data.period.get_length_of_domain_registration_in_months() / 12)
                        (boost::date_time::c_local_adjustor<ptime>::utc_to_local(impl_result.crtime).date())
                        (impl_result.exdate)
                        (registrar_credit_transaction_id)
                        );
                }
            } catch (...) {
                throw BillingFailure();
            }
        }

        ctx.commit_transaction();

        conditionally_enqueue_notification(
            Notification::created,
            impl_result.create_history_id,
            _registrar_id,
            _server_transaction_handle,
            _client_transaction_handle,
            _epp_notification_disabled,
            _client_transaction_handles_prefix_not_to_nofify
        );

        return localized_result;

    } catch(const AuthErrorServerClosingConnection& ) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authentication_error_server_closing_connection,
            std::set<Error>(),
            _lang
        );
    } catch(const ParameterValuePolicyError& e) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_policy_error,
                e.get(),
                _lang
            );
    } catch(const AuthorizationError&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::authorization_error,
            std::set<Error>(),
            _lang
        );
    } catch(const BillingFailure&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::billing_failure,
            std::set<Error>(),
            _lang
        );
    } catch(const ParameterValueRangeError& e) {
            Fred::OperationContextCreator exception_localization_ctx;
            throw create_localized_fail_response(
                exception_localization_ctx,
                Response::parameter_value_range_error,
                e.get(),
                _lang
            );
    } catch(const RequiredParameterMissing& ) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_missing,
            std::set<Error>(),
            _lang
        );
    } catch(const ParameterValueSyntaxError& e) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::parameter_value_syntax_error,
            e.get(),
            _lang
        );
    } catch(const ObjectExists&) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::object_exist,
            std::set<Error>(),
            _lang
        );
    } catch(const LocalizedFailResponse& ) {
        throw;
    } catch(...) {
        Fred::OperationContextCreator exception_localization_ctx;
        throw create_localized_fail_response(
            exception_localization_ctx,
            Response::failed,
            std::set<Error>(),
            _lang
        );
    }

}

}
