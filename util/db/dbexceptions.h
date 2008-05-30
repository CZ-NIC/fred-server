#ifndef DBEXCEPTIONS_H_
#define DBEXCEPTIONS_H_

#include <exception>
#include <string>
#include <sstream>

namespace DBase {

class Exception : public std::exception {
public:
	Exception(const Exception& _ex) throw() :
		std::exception(_ex), _what(_ex._what) {
	}
	Exception& operator=(const Exception& _ex) throw() {
		_what = _ex._what;
		return *this;
	}
	~Exception() throw() {
	}
	virtual const char* what() const throw() {
		return _what.c_str();
	}
	Exception(const std::string& __what) throw() :
		_what(__what) {
	}
protected:
//	Exception(const char * __what = "") throw() :
//		_what(__what) {
//	}
	std::string _what;
};

class UnknownError : public Exception {
public:
	UnknownError() :
		Exception(std::string("Unknown error")) {
	}
};

class ConnectionFailed : public Exception {
public:
	ConnectionFailed(const std::string& _conninfo_str) :
		Exception(std::string("Connection Failed: using connection string \"")
				+ _conninfo_str + std::string("\"")) {
	}
};

class FatalError : public Exception {
public:
	FatalError(const std::string& _what) :
		Exception(_what) {
	}
};

class ResultQueryFailed : public Exception {
public:
	std::string err_msg;
	ResultQueryFailed(const std::string& _query_str, const std::string& _err_msg) :
		Exception(std::string("Result failed: ") + _query_str),
				err_msg(_err_msg) {
		_what += "\n" + err_msg;
	}
	~ResultQueryFailed() throw() {
	}
};

class ResultQueryTransactionFailed : public ResultQueryFailed {
public:
	ResultQueryTransactionFailed(const std::string& _query_str,
			const std::string& _err_msg) :
		ResultQueryFailed(_query_str, _err_msg) {
		_what = "Transaction failed: " + _what;
	}
	~ResultQueryTransactionFailed() throw() {
	}
};

class FieldOutOfRange : public Exception {
public:
	std::string query_str;
	unsigned col;
	unsigned max_col;

	FieldOutOfRange(const std::string& _query_str, unsigned _col,
			unsigned _max_col) :
		Exception(std::string("Field out of range: ")), query_str(_query_str),
				col(_col), max_col(_max_col) {
		std::stringstream what__;
		what__ << "query \"" << _query_str << "\" column index range is [0..";
		what__ << _max_col << ") requested = " << _col;
		_what += what__.str();
	}

	~FieldOutOfRange() throw() {
	}
};

class EscapeStringFailed : public Exception {
public:
	EscapeStringFailed(const std::string& _str) :
		Exception("String escaping failed: using string \"") {
		_what += _str;
		_what += "\"";
	}
};

}

#endif /*EXCEPTIONS_H_*/
