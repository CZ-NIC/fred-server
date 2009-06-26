#!/bin/sh

#. ./config_vars

COLNAME="value_xml"
DBNAME="fred"
DBPORT=22345
DATE_LOW="2008-10-07 09:48:46.428756"
# DATE_HIGH="2008-10-08 15:00:10.493836"		# 50
DATE_HIGH="2009-04-20 14:40:03.533177"		# 50000

echo "timestamp0 - beginning: ";  date

db_notempty() 
{
	echo "The target database is not empty. Table: $1"
	exit 1
}

test_empty()
{
	psql -U $DBNAME -p $DBPORT -c "select count(*) from $1" | grep "\<0\>" > /dev/null  ||  db_notempty $1
}

# test if the target database is empty
for table in request request_property_value request_data session; do
	test_empty $table
done


psql -U $DBNAME -p $DBPORT -f migrate_raw_tables.sql 

psql -U $DBNAME -p $DBPORT -c "select migrate_raw_data('$DATE_LOW'::timestamp without time zone, '$DATE_HIGH'::timestamp without time zone)"

echo "timestamp1 - finished direct table transfers ";  date

# only using insert_props, otherwise database functions

psql -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(startdate, 'YYYY-MM-DD HH24:MI:SS.US') || '|03|' || replace(xml, E'\n', '') as $COLNAME from action_xml ax join action on id=ax.actionid where startdate >= '$DATE_LOW' and startdate < '$DATE_HIGH' order by startdate" | grep -v "^\-\+$\|^\s*$COLNAME\s*$\|([0-9]\+ rows)\|^\s*$"    |  ./migrate_log

echo "timestamp2 - finished ";  date

