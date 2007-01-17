
#define GPC_LIST_HEAD "074" // hlavicka vypisu
#define GPC_LIST_ITEM "075" // polozka vypisu


#define  GPC_MAX_ACCOUNT 16
#define  GPC_MAX_NAME 20
#define  GPC_MAX_DATE 6
#define  GPC_MAX_BALANCE 14
#define  GPC_MAX_RECORD  128

#define GPC_MAX_EVID 13
#define GPC_MAX_PRICE 12 
#define GPC_MAX_SYMBOL 10
#define GPC_MAX_MEMO 20

#ifdef DEBUG
#define debug printf
#else
#define debug  /* printf */
#endif

#define MAX_ITEMS 256 // maximalni pocet polozek vypisu

#define MAX_DATE_STR 12
#define MAX_ACCOUNT GPC_MAX_ACCOUNT+1
#define MAX_NAME GPC_MAX_NAME+1
#define MAX_MEMO GPC_MAX_MEMO+1
#define MAX_SS GPC_MAX_SYMBOL+1
#define MAX_VS GPC_MAX_SYMBOL+1
#define MAX_KS 4+1
#define MAX_PRICE GPC_MAX_PRICE+2
#define MAX_CODE 4+1

#define DEBET -1
#define CREDIT 1

struct GPC_ListHead
{
// char record[3]; // typ zaznamu
char accountNumber[GPC_MAX_ACCOUNT+1]; // cislu uctu klienta
char accountName[GPC_MAX_NAME+1]; // zkraceny nazev klienta
char dateOldBalance[GPC_MAX_DATE+1]; // datum stareho zustatku datum v formatu DDMMYY
char oldBalance[GPC_MAX_BALANCE+1]; // stary zustatek
char oldBalanceSign; // znamenko + ci -
char newBalance[GPC_MAX_BALANCE+1]; // novy zustatek
char newBalanceSign; // znamenko + ci -
char credit[GPC_MAX_BALANCE+1]; // kreditni obrat
char creditSign; // znamenko + ci -
char debet[GPC_MAX_BALANCE+1]; // debetni obrat
char debetSign; // znamenko + ci -
// char numList[3+1]; // poradove cislo vypisu
int num;
char dateList[GPC_MAX_DATE+1]; // datum vypisu v formatu DDMMYY
// char filter[14]; // zbytek vyplneny mezerami;
};


struct GPC_ListItem
{
// char record[3]; // typ zaznamu
char accountNumber[GPC_MAX_ACCOUNT+1]; // cislu uctu klienta
char accountOther[GPC_MAX_ACCOUNT+1]; // cislo protiuctu
char evidNum[GPC_MAX_EVID+1]; // cislo dokladu
char price[GPC_MAX_PRICE+1]; // castka 
// char accountingCode; // kod uctovani
char accountCode;
char varSymbol[GPC_MAX_SYMBOL+1]; // variabilni sybmol
char konstSymbol[GPC_MAX_SYMBOL+1]; // kontsani symbol + sperovy kod banky protiuctu
char specSymbol[GPC_MAX_SYMBOL+1]; 
char dateValuta[GPC_MAX_DATE+1]; // datum zustatku pro vypocet uroku
char memo[GPC_MAX_MEMO+1]; // doplnujici udaj poznamka
char changeCode; // kod zmeny polozky
char dataType[4]; // dryh dat
char date[GPC_MAX_DATE+1]; // datum slatnosti /pripsani na ucet
};


struct ST_Head // hlavick avypisu
{
char account[MAX_ACCOUNT]; // cislo uctu klienta
char name[MAX_NAME]; // nazev klienta
long oldBalnce; // stary sustatek
long newBalance; // novy zustatek
long credit; // creditni obrat
long debet; // debetni obrat
char oldDate[MAX_DATE_STR]; // datum stareho zustatku
char date[MAX_DATE_STR]; // dattum vypisu;
int num; // porwdi vypisu
};

struct ST_Item // statem item polozka vypisu
{
char account[MAX_ACCOUNT]; // cislo protiuctu
char bank[MAX_CODE]; // kod banky proti uctu z KS
char ks[MAX_KS];
char vs[MAX_VS];
char ss[MAX_SS];
char evid[MAX_MEMO]; // doklad cislo
char memo[MAX_MEMO]; // poznamka
char date[MAX_DATE_STR]; // datum splatnosti  zkonvertovane datum fe romatu YYYY-MM-DD
long price; // castka v halirich;
int code; // credit true debet false
};

class GPC
{
public:

GPC(){numrecItem=0;recItem=0;}; // empty
~GPC(){}; // empty

int ReadGPCFile( char * filename );

bool NextItem()  {  if( recItem < numrecItem ) { recItem ++ ; return true; } else return false; };
bool PreviousItem()  {  if( recItem > 0 ) { recItem -- ; return true; } else return false; };
int  GetNumItems() { return numrecItem ;};
int  GetItem() { return recItem; } ;


void GetHead( ST_Head *head );
void GetItem( ST_Item *item );


private:

void ParseItem(char *tmp );
void ParseHead(char *tmp );

char account[GPC_MAX_ACCOUNT];

struct  GPC_ListItem gpc_item[MAX_ITEMS];
struct  GPC_ListHead gpc_head;
int numrecItem , recItem;
};

