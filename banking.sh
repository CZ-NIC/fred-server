#!/bin/sh
# run e-banka list and make invoice
# get 
wget -O ebanka.csv --no-check-certificate  "https://klient2.ebanka.cz/ebts/owa/shop.getpayments?shopname=CZNIC756&creditaccount=756&creditbank=2400&password=kL23Em11eNT&listtype=PLAIN"
# transfer to UTF8
iconv --from-code=WINDOWS-1250 --to-code=UTF8 < ebanka.csv > e.csv
# run e-banka config file /ect/ccReg.conf
# log to syslog
./banking  --ebanka-csv e.csv

