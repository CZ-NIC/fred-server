#define REASON_MSG_BAD_FORMAT_CONTACT_HANDLE  1 
#define REASON_MSG_BAD_FORMAT_NSSET_HANDLE    2 
#define REASON_MSG_BAD_FORMAT_FQDN 3 
#define REASON_MSG_NOT_APPLICABLE_DOMAIN 4

#define REASON_MSG_BLACKLISTED_DOMAIN  36

// for check
#define REASON_MSG_INVALID_FORMAT       5
#define REASON_MSG_REGISTRED            6
#define REASON_MSG_PROTECTED_PERIOD     7

#define REASON_MSG_BAD_IP_ADDRESS       8
#define REASON_MSG_BAD_DNS_NAME         9
#define REASON_MSG_DUPLICITY_DNS_ADDRESS 10
#define REASON_MSG_IP_GLUE_NOT_ALLOWED      11

#define REASON_MSG_MIN_TWO_DNS_SERVER      12
	
// badly entered period by domain renew
#define REASON_MSG_BAD_PERIOD      13
// period is out defined limit
#define REASON_MSG_PERIOD_RANGE 14
// period doesn't correspond to multiply (e.g. 12 months)
#define REASON_MSG_PERIOD_POLICY 15

// country code doesn't exist
#define REASON_MSG_COUNTRY_NOTEXIST  16
// unknown msgID doesn't exist
#define REASON_MSG_MSGID_NOTEXIST  17

// for ENUMval
#define REASON_MSG_VALEXPDATE_NOT_USED  18
#define REASON_MSG_VALEXPDATE_NOT_VALID 19
#define REASON_MSG_VALEXPDATE_REQUIRED 20

// Update NSSET cannot deleted or added DNS host or displaced technical contact minimal 1 technical contact  
#define REASON_MSG_CAN_NOT_REM_DNS 21
#define REASON_MSG_CAN_NOT_ADD_DNS 22
#define REASON_MSG_CAN_NOT_REMOVE_TECH 23


// for nsset which exists and it is still added 
#define REASON_MSG_TECH_EXIST  24
#define REASON_MSG_TECH_NOTEXIST 25

// for admin
#define REASON_MSG_ADMIN_EXIST  26
#define REASON_MSG_ADMIN_NOTEXIST 27

// for domain when owner doesn't exist or nsset 
#define REASON_MSG_NSSET_NOTEXIST 28
#define REASON_MSG_REGISTRANT_NOTEXIST 29

#define REASON_MSG_DNS_NAME_EXIST 30
#define REASON_MSG_DNS_NAME_NOTEXIST 31

// for domain renew when doesn't agree entered date of expiration 
#define REASON_MSG_CUREXPDATE_NOT_EXPDATE 32


// for mod_eppd through through GetTransaction function 
#define REASON_MSG_TRANSFER_OP 33
#define REASON_MSG_CONTACT_IDENTTYPE_MISSING 34
#define REASON_MSG_POLL_MSGID_MISSING  35

#define REASON_MSG_XML_VALIDITY_ERROR 37

// duplicate tech or admin  
#define REASON_MSG_DUPLICITY_CONTACT 38

#define REASON_MSG_BAD_FORMAT_KEYSET_HANDLE   39
#define REASON_MSG_KEYSET_NOTEXIST 40
#define REASON_MSG_NO_DSRECORD 41
#define REASON_MSG_CAN_NOT_REM_DSRECORD 42
#define REASON_MSG_DUPLICITY_DSRECORD 43
#define REASON_MSG_DSRECORD_EXIST 44
#define REASON_MSG_DSRECORD_NOTEXIST 45
#define REASON_MSG_DSRECORD_BAD_DIGEST_TYPE 46
#define REASON_MSG_DSRECORD_BAD_DIGEST_LENGTH 47

#define REASON_MSG_REGISTRAR_AUTOR 48

#define REASON_MSG_TECHADMIN_LIMIT              49
#define REASON_MSG_DSRECORD_LIMIT               50
#define REASON_MSG_DNSKEY_LIMIT                 51
#define REASON_MSG_NSSET_LIMIT                  52

#define REASON_MSG_NO_DNSKEY                    53
#define REASON_MSG_DNSKEY_BAD_FLAGS             54
#define REASON_MSG_DNSKEY_BAD_PROTOCOL          55
#define REASON_MSG_DNSKEY_BAD_ALG               56
#define REASON_MSG_DNSKEY_BAD_KEY_LEN           57 
#define REASON_MSG_DNSKEY_BAD_KEY_CHAR          58

#define REASON_MSG_DNSKEY_EXIST                 59
#define REASON_MSG_DNSKEY_NOTEXIST              60
#define REASON_MSG_DUPLICITY_DNSKEY             61 

#define REASON_MSG_NO_DNSKEY_DSRECORD           62
