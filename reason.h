
#define REASON_MSG_BAD_FORMAT_CONTACT_HANDLE  1 
#define REASON_MSG_BAD_FORMAT_NSSET_HANDLE    2 
#define REASON_MSG_BAD_FORMAT_FQDN 3 
#define REASON_MSG_NOT_APPLICABLE_DOMAIN 4

// pro check
#define REASON_MSG_INVALID_FORMAT       5
#define REASON_MSG_REGISTRED            6
#define REASON_MSG_PROTECTED_PERIOD     7

#define REASON_MSG_BAD_IP_ADDRESS       8
#define REASON_MSG_BAD_DNS_NAME         9
#define REASON_MSG_DUPLICITY_DNS_ADDRESS 10
#define REASON_MSG_IP_GLUE_NOT_ALLOWED      11

#define REASON_MSG_MIN_TWO_DNS_SERVER      12
	
// spatne zadana perioda pri domain renew
#define REASON_MSG_BAD_PERIOD      13
// perioda je mimo stanovene hranice
#define REASON_MSG_PERIOD_RANGE 14
// perioda neodpovida nasobku napr 12 mesicu
#define REASON_MSG_PERIOD_POLICY 15

// country code neexistuje
#define REASON_MSG_COUNTRY_NOTEXIST  16
// nezmame msgID neexistuje
#define REASON_MSG_MSGID_NOTEXIST  17

// pro ENUMval
#define REASON_MSG_VALEXPDATE_NOT_USED  18
#define REASON_MSG_VALEXPDATE_NOT_VALID 19
#define REASON_MSG_BAD_VALEXPDATE  20

// Update NSSET nelze odstranit ci pradat DNS jost ci vyradit tech kontakt min 1 tech
#define REASON_MSG_CAN_NOT_REM_DNS 21
#define REASON_MSG_CAN_NOT_ADD_DNS 22
#define REASON_MSG_CAN_NOT_REMOVE_TECH 23


// pro nsset kdyz uz existuje a je pridavn
#define REASON_MSG_TECH_EXIST  24
#define REASON_MSG_TECH_NOTEXIST 25

// pro admin
#define REASON_MSG_ADMIN_EXIST  26
#define REASON_MSG_ADMIN_NOTEXIST 27

// pro domenu kdyz neexistuje vlastnik ci nsset
#define REASON_MSG_NSSET_NOTEXIST 28
#define REASON_MSG_REGISTRANT_NOTEXIST 29

#define REASON_MSG_DNS_NAME_EXIST 30
#define REASON_MSG_DNS_NAME_NOTEXIST 31

// pro domain renew kdyz nesouhlasi zadany datum expirace
#define REASON_MSG_CUREXPDATE_NOT_EXPDATE 32


// pro mod_eppd pres fce GetTransaction
#define REASON_MSG_TRANSFER_OP 33
#define REASON_MSG_CONTACT_IDENTTYPE_MISSING 34
#define REASON_MSG_POLL_MSGID_MISSING  35
