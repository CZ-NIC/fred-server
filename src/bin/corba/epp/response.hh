/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#define    COMMAND_OK     1000
#define    COMMAND_LOGOUT 1500
#define    COMMAND_FAILED 2400
#define COMMAND_EXCEPTION 2000  // if there is error code bigger then 2000 (two thousand) call exception 

#define COMMAND_OBJECT_NOT_EXIST 2303
#define COMMAND_OBJECT_EXIST    2302
#define COMMAND_AUTH_ERROR    2501
#define COMMAND_AUTHENTICATION_ERROR 2200 
#define COMMAND_AUTOR_ERROR    2201

#define COMMAND_NO_MESG  1300 // if no messages returns poll 
#define COMMAND_ACK_MESG 1301 // messages are in front returns poll 

#define COMMAND_PARAMETR_VALUE_POLICY_ERROR 2306 // bad value e.g. status flag server from client
#define COMMAND_STATUS_PROHIBITS_OPERATION 2304 // current status flag doesn't correspond
#define COMMAND_PROHIBITS_OPERATION 2305

#define COMMAND_NOT_ELIGIBLE_FOR_RENEW   2105    // Object is not eligible for renewal
#define COMMAND_NOT_ELIGIBLE_FOR_TRANSFER  2106 // Object is not eligible for transfer  

#define COMMAND_PARAMETR_ERROR 2005 // parameter value syntax error
#define COMMAND_PARAMETR_RANGE_ERROR  2004 // parameter is out allowed limit
#define COMMAND_PARAMETR_MISSING 2003 // missing parameter 

#define COMMAND_BILLING_FAILURE 2104  // domain billing failure

#define  COMMAND_MAX_SESSION_LIMIT 2502 // maximum number of connection
