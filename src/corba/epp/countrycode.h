#ifndef COUNTRYCODE_H_E104B6D592014DFCA247850CEEFC0522
#define COUNTRYCODE_H_E104B6D592014DFCA247850CEEFC0522

#include <string>
#include <vector>

#define MAX_CC 2

class CountryCode
{
    typedef std::vector<std::string> CountryCodeT;
public:
    CountryCode();
    CountryCode(int num);
    ~CountryCode();

    bool AddCode(const char *code);
    bool TestCountryCode(const char *cc);

    void load();

    int GetNum()
    {
    return cc_.size();
    } // return number of countries

private:
    CountryCodeT cc_;
};//class CountryCode
#endif
