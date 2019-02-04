

#include <boost/test/unit_test.hpp>

#include "src/util/db/query/base_filters.hh"
#include "util/types/convert_sql_base.hh"
#include "src/deprecated/model/log_filter.hh"



using namespace Database::Filters;

BOOST_AUTO_TEST_SUITE(TestFiltersDateTime)

BOOST_AUTO_TEST_CASE(TestDateTimeInterval)
{
    Database::Filters::RequestImpl request_table;




    Database::DateTimeIntervalSpecial type;
    int offset;

    for (type = Database::NONE; type <= Database::PAST_YEAR; type = static_cast<Database::DateTimeIntervalSpecial>
            (static_cast<int>(type) + 1)) {
        for (offset = -2; offset <= 2 ; ++offset) {
            Database::SelectQuery sql;
            Interval<Database::DateTimeInterval> idt (Database::Column("time_begin", request_table.joinRequestTable() ));
            idt.setValue(Database::DateTimeInterval(type, offset, Database::DateTime("2011-03-10 04:00:00"),
                    Database::DateTime("2011-03-22 23:00:00")) );

            idt.serialize(sql, NULL);
            sql.make();

            // TODO : split the string by 'AND' separator, execute the dates in SQL and check the result against something saved.
            // also ad to_string() method to DateTImeIntervalSpecial to make testing easier
            boost::format fmt = boost::format(" Generated sql:%1%") % sql.where().str();
            BOOST_TEST_MESSAGE(fmt.str());
        }
    }

    /*
    Interval<Database::DateInterval> idi(Database::Column("time_begin" , request_table.joinRequestTable()));
    idi.setValue (Database::DateInterval( Database::DateTime("2011-03-10 12:00:00"), Database::DateTime("2011-03-22 23:00:00") ));

    BOOST_TEST_MESSAGE(fmt.str());
*/

}

/*
DateTimeInterval(LAST_MONTH, -1);
tmp->setName("TimeBegin");

DateTimeInterval(DateTimeIntervalSpecial _s,
                 int _offset = 0,
                 const DateTime& _start = DateTime(NEG_INF),
                     const DateTime& _stop  = DateTime(POS_INF));
                  */

BOOST_AUTO_TEST_SUITE_END();
