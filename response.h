#define    COMMAND_OK     1000
#define    COMMAND_LOGOUT 1500
#define    COMMAND_FAILED 2400
#define COMMAND_OBJECT_NOT_EXIST 2303
#define COMMAND_OBJECT_EXIST    2302
#define COMMAND_AUTH_ERROR    2501
#define COMMAND_AUTHENTICATION_ERROR 2200 
#define COMMAND_AUTOR_ERROR    2201

#define COMMAND_NO_MESG  1300 // nejsou zadne zpravi vraci poll 
#define COMMAND_ACK_MESG 1301 // zpravy jsou ve fronte vraci poll

#define COMMAND_PARAMETR_VALUE_POLICY_ERROR 2306 // spatna hodnota napriklad status flag server od klienta
#define COMMAND_STATUS_PROHIBITS_OPERATION 2304 // nevyhovuje stavajici status flag
#define COMMAND_PROHIBITS_OPERATION 2305

#define COMMAND_NOT_ELIGIBLE_FOR_RENEW   2105    // Object is not eligible for renewal
#define COMMAND_NOT_ELIGIBLE_FOR_TRANSFER  2106 // Object is not eligible for transfer  

#define COMMAND_PARAMETR_ERROR 2005 // Parameter value syntax error
#define COMMAND_PARAMETR_RANGE_ERROR  2004 // parametr je mimo povolene hranice
#define COMMAND_PARAMETR_MISSING 2003 // chybejici parametr 

#define COMMAND_BILLING_FAILURE 2104  // chyba v placeni za domenu  Billing failure

#define  COMMAND_MAX_SESSION_LIMIT 2502 // maximalni pocet spojeni
