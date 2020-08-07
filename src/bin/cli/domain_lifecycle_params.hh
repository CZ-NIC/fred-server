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
    struct Command
    {
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
    };
    using Action = boost::variant<boost::blank, Command::ListAll, Command::AppendTop, Command::DeleteTop>;
    Action command;
    bool has_command() const;

    struct Setter
    {
        class ListAll
        {
        public:
            explicit ListAll(DomainLifecycleParams& dst);
            void operator()(bool) const;
        private:
            Action& command_;
        };

        class AppendTop
        {
        public:
            explicit AppendTop(DomainLifecycleParams& dst);
            void operator()(bool) const;
        private:
            Action& command_;
        };
        class ValidForExdateAfter
        {
        public:
            explicit ValidForExdateAfter(DomainLifecycleParams& dst);
            void operator()(const Checked::Date&) const;
        private:
            Action& command_;
        };
        class ExpirationNotifyPeriod
        {
        public:
            explicit ExpirationNotifyPeriod(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class OutzoneUnguardedEmailWarningPeriod
        {
        public:
            explicit OutzoneUnguardedEmailWarningPeriod(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class ExpirationDnsProtectionPeriod
        {
        public:
            explicit ExpirationDnsProtectionPeriod(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class ExpirationLetterWarningPeriod
        {
        public:
            explicit ExpirationLetterWarningPeriod(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class ExpirationRegistrationProtectionPeriod
        {
        public:
            explicit ExpirationRegistrationProtectionPeriod(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class ValidationNotify1Period
        {
        public:
            explicit ValidationNotify1Period(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };
        class ValidationNotify2Period
        {
        public:
            explicit ValidationNotify2Period(DomainLifecycleParams& dst);
            void operator()(int) const;
        private:
            Action& command_;
        };

        class DeleteTop
        {
        public:
            explicit DeleteTop(DomainLifecycleParams& dst);
            void operator()(bool) const;
        private:
            Action& command_;
        };
    };
};

#endif//DOMAIN_LIFECYCLE_PARAMS_HH_7F076CC470931270A7F5660A3E5184D0
