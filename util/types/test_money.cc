//
// very simple (and stupid) unittest for Database::Money
//

#include <iostream>

#include "money.h"

enum Operation {
    IS_SAME,
    IS_NOT_SAME,
};

void
test_money_from_string(int &testNum, const std::string &strVal, Database::Money::value_type contra, Operation operation, int &failures)
{
    Database::Money money(strVal);
    std::cout << "Test no. " << testNum;
    bool retval = (Database::Money::value_type)money == contra;
    if ((operation == IS_SAME && retval) || (operation == IS_NOT_SAME && !retval)) {
        std::cout << " OK" << std::endl;
    } else {
        std::cout 
            << " failed" << std::endl
            << "\tMoney class innner representation ("
            << (Database::Money::value_type)money << ") should be " 
            << ((operation == IS_SAME) ? "" : "not ")
            << "equal to passed Database::Money::value_type (" << contra
            << ")"
            << std::endl
            << "\tMoney was created from string (" << strVal << ")"
            << std::endl << std::endl;
        failures++;
    }
    testNum++;
}

void
test_string_from_money(int &testNum, const Database::Money &money, const std::string &contra, Operation operation, int &failures)
{
    std::cout << "Test no. " << testNum;
    bool retval = (contra.compare(money.format()) == 0);
    if ((operation == IS_SAME && retval) || (operation == IS_NOT_SAME && !retval)) {
        std::cout << " OK" << std::endl;
    } else {
        std::cout 
            << " failed" << std::endl
            << "\tMoney class string representation ("
            << money << ") should be " 
            << ((operation == IS_SAME) ? "" : "not ")
            << "equal to passed string (" << contra
            << ")"
            << std::endl
            << "\tMoney inner representation (" << (Database::Money::value_type)money << ")"
            << std::endl << std::endl;
        failures++;
    }
    testNum++;
}

int
main()
{
    int failures = 0;
    int num = 1;

    test_money_from_string(num, "0", 0, IS_SAME, failures);
    test_money_from_string(num, "0.", 0, IS_SAME, failures);
    test_money_from_string(num, "0.0", 0, IS_SAME, failures);
    test_money_from_string(num, "0.00", 0, IS_SAME, failures);

    test_money_from_string(num, "-0", 0, IS_SAME, failures);
    test_money_from_string(num, "-0.", 0, IS_SAME, failures);
    test_money_from_string(num, "-0.0", 0, IS_SAME, failures);
    test_money_from_string(num, "-0.00", 0, IS_SAME, failures);
    test_money_from_string(num, "-0.000", 0, IS_SAME, failures);

    test_money_from_string(num, ".1", 10, IS_SAME, failures);
    test_money_from_string(num, ".01", 1, IS_SAME, failures);
    test_money_from_string(num, ".011", 1, IS_SAME, failures);
    test_money_from_string(num, ".111", 11, IS_SAME, failures);
    test_money_from_string(num, "-.1", -10, IS_SAME, failures);
    test_money_from_string(num, "-.01", -1, IS_SAME, failures);
    test_money_from_string(num, "-.011", -1, IS_SAME, failures);
    test_money_from_string(num, "-.111", -11, IS_SAME, failures);

    test_money_from_string(num, "0.1", 10, IS_SAME, failures);
    test_money_from_string(num, "0.10", 10, IS_SAME, failures);
    test_money_from_string(num, "0.01", 1, IS_SAME, failures);
    test_money_from_string(num, "-0.1", -10, IS_SAME, failures);
    test_money_from_string(num, "-0.10", -10, IS_SAME, failures);
    test_money_from_string(num, "-0.01", -1, IS_SAME, failures);

    test_money_from_string(num, "123", 12300, IS_SAME, failures);
    test_money_from_string(num, "123.", 12300, IS_SAME, failures);
    test_money_from_string(num, "123.0", 12300, IS_SAME, failures);
    test_money_from_string(num, "123.4", 12340, IS_SAME, failures);
    test_money_from_string(num, "123.45", 12345, IS_SAME, failures);
    test_money_from_string(num, "123.455", 12345, IS_SAME, failures);

    test_money_from_string(num, "-123", -12300, IS_SAME, failures);
    test_money_from_string(num, "-123.", -12300, IS_SAME, failures);
    test_money_from_string(num, "-123.0", -12300, IS_SAME, failures);
    test_money_from_string(num, "-123.4", -12340, IS_SAME, failures);
    test_money_from_string(num, "-123.45", -12345, IS_SAME, failures);
    test_money_from_string(num, "-123.455", -12345, IS_SAME, failures);

    test_money_from_string(num, "21474836.47", 2147483647, IS_SAME, failures);
    test_money_from_string(num, "-21474836.47", -2147483647, IS_SAME, failures);

    test_string_from_money(num, Database::Money(12300), "123.00", IS_SAME, failures);
    test_string_from_money(num, Database::Money(12340), "123.40", IS_SAME, failures);
    test_string_from_money(num, Database::Money(12345), "123.45", IS_SAME, failures);

    test_string_from_money(num, Database::Money(-12300), "-123.00", IS_SAME, failures);
    test_string_from_money(num, Database::Money(-12340), "-123.40", IS_SAME, failures);
    test_string_from_money(num, Database::Money(-12345), "-123.45", IS_SAME, failures);

    test_string_from_money(num, Database::Money(0), "0.00", IS_SAME, failures);
    test_string_from_money(num, Database::Money(1), "0.01", IS_SAME, failures);
    test_string_from_money(num, Database::Money(10), "0.10", IS_SAME, failures);
    test_string_from_money(num, Database::Money(100), "1.00", IS_SAME, failures);

    test_string_from_money(num, Database::Money(-1), "-0.01", IS_SAME, failures);
    test_string_from_money(num, Database::Money(-10), "-0.10", IS_SAME, failures);
    test_string_from_money(num, Database::Money(-100), "-1.00", IS_SAME, failures);


    std::cout << "Number of failures: " << failures << std::endl;
    exit(failures);
}
