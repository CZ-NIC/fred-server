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

#ifndef DOMAIN_LIFECYCLE_PARAMS_HH_7F076CC470931270A7F5660A3E5184D0//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define DOMAIN_LIFECYCLE_PARAMS_HH_7F076CC470931270A7F5660A3E5184D0

#include "src/util/cfg/checked_types.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <chrono>

struct DomainLifecycleParams
{
    using Time = std::chrono::system_clock::time_point;
    using Day = std::chrono::duration<int, std::ratio<24 * 60 * 60>>;
    struct ListAll
    {
    };
    class AppendTop
    {
    public:
        const Time& valid_for_exdate_after() const;
        const Day& expiration_notify_period() const;
        const Day& outzone_unguarded_email_warning_period() const;
        const Day& expiration_dns_protection_period() const;
        const Day& expiration_letter_warning_period() const;
        const Day& expiration_registration_protection_period() const;
        const Day& validation_notify1_period() const;
        const Day& validation_notify2_period() const;

        void check() const;

        AppendTop& valid_for_exdate_after(Time);
        AppendTop& expiration_notify_period(Day);
        AppendTop& outzone_unguarded_email_warning_period(Day);
        AppendTop& expiration_dns_protection_period(Day);
        AppendTop& expiration_letter_warning_period(Day);
        AppendTop& expiration_registration_protection_period(Day);
        AppendTop& validation_notify1_period(Day);
        AppendTop& validation_notify2_period(Day);
    private:
        struct
        {
            boost::optional<Time> valid_for_exdate_after;
            boost::optional<Day> expiration_notify_period;
            boost::optional<Day> outzone_unguarded_email_warning_period;
            boost::optional<Day> expiration_dns_protection_period;
            boost::optional<Day> expiration_letter_warning_period;
            boost::optional<Day> expiration_registration_protection_period;
            boost::optional<Day> validation_notify1_period;
            boost::optional<Day> validation_notify2_period;
        } params_;
    };
    struct DeleteTop
    {
    };
    using Action = boost::variant<boost::blank, ListAll, AppendTop, DeleteTop>;
    Action command;
    bool has_command() const;

    class SetListAll
    {
    public:
        explicit SetListAll(DomainLifecycleParams& dst);
        void operator()(bool) const;
    private:
        Action& command_;
    };

    class SetAppendTop
    {
    public:
        explicit SetAppendTop(DomainLifecycleParams& dst);
        void operator()(bool) const;
    private:
        Action& command_;
    };
    class SetValidForExdateAfter
    {
    public:
        explicit SetValidForExdateAfter(DomainLifecycleParams& dst);
        void operator()(const Checked::Date&) const;
    private:
        Action& command_;
    };
    class SetExpirationNotifyPeriod
    {
    public:
        explicit SetExpirationNotifyPeriod(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetOutzoneUnguardedEmailWarningPeriod
    {
    public:
        explicit SetOutzoneUnguardedEmailWarningPeriod(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetExpirationDnsProtectionPeriod
    {
    public:
        explicit SetExpirationDnsProtectionPeriod(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetExpirationLetterWarningPeriod
    {
    public:
        explicit SetExpirationLetterWarningPeriod(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetExpirationRegistrationProtectionPeriod
    {
    public:
        explicit SetExpirationRegistrationProtectionPeriod(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetValidationNotify1Period
    {
    public:
        explicit SetValidationNotify1Period(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };
    class SetValidationNotify2Period
    {
    public:
        explicit SetValidationNotify2Period(DomainLifecycleParams& dst);
        void operator()(int) const;
    private:
        Action& command_;
    };

    class SetDeleteTop
    {
    public:
        explicit SetDeleteTop(DomainLifecycleParams& dst);
        void operator()(bool) const;
    private:
        Action& command_;
    };
};

#endif//DOMAIN_LIFECYCLE_PARAMS_HH_7F076CC470931270A7F5660A3E5184D0
