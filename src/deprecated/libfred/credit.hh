/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file credit.h
 *  credit header
 */



#ifndef CREDIT_HH_51F9B935380D4D2C861E4C47558CD8EC
#define CREDIT_HH_51F9B935380D4D2C861E4C47558CD8EC

#include "util/decimal/decimal.hh"
#include "src/util/types/money.hh"
#include "libfred/db_settings.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"


namespace LibFred
{
    namespace Credit
    {

           unsigned long long add_credit_to_invoice(
                unsigned long long registrar_id
                , unsigned long long zone_id
                , Money price
                , unsigned long long invoice_id);


            void init_new_registrar_credit (
                Database::ID reg_id, 
                Database::ID zone_id
            );

    }//namespace Credit
} // namespace LibFred

#endif // CREDIT_H_

