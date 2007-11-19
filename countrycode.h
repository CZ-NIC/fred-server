#define MAX_CC 2

class CountryCode
{
public:
  CountryCode(
    int num);
  ~CountryCode();

  bool AddCode(
    const char *code);
  bool TestCountryCode(
    const char *cc);

  int GetNum()
  {
    return num_country;
  } // return number of countries


private:
  char (*CC)[MAX_CC+1]; // country allocated field
  int num_country; // 
  int add;
};

