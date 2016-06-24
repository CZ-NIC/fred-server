/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "tests/interfaces/epp/util.h"

#include <pthread.h>

namespace Test {

RegistrarProvider::RegistrarProvider(Fred::OperationContext &ctx)
:   registrar_a_(create_registrar(ctx, "REGISTRAR_A", false)),
    registrar_b_(create_registrar(ctx, "REGISTRAR_B", false)),
    sys_registrar_(create_registrar(ctx, "SYS_REGISTRAR", true))
{
}

const Fred::InfoRegistrarData& RegistrarProvider::get_registrar_a()
{
    return get_const_instance().registrar_a_;
}

const Fred::InfoRegistrarData& RegistrarProvider::get_registrar_b()
{
    return get_const_instance().registrar_b_;
}

const Fred::InfoRegistrarData& RegistrarProvider::get_sys_registrar()
{
    return get_const_instance().sys_registrar_;
}

Fred::InfoRegistrarData RegistrarProvider::create_registrar(
    Fred::OperationContext &ctx,
    const std::string &handle,
    bool system_registrar)
{
    for (int cnt = 0; true; ++cnt) {
        std::string registrar_handle;
        try {
            registrar_handle = handle;
            if (0 < cnt) {
                std::ostringstream out;
                out << cnt;
                registrar_handle += out.str();
            }
            const Fred::InfoRegistrarData data =
                Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
            if (data.system.get_value_or(false) == system_registrar) {
                return data;
            }
        }
        catch (const Fred::InfoRegistrarByHandle::Exception &e) {
            if (!e.is_set_unknown_registrar_handle()) {
                throw;
            }
            Fred::CreateRegistrar(registrar_handle).set_system(system_registrar).exec(ctx);
            return Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data;
        }
    }
}

const RegistrarProvider& RegistrarProvider::get_const_instance()
{
    static const RegistrarProvider *instance_ptr = NULL;
    if (instance_ptr == NULL) {
        static ::pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        class Lock
        {
        public:
            Lock(::pthread_mutex_t &_mutex):mutex_(_mutex)
            {
                static const int success = 0;
                if (::pthread_mutex_lock(&mutex_) != success) {
                    throw std::runtime_error("unable to lock mutex");
                }
            }
            ~Lock() { ::pthread_mutex_unlock(&mutex_); }
        private:
            ::pthread_mutex_t &mutex_;
        } lock(mutex);
        if (instance_ptr == NULL) {
            Fred::OperationContextCreator ctx;
            static const RegistrarProvider instance(ctx);
            ctx.commit_transaction();
            instance_ptr = &instance;
        }
    }
    return *instance_ptr;
}

}//namespace Test
