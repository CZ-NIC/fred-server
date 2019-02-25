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


def open_output_handler(options, suffix):
  return open(options.filename + '.' + suffix, 'w') if options.stdout == False else sys.stdout
  

def skeleton_header(options, name):
  upper_name    = name.upper();
  capital_name  = name.capitalize();
  indent        = options.indent
  out           = open_output_handler(options, 'h') 

  out.write('#ifndef MODEL_%s_H_\n' % upper_name)
  out.write('#define MODEL_%s_H_\n' % upper_name)
  out.write('\n')
  out.write('/* << include database library settings here >> */\n')
  out.write('#include "model.h"\n')
  out.write('\n')
  out.write('\n')
  out.write('class %s : public Model::Base {\n' % capital_name)
  out.write('public:\n')
  out.write(' ' * indent + '%s();\n' % capital_name)
  out.write(' ' * indent + 'virtual ~%s();\n' % capital_name)
  out.write('\n')
  out.write(' ' * indent + '/* << member fields getters and setter should follow >> */\n')
  out.write('\n')
  out.write('\n')
  out.write(' ' * indent + 'friend class Model::Base;\n')
  out.write('\n')
  out.write(' ' * indent + 'void insert() {\n')
  out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
  out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
  out.write(' ' * 2 * indent + 'Model::Base::insert(this);\n')
  out.write(' ' * 2 * indent + 'tx.commit();\n')
  out.write(' ' * indent + '}\n')
  out.write('\n')
  out.write('\n')
  out.write(' ' * indent + 'void update() {\n')
  out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
  out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
  out.write(' ' * 2 * indent + 'Model::Base::update(this);\n')
  out.write(' ' * 2 * indent + 'tx.commit();\n')
  out.write(' ' * indent + '}\n')
  out.write('\n')
  out.write('\n')
  out.write(' ' * indent + 'void reload() {\n')
  out.write(' ' * 2 * indent + 'Database::Connection conn = Database::Manager::acquire();\n')
  out.write(' ' * 2 * indent + 'Database::Transaction tx(conn);\n')
  out.write(' ' * 2 * indent + 'Model::Base::reload(this);\n')
  out.write(' ' * 2 * indent + 'tx.commit();\n')
  out.write(' ' * indent + '}\n')
  out.write('\n')
  out.write('\n')
  out.write(' ' * indent + 'void load(const Database::Row &_data) {\n')
  out.write(' ' * 2 * indent + 'Model::Base::load(this, _data);\n')
  out.write(' ' * indent + '}\n')
  out.write('\n')
  out.write('\n')
  out.write(' ' * indent + 'typedef Model::Field::List<%s>  field_list;\n' % capital_name)
  out.write(' ' * indent + 'static const field_list& getFields() {\n')
  out.write(' ' * 2 * indent + 'return fields;\n')
  out.write(' ' * indent + '}\n')
  out.write('\n')
  out.write('\n')
  out.write('protected:\n')
  out.write(' ' * indent + '/* << member fields (Field::Field) should follow >> */\n')
  out.write('\n')
  out.write('\n')
  out.write('public:\n')
  out.write(' ' * indent + '/* << model fields (static Model::Field) should follow >> */\n')
  out.write('\n')
  out.write('\n')
  out.write('private:\n')
  out.write(' ' * indent + 'static std::string table_name;  /** < model table name */\n')
  out.write(' ' * indent + 'static field_list  fields;      /** < list of all model fields */\n')
  out.write('};\n')
  out.write('\n')
  out.write('\n')
  out.write('#endif /*MODEL_%s_H_*/\n' % upper_name)
  out.write('\n')


def skeleton_source(options, name):
  lowered_name = name.lower()
  capital_name = name.capitalize()
  out          = open_output_handler(options, 'cc') 

  out.write('#include "model_%s.h"\n' % lowered_name)
  out.write('\n')
  out.write('\n')
  out.write('%s::%s() {\n' % (capital_name, capital_name))
  out.write('}\n')
  out.write('\n')
  out.write('\n')
  out.write('%s::~%s() {\n' % (capital_name, capital_name))
  out.write('}\n')
  out.write('\n')
  out.write('\n')
  out.write('/* << table name auto generated - check it! >> */\n')
  out.write('std::string %s::table_name = "%s";\n' % (capital_name, lowered_name))
  out.write('\n')
  out.write('\n')
  out.write('/* << model field definition should follow >> */\n')
  out.write('\n')
  out.write('%s::field_list %s::fields = list_of<%s::field_list::value_type>( /* << complete list! >> */ );\n' % (capital_name, capital_name, capital_name))
  out.write('\n')


def print_usage(parser):
  print('\nSimple database model skeleton code generator')
  parser.print_help()


import sys
from optparse import OptionParser

options = None

if __name__ == '__main__':
  usage = 'usage: %prog [options] <model-name>'
  parser = OptionParser(usage=usage)
  parser.add_option('-i', '--indent', help='number of indent spaces [default=%default]', dest='indent', type='int', default=2)
  parser.add_option('-f', '--filename', help='file name for output [default=auto-generated]', dest='filename', type='string')
  parser.add_option('--stdout', help='write output to stdout instead of file', dest='stdout', action='store_true', default=False)

  (options, args) = parser.parse_args()
  
  if len(args) < 1:
    print('ERROR: <model-name> not specified')
    print_usage(parser)
    sys.exit(1)

  if options.filename == None:
    options.filename = 'model_' + args[0]
  

  skeleton_header(options, args[0])
  skeleton_source(options, args[0])



