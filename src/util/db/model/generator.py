#!/usr/bin/env python
#
# Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
#
# This file is part of FRED.
#
# FRED is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FRED is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FRED.  If not, see <https://www.gnu.org/licenses/>.

# notes:
#  * every primary or foreign key is mapped to C++ ``unsigned long long'' type
#    (for others sql types -> C++ types see ``guess_type'' function)
#  * every primary key is created with ``.setDefault()'' (in source file)

import re, sys
from optparse import OptionParser

try:
    import pgdb
except ImportError:
    print 'no python postgres binding'
    sys.exit(1)

conn_settings = {}
prefix = ''
indent = 4
table = ''
var_prefix = ''
var_suffix = ''
file_prefix = ''
bracket = False
foreign_table_prefix = ''

def get_table_info(pg_settings, table_name):
    conn = pgdb.connect(host=pg_settings['host'], user=pg_settings['user'],
            database=pg_settings['database'], password=pg_settings['password'])
    cursor = conn.cursor()
    cursor.execute("SELECT co.column_name, co.data_type, co.is_nullable, "
            "co.column_default, kcu.constraint_name, ccu.table_name, "
            "ccu.column_name "
            "FROM information_schema.columns co "
            "LEFT JOIN information_schema.key_column_usage kcu ON "
            "co.table_name=kcu.table_name AND co.column_name=kcu.column_name "
            "LEFT JOIN information_schema.constraint_column_usage ccu ON "
            "kcu.constraint_name=ccu.constraint_name "
            "WHERE co.table_name='%s';" % table_name)
    result = cursor.fetchall()
    cursor.close()
    conn.close()
    return result

# when some column name ends with any of these suffixes, then first letter
# of the suffix will be capitalized
suffixes = ['datetime', 'date', 'time', 'vat', 'xml', 'pdf', 'data_type', 'type',
        'code', 'number', 'symb', 'sym']

# for example ``sakra_pes'' -> ``sakraPes''
def create_camel_case(ident, first_capital=False):
    if ident == None:
        return None
    ident = ident.lower()
    se = re.search('_[a-z]', ident)
    while se:
        ident1 = ident[:se.start()]
        ident2 = ident[se.start():se.end()]
        ident3 = ident[se.end():]
        ident = ident1 + ident2.replace('_', '').upper() + ident3
        se = re.search('_[a-z]', ident)
    if (ident.endswith('id') or ident.endswith('Id')) and ident != 'id':
        ident = ident[:-2]
    for suff in suffixes:
        if ident.endswith(suff) and ident != suff:
            ident1 = ident[:-len(suff)]
            ident = ident1 + suff.capitalize()
            if first_capital:
                return ident[0].capitalize() + ident[1:]
            else:
                return ident
    if first_capital:
        return ident[0].capitalize() + ident[1:]
    else:
        return ident

def is_primary(val):
    if val == None:
        return False
    if val.endswith('_pkey'):
        return True
    return False

def is_foreign(val):
    if val == None:
        return False
    if val.endswith('_fkey'):
        return True
    return False

def create_getter(ident):
    return 'get' + ident[0].upper() + ident[1:]

def create_setter(ident):
    return 'set' + ident[0].upper() + ident[1:]

def guess_type(type, pkey, fkey):
    if pkey or fkey:
        return 'unsigned long long'
    elif type == 'integer':
        return 'int'
    elif type == 'smallint':
        return 'short int'
    elif type == 'numeric':
        return 'std::string'
    elif type == 'date':
        return 'Database::Date'
    elif type.startswith('timestamp'):
        return 'Database::DateTime'
    elif type == 'bigint':
        return 'unsigned long long'
    elif type == 'character varying':
        return 'std::string'
    elif type == 'text':
        return 'std::string'
    elif type == 'character':
        return 'std::string'
    elif type == 'serial':
        return 'unsigned long long'
    elif type == 'inet':
        return 'std::string'
    elif type == 'boolean':
        return 'bool'
    elif type == 'ARRAY':
        return 'std::string'
    else:
        print "Unknown type: %s" % (type)
        sys.exit(1)

def is_not_null(val):
    return False if val == 'YES' else True

def has_default(val):
    return False if val == None else True

def write_getter(out, val):
    if val['foreign'] == True:
        out.write(' ' * indent + 'const %s &%sId() const' % (
            val['data_type'], create_getter(val['name'])))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + 'return %s%sId%s.get();\n' % (
                var_prefix, val['name'], var_suffix))
        out.write(' ' * indent + '}\n')

        out.write(' ' * indent + '%s%s *%s()' % (
            prefix, val['foreign_model'],
            create_getter(val['name'])))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + 'return %s%s.getRelated(this);\n' % (
                foreign_table_prefix, val['name']))
        out.write(' ' * indent + '}\n')
    else:
        out.write(' ' * indent + 'const %s &%s() const' % (
            val['data_type'], create_getter(val['name'])))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + 'return %s%s%s.get();\n' % (
            var_prefix, val['name'], var_suffix))
        out.write(' ' * indent + '}\n')


def write_setter(out, val):
    if val['foreign'] == True:
        out.write(' ' * indent + 'void %sId(const %s &%sId)' % (
            create_setter(val['name']), val['data_type'],
            val['name']))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + '%s%sId%s = %sId;\n' % (
            var_prefix, val['name'], var_suffix, val['name']))
        out.write(' ' * indent + '}\n')

        out.write(' ' * indent + 'void %s(%s%s *foreign_value)' % (
            create_setter(val['name']), prefix,
            val['foreign_model']))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + '%s%s.setRelated(this, foreign_value);\n' % (
            foreign_table_prefix, val['name']))
        out.write(' ' * indent + '}\n')

    else:
        out.write(' ' * indent + 'void %s(const %s &%s)' % (
            create_setter(val['name']), val['data_type'], val['name']))
        out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
        out.write(' ' * 2 * indent + '%s%s%s = %s;\n' % (
            var_prefix, val['name'], var_suffix, val['name']))
        out.write(' ' * indent + '}\n')

# write Field::Field... for class member variable
def write_field_field(out, val):
    if val['foreign'] == True:
        out.write(' ' * indent + 'Field::Field<%s> %s%sId%s;\n' % (
            val['data_type'], var_prefix, val['name'], var_suffix))
    else:
        out.write(' ' * indent + 'Field::Field<%s> %s%s%s;\n' % (
            val['data_type'], var_prefix, val['name'], var_suffix))

# write Model::Field statement
def write_model_field(out, val, cap_name):
    out.write(' ' * indent + 'static Model::Field::')
    if val['primary']:
        # static Model::Field::PrimaryKey<ModelInvoice, unsigned long long>    id;
        out.write('PrimaryKey<%s%s, %s> %s;\n' % (
            prefix, cap_name, val['data_type'], val['name']))
    elif val['foreign']:
        out.write('ForeignKey<%s%s, %s, %s%s> %sId;\n' % (
            prefix, cap_name, val['data_type'], prefix, val['foreign_model'],
            val['name']))
    else:
        out.write('Basic<%s%s, %s> %s;\n' % (
            prefix, cap_name, val['data_type'], val['name']))

# write Field::Lazy statement
def write_field_lazy(out, val):
    if val['foreign'] == False:
        return 
    # Field::Lazy::Field<ModelZone *>          m_zone;
    out.write(' ' * indent + 'Field::Lazy::Field<%s%s *> %s%s%s%s;\n' % (
        prefix, val['foreign_model'], var_prefix, foreign_table_prefix, val['name'], var_suffix))

# write Model::Field::Related
def write_related_one_to_one(out, val, cap_name):
    if val['foreign'] == False:
        return
    # static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelZone>       zone;
    out.write(' ' * indent + 'static Model::Field::Related::OneToOne<%s%s, %s, %s%s> %s%s;\n' % (
        prefix, cap_name, val['data_type'], prefix,
        val['foreign_model'], foreign_table_prefix, val['name']))

def write_includes(out, val):
    if val['foreign'] == False:
        return
    out.write('#include "%s%s.h"\n' % (
        file_prefix, val['foreign_model_fname'].lower()))

def write_h_file(out, values, table_name):
    capital_name = table_name[0].upper() + create_camel_case(table_name)[1:]

    out.write('#ifndef _%s%s_H_\n' % (
        file_prefix.upper(), create_camel_case(table_name).upper()))
    out.write('#define _%s%s_H_\n' % (
        file_prefix.upper(), create_camel_case(table_name).upper()))
    out.write('\n')
    out.write('/* << include database library settings here >> */\n')
    out.write('#include "model.h"\n')

    for val in values:
        write_includes(out, val)


    out.write('\n')
    out.write('\n')
    out.write('class %s%s:\n' % (prefix, capital_name))
    out.write(' ' * indent + 'public Model::Base {\n')
    out.write('public:\n')
    out.write(' ' * indent + '%s%s()\n' % (prefix, capital_name))
    out.write(' ' * indent + '{ }\n')
    out.write(' ' * indent + 'virtual ~%s%s()\n' % (prefix, capital_name))
    out.write(' ' * indent + '{ }\n')

    for val in values:
        write_getter(out, val)
    for val in values:
        write_setter(out, val)

    out.write('\n')
    out.write(' ' * indent + 'friend class Model::Base;\n')
    out.write('\n')
    out.write(' ' * indent + 'void insert()')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
    out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
    out.write(' ' * 2 * indent + 'Model::Base::insert(this);\n')
    out.write(' ' * 2 * indent + 'tx.commit();\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write(' ' * indent + 'void update()')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
    out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
    out.write(' ' * 2 * indent + 'Model::Base::update(this);\n')
    out.write(' ' * 2 * indent + 'tx.commit();\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write(' ' * indent + 'void reload()')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
    out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
    out.write(' ' * 2 * indent + 'Model::Base::reload(this);\n')
    out.write(' ' * 2 * indent + 'tx.commit();\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write(' ' * indent + 'void load(const Database::Row &_data)')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'Model::Base::load(this, _data);\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write(' ' * indent + 'std::string toString() const')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'return Model::Base::toString(this);\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write(' ' * indent + 'typedef Model::Field::List<Model%s>  field_list;\n' % capital_name)
    out.write(' ' * indent + 'static const field_list& getFields()')
    out.write('\n' + ' ' * indent + '{\n') if bracket else out.write(' {\n')
    out.write(' ' * 2 * indent + 'return fields;\n')
    out.write(' ' * indent + '}\n')
    out.write('\n')
    out.write('protected:\n')

    for val in values:
        write_field_field(out, val)

    out.write('\n')
    for val in values:
        write_field_lazy(out, val)

    out.write('\n')

    out.write('public:\n')
    
    for val in values:
        write_model_field(out, val, capital_name)

    out.write('\n')
    for val in values:
        write_related_one_to_one(out, val, capital_name)

    out.write('\n')
    out.write('private:\n')
    out.write(' ' * indent + 'static std::string table_name;  /** < model table name */\n')
    out.write(' ' * indent + 'static field_list  fields;      /** < list of all model fields */\n')
    out.write('}; // class %s%s\n' % (prefix, capital_name))
    out.write('\n')
    out.write('#endif // _%s%s_H_\n' % (
        file_prefix.upper(), create_camel_case(table_name).upper()))
    out.write('\n')
    # for val in values:

def get_special(val):
    special = ''
    if val['default'] == True:
        special = special + '.setDefault()'
    if val['not_null'] == True:
        special = special + '.setNotNull()'
    return special

def write_defines(out, val, cap_name):
    if val['primary'] == True:
        out.write('DEFINE_PRIMARY_KEY(%s%s, %s, %s, %s%s%s, '
                'table_name, "%s", %s)\n' % (
                    prefix, cap_name, val['data_type'], val['name'], 
                    var_prefix, val['name'], var_suffix,
                    val['db_col_name'], '.setDefault()'))
    elif val['foreign'] == True:
        #DEFINE_FOREIGN_KEY(ModelInvoice, Modelzone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id,   )
        out.write('DEFINE_FOREIGN_KEY(%s%s, %s%s, %s, %sId, %s%sId%s, table_name, "%s", %s, %s)\n' % (
            prefix, cap_name, prefix, val['foreign_model'], val['data_type'], val['name'],
            var_prefix, val['name'], var_suffix, val['db_col_name'], val['foreign_db_col_name'],
            get_special(val)))
    else:
        out.write('DEFINE_BASIC_FIELD(%s%s, %s, %s, %s%s%s, '
                'table_name, "%s", %s)\n' % (
                    prefix, cap_name, val['data_type'], val['name'], 
                    var_prefix, val['name'], var_suffix,
                    val['db_col_name'], get_special(val)))

def write_one_to_one(out, val, cap_name):
    if val['foreign'] != True:
        return
    #DEFINE_ONE_TO_ONE(ModelInvoice, ModelZone, zone, m_zone, unsigned long long , zoneId, m_zoneId)
    out.write('DEFINE_ONE_TO_ONE(%s%s, %s%s, %s%s, %s%s%s%s, %s, %sId, %s%sId%s)\n' % (
        prefix, cap_name, prefix, val['foreign_model'], foreign_table_prefix, val['name'],
        var_prefix, foreign_table_prefix, val['name'], var_suffix,
        val['data_type'], val['name'], var_prefix, val['name'], var_suffix))


def write_variables(out, val, cap_name):
    out.write(' ' * indent + '(&%s%s::%s' % (prefix, cap_name, val['name']))
    if val['foreign']:
        out.write('Id)\n')
    else:
        out.write(')\n')

def write_c_file(out, values, table_name):
    capital_name = table_name[0].upper() + create_camel_case(table_name)[1:]
    out.write('#include "%s%s.h"\n' % (
        file_prefix, table_name))
    out.write('\n')
    out.write('std::string %s%s::table_name = "%s";\n' % (
        prefix, capital_name, table_name))
    out.write('\n')

    for val in values:
        write_defines(out, val, capital_name)

    out.write('\n')

    for val in values:
        write_one_to_one(out, val, capital_name)

    out.write('\n')

    out.write('%s%s::field_list %s%s::fields = list_of<%s%s::field_list::value_type>\n'
            % (prefix, capital_name, prefix, capital_name, prefix, capital_name))

    for val in values:
        write_variables(out, val, capital_name)

    # TODO this should be on last line of previous section
    out.write(';\n\n')

def create_values(result):
    retval = []
    for res in result:
        value = {}
        value['name'] = create_camel_case(res[0])
        value['db_col_name'] = res[0]
        value['not_null'] = is_not_null(res[2])
        value['default'] = has_default(res[3])
        value['primary'] = is_primary(res[4])
        value['foreign'] = is_foreign(res[4])
        value['data_type'] = guess_type(res[1], value['primary'], value['foreign'])
        value['foreign_db_col_name'] = res[6]

        value['foreign_model'] = create_camel_case(res[5], True)
        value['foreign_model_fname'] = res[5]
        retval.append(value)
    return retval

def open_output_handler(suffix):
  return open(options.filename + '.' + suffix, 'w') if options.stdout == False else sys.stdout

def create_conn_settings(options):
    conn_settings['host'] = options.host + ':' + options.port
    conn_settings['user'] = options.user
    conn_settings['database'] = options.database
    conn_settings['password'] = options.password
    return conn_settings

if __name__ == '__main__':
    usage = 'usage: %prog [options] table_name'
    parser = OptionParser(usage=usage)
    parser.add_option('-i', '--indent', dest='indent', type='int', default=4,
            help='number of indent spaces [%default]')
    parser.add_option('-s', '--host', dest='host', default='localhost',
            help='database host [%default]')
    parser.add_option('-p', '--port', dest='port', default='5433',
            help='database port [%default]')
    parser.add_option('-u', '--user', dest='user', default='fred',
            help='database user [%default]')
    parser.add_option('-d', '--database', dest='database', default='fred',
            help='database name [%default]')
    parser.add_option('-a', '--password', dest='password', default='password',
            help='database password [%default]')
    parser.add_option('-r', '--prefix', dest='prefix', default='Model',
            help='Prefix of generated class [%default]')
    parser.add_option('-t', '--table', dest='table', help='Table name')
    parser.add_option('-o', '--output', dest='output',
            help='Send output to files (OUTPUT.h and OUTPUT.cc) instead of stdout')
    parser.add_option('', '--cc-suffix', dest='cc_suffix', default='cc',
            help='suffix for c++ source file [%default]')
    parser.add_option('', '--h-suffix', dest='h_suffix', default='h',
            help='suffix for c++ header file [%default]')
    parser.add_option('', '--var-prefix', dest='var_prefix', default='m_',
            help='prefix for each class member varible [%default]')
    parser.add_option('', '--var-suffix', dest='var_suffix', default='',
            help='suffix for each class member variable [%default]')
    parser.add_option('', '--file-prefix', dest='file_prefix', default='model_',
            help='output filename prefix [%default]')
    parser.add_option('-b', '--bracket', dest='bracket', action='store_true',
            help='opening curly bracket will be on the new line')
    parser.add_option('', '--foreign-table-prefix', dest='foreign_table_prefix', default='ftab_',
            help='output filename prefix [%default]')


    (options, args) = parser.parse_args()

    prefix = options.prefix
    indent = options.indent
    conn_settings = create_conn_settings(options)
    var_prefix = options.var_prefix
    var_suffix = options.var_suffix
    file_prefix = options.file_prefix
    bracket = options.bracket
    foreign_table_prefix = options.foreign_table_prefix

    if len(args) >= 2:
        print 'There can be only one extra parameter (table name)'
        sys.exit(1)
    if options.table == None and len(args) == 0:
        print 'Table name not found'
        sys.exit(1)
    else:
        if len(args) == 1:
            table = args[0]
        else:
            table = options.table
    outcc = None
    outh = None
    if options.output == None:
        outcc = sys.stdout
        outh = sys.stdout
    else:
        outcc = open(file_prefix + options.output + '.' + options.cc_suffix, 'w')
        outh = open(file_prefix + options.output + '.' + options.h_suffix, 'w')

    res = get_table_info(conn_settings, table)

    values = create_values(res)
    if len(values) == 0:
        print "no values found"
        sys.exit()

    if outh == sys.stdout:
        outh.write('// **********************************************\n')
        outh.write('// *\n')
        outh.write('// * Header file starts here\n')
        outh.write('// *\n')
        outh.write('// **********************************************\n')
    write_h_file(outh, values, table)

    if outcc == sys.stdout:
        outcc.write('// **********************************************\n')
        outcc.write('// *\n')
        outcc.write('// * Source file starts here\n')
        outcc.write('// *\n')
        outcc.write('// **********************************************\n')
    write_c_file(outcc, values, table)

    if outcc != sys.stdout:
        outcc.close()
    if outh != sys.stdout:
        outh.close()

