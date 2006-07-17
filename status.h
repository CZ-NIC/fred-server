// status flags
#define STATUS_ok                          1 

// #define STATUS_inactive                    2 
#define STATUS_linked                      3

#define STATUS_clientDeleteProhibited    101
// #define STATUS_clientHold                102
#define STATUS_clientRenewProhibited     103
#define STATUS_clientTransferProhibited  104
#define STATUS_clientUpdateProhibited    105
#define STATUS_serverDeleteProhibited    201
// #define STATUS_serverHold                202
#define STATUS_serverRenewProhibited     203
#define STATUS_serverTransferProhibited  204
#define STATUS_serverUpdateProhibited    205
/*
#define STATUS_pendingCreate             301
#define STATUS_pendingDelete             302
#define STATUS_pendingRenew              303
#define STATUS_pendingTransfer           304
#define STATUS_pendingUpdate             305
*/

#define STATUS_DELETE        1
// #define STATUS_HOLD          2
#define STATUS_RENEW         3
#define STATUS_TRANSFER      4
#define STATUS_UPDATE        5
   
#define SERVER_STATUS 200
#define CLIENT_STATUS 100 
 
#define MAX_STATUS 20  // maximalni pocet prvku v poli

class Status
{

public:

Status(){slen=0;sadd=0;srem=0;}; // empty
~Status(){slen=0;sadd=0;srem=0;}; // empty

int  Make(char *array);
void Array(char *string);
bool Test( int status_flag ); // moznosti STATUS_ DEL TRANSFER UPD nebo RENEW
bool Add( int s );
bool Rem( int s );
bool PutAdd( int status );
bool PutRem( int status );

// int Set(int p , int s )
 int Get(int p) { if( p < slen ) return stat[p] ; else return 0 ; } ;
int Length() { return slen ;}
int RemLength(){ return srem;}
int AddLength(){ return sadd;}

// void Debug();
// vraci nazev status flagu
char * GetStatusString( int status ); 
// funkce vracejici id statusu
int  GetStatusNumber( const char  *status );

// jestli je to update status
bool IsUpdateStatus( int status )
{
if( status == STATUS_clientUpdateProhibited  || 
    status == STATUS_serverUpdateProhibited ) return true;
else return false;
}
// jestli je delete status
bool IsDeleteStatus( int status )
{
if( status == STATUS_clientDeleteProhibited  ||
    status == STATUS_serverDeleteProhibited ) return true;
else return false;
}

// jestli je to transfer status
bool IsTransferStatus( int status )
{
if( status == STATUS_clientTransferProhibited  ||
    status == STATUS_serverTransferProhibited ) return true;
else return false;
}

// jestli je to renew status
bool IsRenewStatus( int status )
{
if( status == STATUS_clientRenewProhibited  ||
    status == STATUS_serverRenewProhibited ) return true;
else return false;
}




// jestli je to server status
bool IsServerStatus( int status)
{
if( status > SERVER_STATUS ) return true;
else return false;
};

// test jestli je to clientskyu status flag
bool IsClientStatus( int status)
{
if( status > CLIENT_STATUS && status < SERVER_STATUS ) return true;
else return false;
};

// test jestli je to nejaky jinny status
bool IsOtherStatus( int status )
{
if( status <  CLIENT_STATUS ) return true;
else  return false;
}


private:
int stat[MAX_STATUS]; // pole status flagu
int stat_add[MAX_STATUS]; // pridavany status
int stat_rem[MAX_STATUS]; // ruseny status
int slen , sadd , srem; // delka status pole
};
