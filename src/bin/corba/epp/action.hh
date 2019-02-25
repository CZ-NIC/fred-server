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
#define EPP_ClientLogin         100
#define EPP_ClientLogout        101

// poll function
#define EPP_PollAcknowledgement  120
#define EPP_PollResponse         121

// function for work with contacts
#define EPP_ContactCheck        200
#define EPP_ContactInfo         201
#define EPP_ContactDelete       202
#define EPP_ContactUpdate       203
#define EPP_ContactCreate       204
#define EPP_ContactTransfer     205


// NSSET function
#define EPP_NssetCheck           400
#define EPP_NssetInfo            401
#define EPP_NssetDelete          402
#define EPP_NssetUpdate          403
#define EPP_NssetCreate          404
#define EPP_NssetTransfer        405

// domain function
#define EPP_DomainCheck         500
#define EPP_DomainInfo          501
#define EPP_DomainDelete        502
#define EPP_DomainUpdate        503
#define EPP_DomainCreate        504
#define EPP_DomainTransfer      505
#define EPP_DomainRenew         506
#define EPP_DomainTrade         507

// keyset function
#define EPP_KeysetCheck         600
#define EPP_KeysetInfo          601
#define EPP_KeysetDelete        602
#define EPP_KeysetUpdate        603
#define EPP_KeysetCreate        604
#define EPP_KeysetTransfer      605


// next function
#define EPP_UnknowAction       1000

// list function
#define EPP_ListContact        1002
#define EPP_ListNsset          1004
#define EPP_ListDomain         1005
#define EPP_ListKeyset         1006

#define EPP_ClientCredit       1010

// tech check nsset
#define EPP_NssetTest         1012


// send auth info function
#define EPP_ContactSendAuthInfo 1101
#define EPP_NssetSendAuthInfo   1102
#define EPP_DomainSendAuthInfo  1103
#define EPP_KeysetSendAuthInfo  1106

// info functions
#define EPP_Info   1104
#define EPP_GetInfoResults  1105

// definition of operations from enum_operation
#define OPERATION_DomainCreate 1
#define OPERATION_DomainRenew  2


// definition of operations from enum_operation
#define OPERATION_DomainCreate 1
#define OPERATION_DomainRenew  2


