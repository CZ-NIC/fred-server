#!/bin/sh

#. ./config_vars

COLNAME="value_xml"
DBNAME="fred"
DBPORT=22345
ACTION_MINID=1
ACTION_MAXID=5000

echo "timestamp1";  date

# find initial ID for log_entry
LOG_MIN_ID=`psql -U $DBNAME -p $DBPORT -c "select nextval('log_entry_id_seq'::regclass)" | grep "^\s*[0-9]\+" `
if [ -z "$LOG_MIN_ID" ]; then
	echo Minimal log_entry id not found
	exit 1
fi
LOG_MIN_ID=$((LOG_MIN_ID + 1))

echo min log id: $LOG_MIN_ID


# when changing this, also change INPUT_DATE_LENGTH constant in migrate.cc
psql -U $DBNAME -p $DBPORT -c "select to_char(startdate, 'YYMMDD\"T\"HH24MISS.US') || '|' || replace(xml, E'\n', '') as $COLNAME from action_xml join action on id=actionid where actionid >= $ACTION_MINID and actionid <= $ACTION_MAXID order by actionid" | grep -v "^\-\+$\|^\s*$COLNAME\s*$\|([0-9]\+ rows)\|^\s*$"    |  ./migrate_log

echo "timestamp2";  date

LOG_MAX_ID=`psql -U $DBNAME -p $DBPORT -c "select nextval('log_entry_id_seq'::regclass)" | grep "^\s*[0-9]\+" `
if [ -z "$LOG_MAX_ID" ]; then
	echo Maximal log_entry id not found
	exit 1
fi

LOG_MAX_ID=$((LOG_MAX_ID - 1))

echo max log id: $LOG_MAX_ID


psql -U $DBNAME -p $DBPORT -f update_log_entry.sql 

echo "timestamp3";  date

echo "Calling update_log_entry with log ($LOG_MIN_ID, $LOG_MAX_ID) and action($ACTION_MINID, $ACTION_MAXID) "

psql -U $DBNAME -p $DBPORT -c "select update_log_entry($LOG_MIN_ID, $LOG_MAX_ID, $ACTION_MINID, $ACTION_MAXID)"

echo "timestamp4";  date

