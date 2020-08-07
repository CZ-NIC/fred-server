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

#include "src/bin/cli/domain_lifecycle_params.hh"

#include <stdexcept>
#include <utility>

namespace {

struct IsBlank : boost::static_visitor<bool>
{
    template <typename T>
    bool operator()(const T&) const
    {
        return false;
    }
    bool operator()(const boost::blank&) const
    {
        return true;
    }
};

struct Require
{
    static void is_true(bool value)
    {
        if (!value)
        {
            struct IncorrectValue : std::runtime_error
            {
                IncorrectValue() : std::runtime_error{"value must be true"} { }
            };
            throw IncorrectValue{};
        }
    }
    static void is_blank(const DomainLifecycleParams::Action& command)
    {
        if (!boost::apply_visitor(IsBlank{}, command))
        {
            struct IsNotBlank : std::runtime_error
            {
                IsNotBlank() : std::runtime_error{"command must be blank"} { }
            };
            throw IsNotBlank{};
        }
    }
};

}//namespace {anonymous}

bool DomainLifecycleParams::has_command() const
{
    return !boost::apply_visitor(IsBlank{}, command);
}

const DomainLifecycleParams::Time& DomainLifecycleParams::Command::AppendTop::valid_for_exdate_after() const
{
    return *params_.valid_for_exdate_after;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::expiration_notify_period() const
{
    return *params_.expiration_notify_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::outzone_unguarded_email_warning_period() const
{
    return *params_.outzone_unguarded_email_warning_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::expiration_dns_protection_period() const
{
    return *params_.expiration_dns_protection_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::expiration_letter_warning_period() const
{
    return *params_.expiration_letter_warning_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::expiration_registration_protection_period() const
{
    return *params_.expiration_registration_protection_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::validation_notify1_period() const
{
    return *params_.validation_notify1_period;
}

const DomainLifecycleParams::Day& DomainLifecycleParams::Command::AppendTop::validation_notify2_period() const
{
    return *params_.validation_notify2_period;
}

void DomainLifecycleParams::Command::AppendTop::check() const
{
    struct MissingOption : std::runtime_error
    {
        MissingOption(const char* msg) : std::runtime_error{msg} { }
    };
    if (params_.valid_for_exdate_after == boost::none)
    {
        throw MissingOption{"valid_for_exdate_after must be set"};
    }
    if (params_.expiration_notify_period == boost::none)
    {
        throw MissingOption{"expiration_notify_period must be set"};
    }
    if (params_.outzone_unguarded_email_warning_period == boost::none)
    {
        throw MissingOption{"outzone_unguarded_email_warning_period must be set"};
    }
    if (params_.expiration_dns_protection_period == boost::none)
    {
        throw MissingOption{"expiration_dns_protection_period must be set"};
    }
    if (params_.expiration_letter_warning_period == boost::none)
    {
        throw MissingOption{"expiration_letter_warning_period must be set"};
    }
    if (params_.expiration_registration_protection_period == boost::none)
    {
        throw MissingOption{"expiration_registration_protection_period must be set"};
    }
    if (params_.validation_notify1_period == boost::none)
    {
        throw MissingOption{"validation_notify1_period must be set"};
    }
    if (params_.validation_notify2_period == boost::none)
    {
        throw MissingOption{"validation_notify2_period must be set"};
    }

    struct ChronologyViolation : std::runtime_error
    {
        ChronologyViolation(const char* msg) : std::runtime_error{msg} { }
    };
    if (params_.outzone_unguarded_email_warning_period <= params_.expiration_notify_period)
    {
        throw ChronologyViolation{"expiration_notify_period must be lesser then outzone_unguarded_email_warning_period"};
    }
    if (params_.expiration_dns_protection_period <= params_.outzone_unguarded_email_warning_period)
    {
        throw ChronologyViolation{"outzone_unguarded_email_warning_period must be lesser then expiration_dns_protection_period"};
    }
    if (params_.expiration_letter_warning_period <= params_.expiration_dns_protection_period)
    {
        throw ChronologyViolation{"expiration_dns_protection_period must be lesser then expiration_letter_warning_period"};
    }
    if (params_.expiration_registration_protection_period <= params_.expiration_letter_warning_period)
    {
        throw ChronologyViolation{"expiration_letter_warning_period must be lesser then expiration_registration_protection_period"};
    }
    if (params_.expiration_registration_protection_period <= params_.validation_notify1_period)
    {
        throw ChronologyViolation{"validation_notify1_period must be lesser then expiration_registration_protection_period"};
    }
    if (params_.validation_notify2_period <= params_.validation_notify1_period)
    {
        throw ChronologyViolation{"validation_notify1_period must be lesser then validation_notify2_period"};
    }
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::valid_for_exdate_after(Time value)
{
    params_.valid_for_exdate_after = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::expiration_notify_period(Day value)
{
    params_.expiration_notify_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::outzone_unguarded_email_warning_period(Day value)
{
    params_.outzone_unguarded_email_warning_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::expiration_dns_protection_period(Day value)
{
    params_.expiration_dns_protection_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::expiration_letter_warning_period(Day value)
{
    params_.expiration_letter_warning_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::expiration_registration_protection_period(Day value)
{
    params_.expiration_registration_protection_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::validation_notify1_period(Day value)
{
    params_.validation_notify1_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Command::AppendTop& DomainLifecycleParams::Command::AppendTop::validation_notify2_period(Day value)
{
    params_.validation_notify2_period = std::move(value);
    return *this;
}

DomainLifecycleParams::Setter::ListAll::ListAll(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ListAll::operator()(bool value) const
{
    Require::is_true(value);
    Require::is_blank(command_);
    command_ = Command::ListAll{};
}

DomainLifecycleParams::Setter::DeleteTop::DeleteTop(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::DeleteTop::operator()(bool value) const
{
    Require::is_true(value);
    Require::is_blank(command_);
    command_ = Command::DeleteTop{};
}

namespace {

struct GetAppendTop : boost::static_visitor<DomainLifecycleParams::Command::AppendTop*>
{
    DomainLifecycleParams::Command::AppendTop* operator()(DomainLifecycleParams::Command::AppendTop& options) const
    {
        return &options;
    }
    template <typename T>
    DomainLifecycleParams::Command::AppendTop* operator()(T&) const
    {
        struct NotAppendTopCommand : std::runtime_error
        {
            NotAppendTopCommand() : std::runtime_error{"can be used just for the \"append\" command"} { }
        };
        throw NotAppendTopCommand{};
    }
};

DomainLifecycleParams::Command::AppendTop& get_append_top_command(DomainLifecycleParams::Action& command)
{
    return *boost::apply_visitor(GetAppendTop{}, command);
}

DomainLifecycleParams::Time into_chrono_time(const boost::gregorian::date& date)
{
    const auto days_since_epoch = date - boost::gregorian::date(1970, boost::gregorian::Jan, 1);
    const auto seconds_since_epoch = days_since_epoch.days() * (24 * 60 * 60ll);
    return DomainLifecycleParams::Time{std::chrono::seconds{seconds_since_epoch}};
}

}//namespace {anonymous}

DomainLifecycleParams::Setter::AppendTop::AppendTop(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::AppendTop::operator()(bool value) const
{
    Require::is_true(value);
    Require::is_blank(command_);
    command_ = Command::AppendTop{};
}

DomainLifecycleParams::Setter::ValidForExdateAfter::ValidForExdateAfter(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ValidForExdateAfter::operator()(const Checked::Date& value) const
{
    get_append_top_command(command_).valid_for_exdate_after(into_chrono_time(value.date));
}

DomainLifecycleParams::Setter::ExpirationNotifyPeriod::ExpirationNotifyPeriod(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ExpirationNotifyPeriod::operator()(int days) const
{
    get_append_top_command(command_).expiration_notify_period(Day{days});
}

DomainLifecycleParams::Setter::OutzoneUnguardedEmailWarningPeriod::OutzoneUnguardedEmailWarningPeriod(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::OutzoneUnguardedEmailWarningPeriod::operator()(int days) const
{
    get_append_top_command(command_).outzone_unguarded_email_warning_period(Day{days});
}

DomainLifecycleParams::Setter::ExpirationDnsProtectionPeriod::ExpirationDnsProtectionPeriod(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ExpirationDnsProtectionPeriod::operator()(int days) const
{
    get_append_top_command(command_).expiration_dns_protection_period(Day{days});
}

DomainLifecycleParams::Setter::ExpirationLetterWarningPeriod::ExpirationLetterWarningPeriod(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ExpirationLetterWarningPeriod::operator()(int days) const
{
    get_append_top_command(command_).expiration_letter_warning_period(Day{days});
}

DomainLifecycleParams::Setter::ExpirationRegistrationProtectionPeriod::ExpirationRegistrationProtectionPeriod(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ExpirationRegistrationProtectionPeriod::operator()(int days) const
{
    get_append_top_command(command_).expiration_registration_protection_period(Day{days});
}

DomainLifecycleParams::Setter::ValidationNotify1Period::ValidationNotify1Period(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ValidationNotify1Period::operator()(int days) const
{
    get_append_top_command(command_).validation_notify1_period(Day{days});
}

DomainLifecycleParams::Setter::ValidationNotify2Period::ValidationNotify2Period(DomainLifecycleParams& dst)
    : command_{dst.command}
{ }

void DomainLifecycleParams::Setter::ValidationNotify2Period::operator()(int days) const
{
    get_append_top_command(command_).validation_notify2_period(Day{days});
}
