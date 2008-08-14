// EPP code definition in ccReg

// login
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
#define EPP_NSsetCheck           400
#define EPP_NSsetInfo            401
#define EPP_NSsetDelete          402
#define EPP_NSsetUpdate          403
#define EPP_NSsetCreate          404
#define EPP_NSsetTransfer        405 

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
#define EPP_KeySetCheck         600
#define EPP_KeySetInfo          601
#define EPP_KeySetDelete        602
#define EPP_KeySetUpdate        603
#define EPP_KeySetCreate        604
#define EPP_KeySetTransfer      605


// next function
#define EPP_UnknowAction       1000

// list function
#define EPP_ListContact        1002 
#define EPP_ListNSset          1004 
#define EPP_ListDomain         1005 
#define EPP_ListKeySet         1006

#define EPP_ClientCredit       1010

// tech check nsset
#define EPP_NSsetTest         1012


// send auth info function
#define EPP_ContactSendAuthInfo 1101
#define EPP_NSSetSendAuthInfo   1102
#define EPP_DomainSendAuthInfo  1103
#define EPP_KeySetSendAuthInfo  1106

// info functions
#define EPP_Info   1104
#define EPP_GetInfoResults  1105

// definition of operations from enum_operation
#define OPERATION_DomainCreate 1
#define OPERATION_DomainRenew  2


// definition of operations from enum_operation
#define OPERATION_DomainCreate 1
#define OPERATION_DomainRenew  2


