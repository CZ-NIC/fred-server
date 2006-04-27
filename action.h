// definice kodu EPP v ccReg
// prihlasovani
#define EPP_ClientLogin         100
#define EPP_ClientLogout        101

// funkce pro praci s kontakty
#define EPP_ContactCheck        200
#define EPP_ContactInfo         201
#define EPP_ContactDelete       202  
#define EPP_ContactUpdate       203 
#define EPP_ContactCreate       204
#define EPP_ContactTransfer     205
// funkce pro hosty
#define EPP_HostCheck           300
#define EPP_HostInfo            301 
#define EPP_HostDelete          302 
#define EPP_HostUpdate          303
#define EPP_HostCreate          304

// funkce pro NSSET
#define EPP_NSsetCheck           400
#define EPP_NSsetInfo            401
#define EPP_NSsetDelete          402
#define EPP_NSsetUpdate          403
#define EPP_NSsetCreate          404


// funkce pro domeny
#define EPP_DomainCheck         500
#define EPP_DomainInfo          501
#define EPP_DomainDelete        502
#define EPP_DomainUpdate        503  
#define EPP_DomainCreate        504  
#define EPP_DomainTransfer      505 
#define EPP_DomainRenew         506 

// dalsi funkce
#define EPP_UnknowAction       1000
