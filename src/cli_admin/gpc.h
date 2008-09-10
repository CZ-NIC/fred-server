#define GPC_LIST_HEAD "074" // head of list
#define GPC_LIST_ITEM "075" // item of list
#define GPC_LIST_VZP  "076" // item of list

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
#define debug_print printf
#else
#define debug_print  /* printf */
#endif

#define MAX_ITEMS 256 // maximum amount of list item
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
  // char record[3]; // record type
  char accountNumber[GPC_MAX_ACCOUNT+1]; // client account number
  char accountName[GPC_MAX_NAME+1]; // short name of client 
  char dateOldBalance[GPC_MAX_DATE+1]; // date of old balance, date in format DDMMYY
  char oldBalance[GPC_MAX_BALANCE+1]; // old balance
  char oldBalanceSign; // sign + or -
  char newBalance[GPC_MAX_BALANCE+1]; // new balance
  char newBalanceSign; // sign + or -
  char credit[GPC_MAX_BALANCE+1]; // credit turn
  char creditSign; // sign + or -
  char debet[GPC_MAX_BALANCE+1]; // debit turn
  char debetSign; // sign + or -
  // char numList[3+1]; // ordinal number of list
  int num;
  char dateList[GPC_MAX_DATE+1]; // date of list in format DDMMYY
  // char filter[14]; // rest fulfiled with clear spaces;
};

struct GPC_ListItem
{
  // char record[3]; // record type
  char accountNumber[GPC_MAX_ACCOUNT+1]; // client account number 
  char accountOther[GPC_MAX_ACCOUNT+1]; // contra-account number
  char evidNum[GPC_MAX_EVID+1]; // evidence number
  char price[GPC_MAX_PRICE+1]; // amount 
  // char accountingCode; // accounting code
  char accountCode;
  char varSymbol[GPC_MAX_SYMBOL+1]; // variable symbol
  char konstSymbol[GPC_MAX_SYMBOL+1]; // constant symbol + contra-account sper bank code (in czech sperovy kod banky protiuctu)
  char specSymbol[GPC_MAX_SYMBOL+1];
  char dateValuta[GPC_MAX_DATE+1]; // date of balance for interest count 
  char memo[GPC_MAX_MEMO+1]; // supplementary data
  char changeCode; // change of item code 
  char dataType[4]; // data type
  char date[GPC_MAX_DATE+1]; // maturity date / putting to account
};

struct ST_Head // head of list
{
  char account[MAX_ACCOUNT]; // client account number 
  char name[MAX_NAME]; // client name
  long oldBalnce; // old balance
  long newBalance; // new balance
  long credit; // credit turn
  long debet; // debit turn
  char oldDate[MAX_DATE_STR]; // date of old balance
  char date[MAX_DATE_STR]; // date of list;
  int num; // order of list
};

struct ST_Item // statement item 
{
  char account[MAX_ACCOUNT]; // contra-account number 
  char bank[MAX_CODE]; // bank code against account from KS
  char ks[MAX_KS];
  char vs[MAX_VS];
  char ss[MAX_SS];
  char evid[MAX_MEMO]; // evidence number
  char memo[MAX_MEMO]; // note
  char date[MAX_DATE_STR]; // payable date converted date in format YYYY-MM-DD
  long price; // price in pennies;
  int code; // credit true debet false
};

class GPC
{
public:

  GPC()
  {
    numrecItem=0;
    recItem=0;
  }
  ; // empty
  ~GPC()
  {
  }
  ; // empty

  int ReadGPCFile(
    char * filename);

  bool NextItem()
  {
    if (recItem < numrecItem) {
      recItem ++;
      return true;
    } else
      return false;
  }
  ;
  bool PreviousItem()
  {
    if (recItem > 0) {
      recItem --;
      return true;
    } else
      return false;
  }
  ;
  int GetNumItems()
  {
    return numrecItem;
  }
  ;
  int GetItem()
  {
    return recItem;
  }
  ;

  void GetHead(
    ST_Head *head);
  void GetItem(
    ST_Item *item);

private:

  void ParseItem(
    char *tmp);
  void ParseHead(
    char *tmp);

  char account[GPC_MAX_ACCOUNT];

  struct GPC_ListItem gpc_item[MAX_ITEMS];
  struct GPC_ListHead gpc_head;
  int numrecItem, recItem;
};

