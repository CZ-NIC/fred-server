#!/bin/sh

#. ./config_vars

COLNAME="value_xml"
DBNAME="fred"
DBPORT=22345
ACTION_MINID=1
#ACTION_MAXID=5000
ACTION_MAXID=50

echo "timestamp0 - beginning: ";  date

# find initial ID for log_entry
LOG_MIN_ID=`psql -U $DBNAME -p $DBPORT -c "select nextval('log_entry_id_seq'::regclass)" | grep "^\s*[0-9]\+" `
if [ -z "$LOG_MIN_ID" ]; then
	echo Minimal log_entry id not found
	exit 1
fi
LOG_MIN_ID=$((LOG_MIN_ID + 1))

echo min log id: $LOG_MIN_ID


psql -U $DBNAME -p $DBPORT -f migrate_raw_tables.sql 

psql -U $DBNAME -p $DBPORT -c "select migrate_raw_data($ACTION_MINID, $ACTION_MAXID)"

## whatta use : \\pset pager \\\\ 
##raw migrate action to log_entry
#echo "1 -------------"
#echo "insert into log_entry (id, time_begin, time_end, source_ip, service, action_type, is_monitoring) select id, startdate, enddate, null, 3, action, false from action where id>=$ACTION_MINID and id<=$ACTION_MAXID" | psql -U $DBNAME -p $DBPORT 
#
##raw migrate action_xml to log_raw_content
## we can join either action or log_entry
#echo "2 -------------"
#echo "insert into log_raw_content (entry_time_begin, entry_id , content, is_response) select l.time_begin, actionid, xml, false from action_xml join log_entry l on l.id=actionid where actionid >= $ACTION_MINID and actionid<=$ACTION_MAXID" and xml is not null | psql -U $DBNAME -p $DBPORT
#
## this and ^ could be perhaps done in one statement :)
#echo "3 -------------"
#echo "insert into log_raw_content (entry_time_begin, entry_id , content, is_response) select l.time_begin, actionid, xml_out, true from action_xml join log_entry l on l.id=actionid where actionid >= $ACTION_MINID and actionid<=$ACTION_MAXID" and xml_out is not null | psql -U $DBNAME -p $DBPORT
#

echo "timestamp1 - finished direct table transfers ";  date

# only using insert_props, otherwise database functions

psql -U $DBNAME -p $DBPORT -c "select to_char(actionid, '99999999999999999999') || '|' || to_char(startdate, 'YYYY-MM-DD HH24:MI:SS.US') || '|' || replace(xml, E'\n', '') as $COLNAME from action_xml ax join action on id=ax.actionid where ax.actionid >= $ACTION_MINID and ax.actionid < $ACTION_MAXID order by actionid" | grep -v "^\-\+$\|^\s*$COLNAME\s*$\|([0-9]\+ rows)\|^\s*$"    |  ./migrate_log


echo "timestamp2 - finished processing with migrate_log ";  date

LOG_MAX_ID=`psql -U $DBNAME -p $DBPORT -c "select nextval('log_entry_id_seq'::regclass)" | grep "^\s*[0-9]\+"`
if [ -z "$LOG_MAX_ID" ]; then
	echo Maximal log_entry id not found
	exit 1
fi

LOG_MAX_ID=$((LOG_MAX_ID - 1))

echo max log id: $LOG_MAX_ID


#TODO - we don't need it right now
# psql -U $DBNAME -p $DBPORT -f update_log_entry.sql 
#echo "timestamp3";  date
#TODO - we don't need it right now
#echo "Calling update_log_entry with log ($LOG_MIN_ID, $LOG_MAX_ID) and action($ACTION_MINID, $ACTION_MAXID) "
#psql -U $DBNAME -p $DBPORT -c "select update_log_entry($LOG_MIN_ID, $LOG_MAX_ID, $ACTION_MINID, $ACTION_MAXID)"

echo "timestamp4 - finished ";  date

