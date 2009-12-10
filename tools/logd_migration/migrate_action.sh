#!/bin/sh

DBNAME="fred"
DBPORT=22345

DATE_LOW="2009-11-06 21:34:02.550372"
# DATE_HIGH="2009-12-09 10:35:33.628458"
DATE_HIGH="2009-12-10 10:27:54.105179"

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

psql -P tuples_only=on -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(time_begin, 'YYYY-MM-DD HH24:MI:SS.US') || '|' || is_monitoring::char || '|' || replace(xml, E'\n', '') from action_xml ax join request on id=ax.actionid where time_begin >= '$DATE_LOW' and time_begin <= '$DATE_HIGH' order by time_begin" |  ./migrate_log

# COLNAME="valuexxx"
# psql -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(startdate, 'YYYY-MM-DD HH24:MI:SS.US') || '|' || is_monitoring::char || '|' || replace(xml, E'\n', '') as $COLNAME from action_xml ax join action on id=ax.actionid where startdate >= '$DATE_LOW' and startdate <= '$DATE_HIGH' order by startdate" | grep -v "^\-\+$\|^\s*$COLNAME\s*$\|([0-9]\+ rows)\|^\s*$"    |  ./migrate_log


echo "timestamp2 - finished ";  date

