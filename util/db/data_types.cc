#include "data_types.h"

namespace DBase {

std::ostream& operator<<(std::ostream &_os, const Date& _v) {
	return _os << _v.str();
}

Date operator+(const Date& _d, Days _days) {
	return Date(_d.value + days(_days.value));
}

Date operator+(const Date& _d, Months _months) {
	return Date(_d.value + months(_months.value));
}

Date operator+(const Date& _d, Years _years) {
	return Date(_d.value + years(_years.value));
}


Date operator-(const Date& _d, Days _days) {
	return Date(_d.value - days(_days.value));
}

Date operator-(const Date& _d, Months _months) {
	return Date(_d.value - months(_months.value));
}

Date operator-(const Date& _d, Years _years) {
	return Date(_d.value - years(_years.value));
}

std::ostream& operator<<(std::ostream &_os, const DateTime& _v) {
	return _os << _v.str();
}

bool operator<=(const DateTime &_left, const DateTime &_right) {
  return _left.value <= _right.value;
}

DateTime operator+(const DateTime& _d, Time _time) {
	return DateTime(_d.value + _time.get());
};

DateTime operator-(const DateTime& _d, Time _time) {
	return DateTime(_d.value - _time.get());
};

DateTime operator+(const DateTime& _d, Days _days) {
	return DateTime(_d.value + days(_days.value));
}

std::ostream& operator<<(std::ostream &_os, const Time& _v) {
	return _os << _v.str();
}

DateTime operator+(const DateTime& _d, Months _months) {
	return DateTime(_d.value + months(_months.value));
}

DateTime operator+(const DateTime& _d, Years _years) {
	return DateTime(_d.value + years(_years.value));
}

DateTime operator+(const DateTime& _d, Hours _hours) {
	return DateTime(_d.value + hours(_hours.value));
}

DateTime operator+(const DateTime& _d, Minutes _minutes) {
	return DateTime(_d.value + minutes(_minutes.value));
}

DateTime operator+(const DateTime& _d, Seconds _seconds) {
	return DateTime(_d.value + seconds(_seconds.value));
}

DateTime operator-(const DateTime& _d, Days _days) {
	return DateTime(_d.value - days(_days.value));
}

DateTime operator-(const DateTime& _d, Months _months) {
	return DateTime(_d.value - months(_months.value));
}

DateTime operator-(const DateTime& _d, Years _years) {
	return DateTime(_d.value - years(_years.value));
}

DateTime operator-(const DateTime& _d, Hours _hours) {
	return DateTime(_d.value - hours(_hours.value));
}

DateTime operator-(const DateTime& _d, Minutes _minutes) {
	return DateTime(_d.value - minutes(_minutes.value));
}

DateTime operator-(const DateTime& _d, Seconds _seconds) {
	return DateTime(_d.value - seconds(_seconds.value));
}

std::ostream& operator<<(std::ostream &_os, const DateInterval& _v) {
	return _os << _v.str();
}

std::ostream& operator<<(std::ostream &_os, const DateTimeInterval& _v) {
	return _os << _v.str();
}

std::ostream& operator<<(std::ostream &_os, const ID& _v) {
	return _os << _v.value;
}

bool operator<(const ID& _left, const ID& _right) {
	return _left.value < _right.value;
}

std::ostream& operator<<(std::ostream &_os, const Money& _v) {
	return _os << _v.value;
}

bool operator<(const Money& _left, const Money& _right) {
  return _left.value < _right.value;
}

}
