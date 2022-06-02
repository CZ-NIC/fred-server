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
#ifndef OPERATION_CONTEXT_HH_2782B00A56871AA3254E8AFFC6A840A9//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define OPERATION_CONTEXT_HH_2782B00A56871AA3254E8AFFC6A840A9

#include "libfred/opcontext.hh"

#include <memory>
#include <utility>


namespace Test {

class OperationContext
{
public:
    OperationContext();
    explicit OperationContext(void(*commit_done)());
    ~OperationContext();
    LibFred::OperationContext::DbConn& get_conn() const;
    ::Logging::Log& get_log();
    operator LibFred::OperationContext& ();
    operator const LibFred::OperationContext& () const;
    OperationContext& commit_transaction() &;
    void commit_transaction() &&;
    OperationContext& rollback_transaction() &;
    void rollback_transaction() &&;
private:
    std::unique_ptr<LibFred::OperationContextCreator> ctx_;
    void(*commit_done_)();
};

template <typename T>
decltype(auto) rollback(T&& ctx)
{
    return std::forward<T>(ctx).rollback_transaction();
}

template <typename T>
decltype(auto) commit(T&& ctx)
{
    return std::forward<T>(ctx).commit_transaction();
}

struct Rollback
{
    template <typename T>
    explicit Rollback(T&& ctx);
};

struct Commit
{
    template <typename T>
    explicit Commit(T&& ctx);
};

}//namespace Test

#endif//OPERATION_CONTEXT_HH_2782B00A56871AA3254E8AFFC6A840A9
