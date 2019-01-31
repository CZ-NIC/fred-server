#include "util/db/nullable.hh"
#include "libfred/registrable_object/contact/place_address.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/db_settings.hh"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>

class InputSequenceFailure:public std::runtime_error
{
public:
    InputSequenceFailure(const std::string& _msg):std::runtime_error(_msg) { }
};

class LineTooLong:public InputSequenceFailure
{
public:
    LineTooLong(const std::string& _msg):InputSequenceFailure(_msg) { }
};

class UnexpectedCharacter:public InputSequenceFailure
{
public:
    UnexpectedCharacter(const std::string& _msg):InputSequenceFailure(_msg) { }
};

class UnexpectedEndOfLine:public UnexpectedCharacter
{
public:
    UnexpectedEndOfLine(const std::string& _msg):UnexpectedCharacter(_msg) { }
};

class InvalidUtf8Character:public UnexpectedCharacter
{
public:
    InvalidUtf8Character(const std::string& _msg):UnexpectedCharacter(_msg) { }
};

typedef std::map< LibFred::ContactAddressType, LibFred::ContactAddress > TypeToAddress;
struct ContactId
{
    ContactId(::uint64_t _id, const std::string& _handle):id(_id), handle(_handle) { }
    ::uint64_t id;
    std::string handle;
    bool operator<(const ContactId& _b)const { return this->id < _b.id; }
};
typedef std::map< ContactId, TypeToAddress > ContactIdToAddresses;

void get_contact_addresses(std::istream& _data_source, ContactIdToAddresses& _addresses);
void import_contact_addresses(const ContactIdToAddresses& _addresses,
                              const std::string& _registrar,
                              LibFred::OperationContext& _ctx,
                              unsigned long long _logd_request_id);

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Failure: I expect 3 arguments - postgres database connection string like PQconnectdb and\n"
                     "                                logd request id\n"
                     "                                registrar handle\n\n"
                     "Usage: " << argv[0] << " PQ_CONN_INFO LOGD_REQUEST_ID REGISTRAR\n"
                     "       PQ_CONN_INFO ...... see [http://www.postgresql.org/docs/9.1/static/libpq-connect.html]\n"
                     "       LOGD_REQUEST_ID ... id of logger request\n"
                     "       REGISTRAR ......... registrar which performs the changes\n\n"
                     "Example: CONN_INFO=\"host=<host> dbname=fred user=fred password=<password>\" \\\n"
                     "         " << argv[0] << " \"$CONN_INFO\" 0 REG-CZNIC < mojeid-contact-address.out" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        const char* const conn_info = argv[1];
        const unsigned long long logd_request_id = boost::lexical_cast< unsigned long long >(argv[2]);
        const std::string registrar = argv[3];
        Database::emplace_default_manager<Database::StandaloneManager>(conn_info);
        ContactIdToAddresses addresses;
        get_contact_addresses(std::cin, addresses);
        LibFred::OperationContextCreator ctx;
        import_contact_addresses(addresses, registrar, ctx, logd_request_id);
        ctx.commit_transaction();
        return EXIT_SUCCESS;
    }
    catch (const InputSequenceFailure &e)
    {
        std::cerr << "Input data error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

namespace {

typedef Nullable<std::string> Column;
typedef std::vector<Column> Row;

Column get_column(const char*& _c, bool& _eol);
Row get_row(const char* _c);
std::string utf8_substr(const std::string& _s, ::size_t _length);
bool is_mojeid_contact_in_fred_db(::uint64_t _contact_id, LibFred::OperationContext& _ctx);

}//namespace {anonymous}

void get_contact_addresses(std::istream& _data_source, ContactIdToAddresses& _addresses)
{
    int line_count = 0;
    _addresses.clear();
    while (true)
    {
        enum { MAX_LINE_LENGTH = 1024 };
        char line[MAX_LINE_LENGTH];
        ++line_count;
        _data_source.getline(line, MAX_LINE_LENGTH);
        if (_data_source.eof())
        {
            break;
        }
        if (_data_source.fail())
        {
            std::ostringstream msg;
            msg << "line " << line_count << " too long";
            throw LineTooLong(msg.str());
        }
        const char* c = line;
        // contact_id,handle,type,street1,street2,street3,city,state,postal_code,country,company_name
        const Row row = get_row(c);
        const ContactId contact_id(boost::lexical_cast< ::uint64_t >(row[0].get_value()),
                                   row[1].get_value());
        const std::string addr_type = row[2].get_value();
        const LibFred::ContactAddressType type(LibFred::ContactAddressType::from_string(addr_type));
        LibFred::ContactAddress addr;
        addr.street1 = row[3].get_value();
        if (!row[4].isnull() && !row[4].get_value().empty())
        {
            addr.street2 = row[4].get_value();
        }
        if (!row[5].isnull() && !row[5].get_value().empty())
        {
            addr.street3 = row[5].get_value();
        }
        addr.city = row[6].get_value();
        if (!row[7].isnull() && !row[7].get_value().empty())
        {
            addr.stateorprovince = row[7].get_value();
        }
        enum { MAX_LENGTH_OF_FRED_POSTALCODE = 32 };
        addr.postalcode = utf8_substr(row[8].get_value(), MAX_LENGTH_OF_FRED_POSTALCODE);
        addr.country = row[9].get_value();
        if (!row[10].isnull() && !row[10].get_value().empty())
        {
            addr.company_name = row[10].get_value();
        }
        _addresses[contact_id][type] = addr;
    }
}

void import_contact_addresses(const ContactIdToAddresses& _addresses,
                              const std::string& _registrar,
                              LibFred::OperationContext& _ctx,
                              unsigned long long _logd_request_id)
{
    for (ContactIdToAddresses::const_iterator contact_ptr = _addresses.begin();
         contact_ptr != _addresses.end(); ++contact_ptr)
    {
        const ContactId contact = contact_ptr->first;
        if (!is_mojeid_contact_in_fred_db(contact.id, _ctx))
        {
            std::cout << "mojeid contact " << contact.id << ":" << contact.handle << " "
                         "doesn't found in fred" << std::endl;
            continue;
        }
        LibFred::UpdateContactById update_contact(contact.id, _registrar);
        std::ostringstream out;
        for (TypeToAddress::const_iterator type_ptr = contact_ptr->second.begin();
             type_ptr != contact_ptr->second.end(); ++type_ptr)
        {
            const std::string addr_type = type_ptr->first.to_string();
            out << contact.handle << "[" << addr_type << (addr_type.length() == 8 ? "] = " : "]  = ") << type_ptr->second << std::endl;
            const LibFred::ContactAddress &address = type_ptr->second;
            switch (type_ptr->first.value)
            {
                case LibFred::ContactAddressType::MAILING:
                    update_contact.set_address< LibFred::ContactAddressType::MAILING >(address);
                    break;
                case LibFred::ContactAddressType::BILLING:
                    update_contact.set_address< LibFred::ContactAddressType::BILLING >(address);
                    break;
                case LibFred::ContactAddressType::SHIPPING:
                    update_contact.set_address< LibFred::ContactAddressType::SHIPPING >(address);
                    break;
                case LibFred::ContactAddressType::SHIPPING_2:
                    update_contact.set_address< LibFred::ContactAddressType::SHIPPING_2 >(address);
                    break;
                case LibFred::ContactAddressType::SHIPPING_3:
                    update_contact.set_address< LibFred::ContactAddressType::SHIPPING_3 >(address);
                    break;
            }
        }
        try
        {
            update_contact.set_logd_request_id(_logd_request_id);
            update_contact.exec(_ctx);
        }
        catch (const std::exception& e)
        {
            std::cerr << "catch exception: " << e.what() << std::endl
                      << out.str() << std::endl
                      << boost::diagnostic_information(e) << std::endl;
            throw;
        }
    }
}

namespace
{

static const char c_quotes = '"';
static const char c_delimiter = ',';

Column get_column(const char*& _c, bool& _eol)
{
    if (*_c != c_quotes)
    {
        const char* const begin = _c;
        while ((*_c != c_delimiter) &&
               (*_c != '\0'))
        {
            ++_c;
        }
        const char* const end = _c;
        const ::ssize_t data_length = end - begin;
        _eol =* _c == '\0';
        if (!_eol)
        {
            ++_c;
        }
        Column col;
        if (0 < data_length)
        {
            col = std::string(begin, data_length);
        }
        return col;
    }

    std::string data;
    ++_c;
    for (;;)
    {
        const char* const begin = _c;
        while (*_c != c_quotes)
        {
            if (*_c == '\0')
            {
                throw UnexpectedEndOfLine("I expect quotes, not EOL");
            }
            ++_c;
        }
        const char* const end = _c;
        data.append(begin, end - begin);
        ++_c;
        if (*_c != c_quotes)
        {
            break;
        }
    }
    _eol =* _c == '\0';
    if (_eol)
    {
        return data;
    }
    if (*_c == c_delimiter)
    {
        ++_c;
        return data;
    }
    throw UnexpectedCharacter("I expect comma or EOL, not '" + std::string(_c, 1) + "'");
}

Row get_row(const char* _c)
{
    Row row;
    for (bool eol = false; !eol; row.push_back(get_column(_c, eol)))
    {
    }
    return row;
}

const char* next_utf8_character(const char* _c, const char* _e);

std::string utf8_substr(const std::string& _s, ::size_t _length)
{
    ::size_t count = 0;
    const char* c = _s.c_str();
    const char* const end = c + _s.length();
    while ((c < end) && (count < _length))
    {
        c = next_utf8_character(c, end);
        ++count;
    }
    return _s.substr(0, c - _s.c_str());
}

const char* next_utf8_character(const char* _c, const char* _e)
{
    ::uint8_t bytes;
    switch (static_cast< ::uint8_t >(*_c))
    {
        case 0x00 ... 0x7f:
            return _c + 1;
        case 0xc0 ... 0xdf:
            bytes = 2;
            break;
        case 0xe0 ... 0xef:
            bytes = 3;
            break;
        case 0xf0 ... 0xf7:
            bytes = 4;
            break;
        case 0xf8 ... 0xfb:
            bytes = 5;
            break;
        case 0xfc ... 0xfd:
            bytes = 6;
            break;
        default:
            throw InvalidUtf8Character("Invalid first utf8 byte");
    }
    if ((_e - bytes) < _c)
    {
        throw InvalidUtf8Character("Utf8 character fragmented");
    }
    const char* const retval = _c + bytes;
    for (++_c; _c < retval; ++_c)
    {
        switch (static_cast< ::uint8_t >(*_c))
        {
            case 0x80 ... 0xbf:
                break;
            default:
                throw InvalidUtf8Character("Invalid utf8 byte");
        }
    }
    return retval;
}

bool is_mojeid_contact_in_fred_db(::uint64_t _contact_id, LibFred::OperationContext& _ctx)
{
    const Database::Result result = _ctx.get_conn().exec_params(
        "SELECT 1 "
        "FROM object_registry obr "
        "JOIN contact c ON c.id=obr.id "
        "JOIN object_state os ON os.object_id=obr.id "
        "WHERE os.state_id=(SELECT id FROM enum_object_states "
                           "WHERE name='mojeidContact') AND "
              "os.valid_to IS NULL AND "
              "obr.id=$1::BIGINT",
        Database::query_param_list(_contact_id));
    return 0 < result.size();
}

}
