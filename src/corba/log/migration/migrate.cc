/*
 * migrate.cc
 *
 *  Created on: Jan 23, 2009
 *      Author: jvicenik
 */


#include <string>
#include <signal.h>

#include "migrate.h"

#include "old_utils/conf.h"

/*
#include "corba/mailer_manager.h"
#include <corba/ccReg.hh>
#include <corba/ccReg.hh>
*/

#include "conf/manager.h"



#include "old_utils/log.h"
#include "old_utils/conf.h"

#include "util.h"

#include "m_epp_parser.h"

using namespace Database;


static const char * EPP_SCHEMA = "/home/jvicenik/devel/fred_svn/fred/build/root/share/fred-mod-eppd/schemas/all.xsd";
static const std::string TEST_QUERY = "insert into test_pv (entry_id, name_id, value) values (345, 608, 'ghgh')";
static const std::string CONNECTION_STRING = "host=localhost port=22345 dbname=fred user=fred password=password connect_timeout=2";

// constants from log_impl.cc
const std::string Backend::LAST_PROPERTY_VALUE_ID = "select currval('log_property_value_id_seq1'::regclass)";
const std::string Backend::LAST_PROPERTY_NAME_ID = "select currval('log_property_name_id_seq'::regclass)";
const std::string Backend::LAST_ENTRY_ID = "select currval('log_entry_id_seq'::regclass)";

/*
unsigned long long Conversion<unsigned long long>::from_string(const std::string& _value) {
  return atoll(_value.c_str());
}
*/

void signal_handler(int signum);

void signal_handler(int signum)
{
	std::cout << "Received signal " << signum << ", exiting. " << std::endl;	
	exit(0);
}

// Backend ctor: connect to the database and fill property_names map
Backend::Backend(const std::string database) : conn(), trans(NULL), transaction_counter(TRANS_LIMIT)
{
	try {
		conn.open(database);

		trans = new Transaction(conn);

		Result res = trans->exec("select id, name from log_property_name");
		if (res.size() > PROP_NAMES_SIZE_LIMIT) {
			logger(" Number of entries in log_property_name is over the limit.");
			return;
		}

		for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
			Row row = *it;

			property_names[row[1]] = row[0];
		}


	} catch (Database::Exception &ex) {
		logger(ex.what());
		abort();
/*
		delete trans.release();
		trans.reset(new Transaction(uconn));
*/
		throw;
	}

}

// check if a log record with the specified ID exists and if it can be modified (time_end isn't set yet)
bool Backend::record_check(TID id)
{
	std::ostringstream query;

	query << "select time_end from log_entry where id=" << id;
	Result res = trans->exec(query.str());

	// if there is no record with specified ID
	if(res.size() == 0) {
		logger(boost::format("record with ID %1% doesn't exist") % id);
		return false;
	}

	// if the time_end is already filled (so the record
	// is complete and cannot be modified)
	if(res[0][0].isnull()) {
		logger(boost::format("record with ID %1% was already completed") % id);
		return false;
	}

	return true;
}

// find ID for the given name of a property
TID Backend::find_property_name_id(const char *name)
{
	TID name_id=0;
	std::ostringstream query;
	std::map<std::string, TID>::iterator iter;

	iter = property_names.find(name);

	if(iter != property_names.end()) {
		name_id = iter->second;
	} else {
		// if the name isn't cached in the memory, try to find it in the database
		std::string s_name = Util::escape(name);

		query << "select id from log_property_name where name='" << s_name
				<< "'";
		Result res = trans->exec(query.str());

		if (res.size() > 0) {
			// okay, it was found in the database
			name_id = res[0][0];
		} else if (res.size() == 0) {
			// if not, add it to the database
			query.str("");
			query << "insert into log_property_name (name) values ('" << s_name
					<< "')";
			trans->exec(query.str());

			query.str("");
			query << LAST_PROPERTY_NAME_ID;
			res = trans->exec(query.str());

			name_id = res[0][0];
		}

		// now that we know the right database id of the name
		// we can add it to the map
		property_names[name] = name_id;
	}

	return name_id;
}

/**
 * Find last ID used in log_property_value table
 */
inline TID Backend::find_last_property_value_id()
{
	Result res = trans->exec(LAST_PROPERTY_VALUE_ID);
	return res[0][0];
}

// insert properties for the given log_entry record
void Backend::insert_props(TID entry_id, const ccProperties& props)
{
	std::string s_val;
	std::ostringstream query;
	TID name_id, last_id = 0;

	if(props.length() == 0) {
		return;
	}

	// process the first record
	s_val = Util::escape((const char*) props[0].value);
	name_id = find_property_name_id(props[0].name);

	query.str("");
	if (props[0].child) {
		std::ostringstream msg;
		msg << "entry ID " << entry_id << ": first property marked as child. Ignoring this flag ";
		// the first property is set to child - this is an error
		logger(msg.str());
	}
	query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values (" << entry_id << ", "
		<< name_id << ", E'" << s_val << "', " << (props[0].output ? "true" : "false") << ", null)";
	trans->exec(query.str());

	// obtain last_id
	last_id = find_last_property_value_id();

	// process the rest of the sequence
	for (unsigned i = 1; i < props.length(); i++) {

		s_val = Util::escape((const char*) props[i].value);
		name_id = find_property_name_id(props[i].name);

		query.str("");
		if(props[i].child) {
			// child property set and parent id available
			query   << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values (" << entry_id << ", "
				<< name_id << ", E'" << s_val << "', " << (props[i].output ? "true" : "false") << ", " << last_id << ")";

			trans->exec(query.str());
		} else {
			// not a child property
			query << "insert into log_property_value (entry_id, name_id, value, output, parent_id) values (" << entry_id << ", "
				<< name_id << ", E'" << s_val << "', " << (props[i].output ? "true" : "false") << ", null)";
			trans->exec(query.str());

			last_id = find_last_property_value_id();
		}
	}
}

// log a new event, return the database ID of the record
TID Backend::new_event(const char *sourceIP, LogServiceType service, const char *content_in, const ccProperties& props)
{
	std::ostringstream query;
	std::string time, s_sourceIP, s_content;
	TID entry_id;

	// get formatted UTC with microseconds
	// TODO not needed
	time = boost::posix_time::to_iso_string(microsec_clock::universal_time());
	// time = "2009-01-01 9:00:00";

	try {
		// Transaction t(conn);
		if(sourceIP != NULL && sourceIP[0] != '\0') {
			// make sure these values can be safely used in an SQL statement
			s_sourceIP = Util::escape(std::string(sourceIP));
			query << "insert into log_entry (time_begin, source_ip, service) values ('" << time << "', '" << s_sourceIP << "', "
				<< service << ")";
		} else {
			query << "insert into log_entry (time_begin, service) values ('"
				<< time << "', " << service << ")";
		}
		trans->exec(query.str());

		// get the id of the new entry
		query.str("");

		query << LAST_ENTRY_ID;
		Result res = trans->exec(query.str());
		entry_id = res[0][0];

	} catch (Database::Exception &ex) {
		logger(ex.what());
		abort();
		throw;
	}

	try {
		// insert into log_raw_content
		if(content_in != NULL) {
			s_content = Util::escape(std::string(content_in));
			query.str("");
			query << "insert into log_raw_content (entry_id, content, is_response) values (" << entry_id << ", E'" << s_content << "', false)";

			// logger("fred-logd").debug(query.str());
			trans->exec(query.str());
		}

		// inserting properties
		insert_props(entry_id, props);

		commit();

	} catch (Database::Exception &ex) {
		logger(ex.what());
		abort();
		throw;
	}

	// t.commit();
	return entry_id;
}

void Backend::test()
{
	try {
		Result res = trans->exec("select str from test where tid=7");

		if (! res[0][0].isnull()) {
			std::cout << "statement select str from test where tid=7 yields not null (which is rubbish) ";
		}

		res = trans->exec("select str from test where tid=8");

		if (res[0][0].isnull()) {
			std::cout << "statement select str from test where tid=8 yields null (which is rubbish) ";
		}

	} catch (Database::Exception &ex) {
		logger(ex.what());
		abort();
		throw;
	}
}

// update existing log record with given ID
bool Backend::update_event(TID id, const ccProperties &props)
{
	std::ostringstream query;

	try {
		// perform check
		if (!record_check(id)) return false;

		insert_props(id, props);
	} catch (Database::Exception &ex) {
		logger(ex.what());
/*
		delete trans.release();
		trans.reset(new Transaction(uconn));
*/
		return false;
	}
	return true;

}

int main()
{
	epp_red_command_type cmd_type;
	epp_command_data *cdata; /* command data structure */
	parser_status pstat;
	std::string line, rawline;
	Backend serv(CONNECTION_STRING);

	clock_t time1, time2, time3, t_parser=0, t_logcomm=0, t_backend=0;

	cdata = NULL;

	epp_parser_init(EPP_SCHEMA);

	signal(15, signal_handler);
	signal(11, signal_handler);
	signal(6, signal_handler);

	while(std::getline(std::cin, rawline)) {
		size_t i;
		// if(cin.fail())

		// remove spaces at the beginning

		for (i=0; rawline[i] == ' ' && i<rawline.length(); i++)
			;

		if (i == rawline.length()) continue;

		line = rawline.substr(i);

		time1 = clock();
		pstat = epp_parse_command(line.c_str(), line.length(), &cdata, &cmd_type);

		time2 = clock();
		t_parser += time2 - time1;
		// count << (int)pstat << endl;

		/* test if the failure is serious enough to close connection */
		if (pstat > PARSER_HELLO) {
			switch (pstat) {
				case PARSER_NOT_XML:
					serv.logger("Request is not XML");
					continue;
				case PARSER_NOT_COMMAND:
					serv.logger("Request is neither a command nor hello");
					continue;
				case PARSER_ESCHEMA:
					serv.logger("Schema's parser error - check correctness of schema");
					continue;
				case PARSER_EINTERNAL:
					serv.logger("Internal parser error occured when processing request");
					continue;
				default:
					serv.logger("Unknown error occured during parsing stage");
					continue;
			}
		}

		time2 = clock();

		std::auto_ptr<ccProperties> props = log_epp_command(cdata, cmd_type, -1);

		time3 = clock();
		t_logcomm += time3 - time2;

		serv.new_event(NULL, LC_EPP, line.c_str(), *props);

		t_backend += clock() - time3;

	}

	printf(" --------- REPORT: \n"
		" Parser: 	   %12i \n"
		" log_epp_command: %12i \n"
		" Backend: 	   %12i \n",
		(int)t_parser,
		(int)t_logcomm,
		(int)t_backend);


	// if the transaction wasn't commited in the previous command
}

