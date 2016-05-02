/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 */

#ifndef FREDLIB_NOTIFIER2_EXCEPTIONS_06873297102
#define FREDLIB_NOTIFIER2_EXCEPTIONS_06873297102

namespace Notification {

struct ExceptionInterface {
    virtual const char* what() const = 0;
    virtual ~ExceptionInterface() {}
};

struct ExceptionEventNotImplemented : ExceptionInterface {
    const char* what() const { return "event not implemented"; }
};

struct ExceptionObjectTypeNotImplemented : ExceptionInterface {
    const char* what() const { return "object type not yet implemented"; }
};

struct ExceptionAddressTypeNotImplemented : ExceptionInterface {
    const char* what() const { return "address type not yet implemented"; }
};

struct ExceptionUnknownHistoryId : ExceptionInterface {
    const char* what() const { return "unknown history id"; }
};

struct ExceptionInvalidUpdateEvent : ExceptionInterface {
    const char* what() const { return "invalid update event"; }
};

struct ExceptionUnknownEmailTemplate : ExceptionInterface {
    const char* what() const { return "unknown e-mail template"; }
};

struct ExceptionMissingChangesFlagInUpdateNotificationContent : ExceptionInterface {
    const char* what() const { return "missing changes flag in update notification content"; }
};

struct ExceptionInvalidNotificationContent : ExceptionInterface {
    const char* what() const { return "invalid notification content"; }
};

struct ExceptionDataLoss : ExceptionInterface {
    const char* what() const { return "data loss"; }
};

struct ExceptionUnknownSSNType : ExceptionInterface {
    const char* what() const { return "unknown SSN type"; }
};

}

#endif
