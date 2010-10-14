#!/bin/bash
#sms sender parameters
# $LOGIN $PASSWORD
# $1 - phone number $2 - text message content
########################################################
#echo $1 $2

PHONE_NUMBER=`echo $1 |  tr -d ' .'` #remove spaces and dots
TEXTMESSAGE=$2

#adjust logfile name
LOGTEMPFILE=$(mktemp /tmp/smslog.XXXXXXXXXX) || { echo "Failed to create temp logfile"; exit 1; }

TEXTMESSAGE=$(python -c "import urllib; print urllib.quote('$TEXTMESSAGE');")
TEXTMESSAGEAUTHSUBSTR=${TEXTMESSAGE:0:31}
PASSWORDMD5=`echo $PASSWORD | openssl dgst -md5`
ACTION=send
AUTHTEXT=$PASSWORDMD5$LOGIN$ACTION$TEXTMESSAGEAUTHSUBSTR
AUTHTEXTMD5=`echo $AUTHTEXT | openssl dgst -md5`
REQUEST_DATA=' -d login='$LOGIN' -d auth='$AUTHTEXTMD5' --data-urlencode msisdn='$PHONE_NUMBER' -d msg='$TEXTMESSAGE' -d recack=1'
echo curl -k $REQUEST_DATA  https://mobilem.cz/api/xmlapi2.xp > $LOGTEMPFILE

#debug
#CURL_RESULT=`echo status error`

CURL_RESULT=`curl -k $REQUEST_DATA https://mobilem.cz/api/xmlapi2.xp`

echo $CURL_RESULT >> $LOGTEMPFILE
echo $CURL_RESULT | grep status | grep ok && exit 0
exit  1
