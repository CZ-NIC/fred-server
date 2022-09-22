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
#include "test/poc/parallel-tests/fixtures/operation_context.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"

#include "util/log/logger.hh"

#include <exception>
#include <functional>
#include <utility>


namespace Test {

namespace {

class ScopeExit
{
public:
    ScopeExit(std::function<void()> on_exit)
        : on_exit_{std::move(on_exit)}
    { }
    ~ScopeExit()
    {
        try
        {
            on_exit_();
        }
        catch (...) { }
    }
private:
    std::function<void()> on_exit_;
};

}//namespace Test::{anonymous}

OperationContext::OperationContext()
    : OperationContext{HasFreshDatabase::commit_done}
{ }

OperationContext::OperationContext(void(*commit_done)())
    : ctx_{std::make_unique<LibFred::OperationContextCreator>()},
      commit_done_{commit_done}
{ }

OperationContext::~OperationContext()
{
    try
    {
        ctx_.reset();
    }
    catch (const std::exception& e)
    {
        try { ::LOGGER.error(boost::format{"destroying of operation context failed: %1%"} % e.what()); } catch (...) { }
    }
    catch (...)
    {
        try { ::LOGGER.error("destroying of operation context failed by an unknown exception"); } catch (...) { }
    }
}

LibFred::OperationContext::DbConn& OperationContext::get_conn() const
{
    return ctx_->get_conn();
}

::Logging::Log& OperationContext::get_log()
{
    return ctx_->get_log();
}

OperationContext::operator LibFred::OperationContext& ()
{
    return static_cast<LibFred::OperationContext&>(*ctx_);
}

OperationContext::operator const LibFred::OperationContext& () const
{
    return static_cast<const LibFred::OperationContext&>(*ctx_);
}

OperationContext& OperationContext::commit_transaction() &
{
    ScopeExit scope_exit_fires{[&]() { ctx_ = std::make_unique<LibFred::OperationContextCreator>(); }};
    ctx_->commit_transaction();
    commit_done_();
    return *this;
}

void OperationContext::commit_transaction() &&
{
    ScopeExit scope_exit_fires{[&]() { ctx_.reset(); }};
    ctx_->commit_transaction();
    commit_done_();
}

OperationContext& OperationContext::rollback_transaction() &
{
    ctx_ = std::make_unique<LibFred::OperationContextCreator>();
    return *this;
}

void OperationContext::rollback_transaction() &&
{
    ctx_.reset();
}

template <typename T>
Rollback::Rollback(T&& ctx)
{
    rollback(std::forward<T>(ctx));
}

template Rollback::Rollback(OperationContext&);
template Rollback::Rollback(OperationContext&&);

template <typename T>
Commit::Commit(T&& ctx)
{
    commit(std::forward<T>(ctx));
}

template Commit::Commit(OperationContext&);
template Commit::Commit(OperationContext&&);

}//namespace Test
