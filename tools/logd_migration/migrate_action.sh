#!/bin/sh

DBNAME="fred"
DBPORT=22345

# dates only apply to action & action_xml, not to login
# login is ALWAYS migrated completely
DATE_LOW="2006-07-23 15:44:19.755027"
DATE_HIGH="2011-04-02 15:40:41.666799"

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

echo "--- initialised transfer: "; date

time psql -U $DBNAME -p $DBPORT -c "select migrate_raw_data('$DATE_LOW'::timestamp without time zone, '$DATE_HIGH'::timestamp without time zone)"

echo "timestamp1 - finished direct table transfers ";  date

# only using insert_props, otherwise database functions

psql -P tuples_only=on -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(time_begin, 'YYYY-MM-DD HH24:MI:SS.US') || '|' || is_monitoring::char || '|' || replace(xml, E'\n', '') from action_xml ax join request on id=ax.actionid where time_begin >= '$DATE_LOW' and time_begin <= '$DATE_HIGH' order by time_begin" |  ./migrate_log

# COLNAME="valuexxx"
# psql -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(startdate, 'YYYY-MM-DD HH24:MI:SS.US') || '|' || is_monitoring::char || '|' || replace(xml, E'\n', '') as $COLNAME from action_xml ax join action on id=ax.actionid where startdate >= '$DATE_LOW' and startdate <= '$DATE_HIGH' order by startdate" | grep -v "^\-\+$\|^\s*$COLNAME\s*$\|([0-9]\+ rows)\|^\s*$"    |  ./migrate_log


echo "timestamp2 - finished ";  date

