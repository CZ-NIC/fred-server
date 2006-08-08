#include "dbsql.h"

typedef short disclose;
typedef char * string;

struct ContactChange
  {
    string Name;                // jmeno nebo nazev kontaktu
    string Organization;        // nazev organizace
    string Street1;             // adresa
    string Street2;             // adresa
    string Street3;             // adresa
    string City;                // obec
    string StateOrProvince;
    string PostalCode;          // PSC
    string CC;                  // country code dvojmistny kod zeme ISO
    string Telephone;
    string Fax;
    string Email;
    string NotifyEmail;         // upozornovaci email
    string VAT;                 // DIC
    string SSN;                 // SSN
    disclose DiscloseName;      // povolovani zobrazeni
    disclose DiscloseOrganization;
    disclose DiscloseAddress;
    disclose DiscloseTelephone;
    disclose DiscloseFax;
    disclose DiscloseEmail;
  };


typedef long long timestamp;


class ccReg_EPP_test
{
public:
ccReg_EPP_test(const char *db);
~ccReg_EPP_test(){};

bool DatabaseConnect();
void DatabaseDisconnect();

int Login(const char *ClID , const char* clTRID , const char* XML);

int ContactDelete(const char* handle , long clientID, const char* clTRID , const char* XML );
int ContactCreate( const char *handle, const ContactChange & c, timestamp & crDate,  long clientID, const char *clTRID , const char* XML );
private:
char database[128]; // nazev spojeni na databazi
DB DBsql;
};
