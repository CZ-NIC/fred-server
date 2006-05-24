// status flags
#define STATUS_ok                          1 
#define STATUS_inactive                    2 
#define STATUS_linked                      3
#define STATUS_clientDeleteProhibited    101
#define STATUS_clientHold                102
#define STATUS_clientRenewProhibited     103
#define STATUS_clientTransferProhibited  104
#define STATUS_clientUpdateProhibited    105
#define STATUS_serverDeleteProhibited    201
#define STATUS_serverHold                202
#define STATUS_serverRenewProhibited     203
#define STATUS_serverTransferProhibited  204
#define STATUS_serverUpdateProhibited    205
#define STATUS_pendingCreate             301
#define STATUS_pendingDelete             302
#define STATUS_pendingRenew              303
#define STATUS_pendingTransfer           304
#define STATUS_pendingUpdate             305


#define MAX_STATUS 20  // maximalni pocet prvku v poli

class Status
{

public:

Status(){slen=0;}; // empty
~Status(){slen=0;}; // empty

int  Make(char *array);
void Array(char *string);
bool Test( int status );
bool Add( int status );
bool Rem( int status );
// int Set(int p , int s )
 int Get(int p) { if( p < slen ) return stat[p] ; else return 0 ; } ;
int Length() { return slen ;}
private:
int stat[MAX_STATUS]; // pole status flagu
int slen; // delka status pole
};
