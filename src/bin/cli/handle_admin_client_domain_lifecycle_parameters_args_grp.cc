/*
 * Copyright (C) 2020  CZ.NIC, z. s. p. o.
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

#include "src/bin/cli/handle_adminclientselection_args.hh"

#include <boost/program_options.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>

const DomainLifecycleParams& HandleAdminClientDomainLifecycleParametersArgsGrp::get_params() const noexcept
{
    return params_;
}

CommandDescription HandleAdminClientDomainLifecycleParametersArgsGrp::get_command_option()
{
    return CommandDescription{"domain_lifecycle_parameters"};
}

std::shared_ptr<boost::program_options::options_description> HandleAdminClientDomainLifecycleParametersArgsGrp::get_options_description()
{
    auto cfg_opts = std::make_shared<boost::program_options::options_description>("Commands for domain lifecycle parameters management", 120);
    cfg_opts->add_options()
            ("domain_lifecycle_parameters", "list, append, delete records in domain_lifecycle_parameters table")
            ("list", boost::program_options::value<bool>()->zero_tokens()->notifier(DomainLifecycleParams::SetListAll{params_}),
             "command: list all records from the domain_lifecycle_parameters table")
            ("append", boost::program_options::value<bool>()->zero_tokens()->notifier(DomainLifecycleParams::SetAppendTop{params_}),
             "command: append record to the domain_lifecycle_parameters table")
            ("delete", boost::program_options::value<bool>()->zero_tokens()->notifier(DomainLifecycleParams::SetDeleteTop{params_}),
             "command: delete the latest future record from the domain_lifecycle_parameters table")
            ("valid_for_exdate_after", boost::program_options::value<Checked::Date>()->notifier(DomainLifecycleParams::SetValidForExdateAfter{params_}),
             "set valid_for_exdate_after parameter (date)")
            ("expiration_notify_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetExpirationNotifyPeriod{params_}),
             "set expiration_notify_period (days)")
            ("outzone_unguarded_email_warning_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetOutzoneUnguardedEmailWarningPeriod{params_}),
             "set outzone_unguarded_email_warning_period (days)")
            ("expiration_dns_protection_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetExpirationDnsProtectionPeriod{params_}),
             "set expiration_dns_protection_period (days)")
            ("expiration_letter_warning_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetExpirationLetterWarningPeriod{params_}),
             "set expiration_letter_warning_period (days)")
            ("expiration_registration_protection_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetExpirationRegistrationProtectionPeriod{params_}),
             "set expiration_registration_protection_period (days)")
            ("validation_notify1_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetValidationNotify1Period{params_}),
             "set validation_notify1_period (days)")
            ("validation_notify2_period", boost::program_options::value<int>()->notifier(DomainLifecycleParams::SetValidationNotify2Period{params_}),
             "set validation_notify2_period (days)");
    return cfg_opts;
}

std::size_t HandleAdminClientDomainLifecycleParametersArgsGrp::handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
{
    boost::program_options::variables_map vm;
    handler_parse_args{}(this->get_options_description(), vm, argc, argv, fa);
    return option_group_index;
}
