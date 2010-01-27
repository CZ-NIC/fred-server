#ifndef COUNTRYCODE_H
#define COUNTRYCODE_H

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
#endif //COUNTRYCODE_H
