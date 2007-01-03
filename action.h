// definice kodu EPP v ccReg

// prihlasovani
#define EPP_ClientLogin         100
#define EPP_ClientLogout        101

// poll funkce
#define EPP_PollAcknowledgement  120 
#define EPP_PollResponse         121

// funkce pro praci s kontakty
#define EPP_ContactCheck        200
#define EPP_ContactInfo         201
#define EPP_ContactDelete       202  
#define EPP_ContactUpdate       203 
#define EPP_ContactCreate       204
#define EPP_ContactTransfer     205


// funkce pro NSSET
#define EPP_NSsetCheck           400
#define EPP_NSsetInfo            401
#define EPP_NSsetDelete          402
#define EPP_NSsetUpdate          403
#define EPP_NSsetCreate          404
#define EPP_NSsetTransfer        405 

// funkce pro domeny
#define EPP_DomainCheck         500
#define EPP_DomainInfo          501
#define EPP_DomainDelete        502
#define EPP_DomainUpdate        503  
#define EPP_DomainCreate        504  
#define EPP_DomainTransfer      505 
#define EPP_DomainRenew         506 
#define EPP_DomainTrade         507 


// dalsi funkce
#define EPP_UnknowAction       1000

// list funkce
#define EPP_ListContact        1002 
#define EPP_ListNSset          1004 
#define EPP_ListDomain         1005 

#define EPP_ClientCredit       1010

// send auth info fce
#define EPP_ContactSendAuthInfo 1101
#define EPP_NSSetSendAuthInfo   1102
#define EPP_DomainSendAuthInfo  1103

// definice operaci z enum_operation
#define OPERATION_DomainCreate 1
#define OPERATION_DomainRenew  2

