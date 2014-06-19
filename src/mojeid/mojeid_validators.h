/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @contact_verification_validators.h
 *  mojeid contact verification
 */

#ifndef MOJEID_VALIDATORS_H_274554864
#define MOJEID_VALIDATORS_H_274554864

#include "src/fredlib/contact_verification/contact_verification_validators.h"
#include "src/mojeid/mojeid_checkers.h"


namespace Fred {
namespace Contact {
namespace Verification {

ContactValidator create_conditional_identification_validator_mojeid();
ContactValidator create_finish_identification_validator_mojeid();
ContactValidator create_verified_transfer_validator_mojeid();
ContactValidator create_contact_update_validator_mojeid();

}
}
}

#endif //MOJEID_VALIDATORS_H_

