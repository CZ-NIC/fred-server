#!/bin/bash
# sms sender parameters
# $LOGIN $PASSWORD
# $1 - phone number $2 - text message content
#
# dependancies : openssl, python, curl
########################################################
# echo $1 $2

PHONE_NUMBER=`echo $1 |  tr -d ' .'` #remove spaces and dots
TEXTMESSAGE=$2
URL=http://api.mobilem.cz/xmlapi2.xp
#URL=https://api.mobilem.cz/xmlapi2.xp

# adjust logfile name
#LOGTEMPFILE=$(mktemp /tmp/smslog.XXXXXXXXXX) || { echo "Failed to create temp logfile"; exit 1; }
if [ -z "$LOGFILE" ]; then
    LOGFILE=/tmp/fred_sms_tomato.log
fi

echo "-=-=-=-=-=-=-=-= $(date) =-=-=-=-=-=-=-=-" >> $LOGFILE
# prepare request data
TEXTMESSAGEAUTHSUBSTR=${TEXTMESSAGE:0:31}
PASSWORDMD5=`echo -ne $PASSWORD | openssl dgst -md5 | cut -d' ' -f2`
ACTION=send
AUTHTEXT=$PASSWORDMD5$LOGIN$ACTION$TEXTMESSAGEAUTHSUBSTR
AUTHTEXTMD5=`echo -ne $AUTHTEXT | openssl dgst -md5 | cut -d' ' -f2`
TEXTMESSAGE2=$(python -c "import urllib; print urllib.quote('$TEXTMESSAGE');")
REQUEST_DATA=" -d action=$ACTION -d login=$LOGIN -d auth=$AUTHTEXTMD5 --data-urlencode msisdn=$PHONE_NUMBER -d msg=$TEXTMESSAGE2 -d recack=1 -d sendermsisdn=mojeID"

# log and process request
echo "QUERY" >> $LOGFILE
echo "curl -k $REQUEST_DATA  $URL" >> $LOGFILE
CURL_RESULT=`curl $REQUEST_DATA $URL 2>/dev/null`

echo "RESPONSE" >> $LOGFILE
echo $CURL_RESULT >> $LOGFILE
echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" >> $LOGFILE
echo $CURL_RESULT | grep "status='ok'" >/dev/null && exit 0
exit  1
