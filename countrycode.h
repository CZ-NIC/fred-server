
#define MAX_CC 2

class CountryCode
{
public:
CountryCode(int num);
~CountryCode();

bool AddCode(const char *code);
bool TestCountryCode(const char *cc);
//char * GetCountryNameEN( const char *cc ) { return GetValueFromTable("enum_country" , "country" , "id" , cc ); };
//char * GetCountryNameCS( const char *cc ) { return  GetValueFromTable("enum_country" , "country_cs" , "id" , cc ); };


int GetNum(){ return  num_country; } // vraci pocet zemi v ciselniku 



private:
char (*CC)[MAX_CC+1]; // pole kodu zemi
int num_country; // 
int add;
};




