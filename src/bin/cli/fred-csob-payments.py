#!/usr/bin/python
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

import imaplib, sys, xml.dom.minidom
from email.FeedParser import FeedParser
imaphost='mail.nic.cz'
imapuser='fred-banka@nic.cz'
imappass='rvAAQ6jK'
fromaddress='Administrator@TBS.CSOB.CZ'
ucet= '188208275'
ucet2= '210345314'
server = imaplib.IMAP4(imaphost)
server.login(imapuser, imappass)
server.select()
r, data = server.search(None, '((FROM %s) (SINCE 1-Feb-2009) (SUBJECT "pis z") (OR (TEXT %s) (TEXT %s)))' % (fromaddress,ucet,ucet2))
for msgid in data[0].split(' '):
    (r, data) = server.fetch(msgid, '(RFC822)')
    maildata = data[0][1]
    # separate attachment in which we are interested
    fp = FeedParser()
    fp.feed(maildata)
    mail = fp.close()
    if not mail.is_multipart():
        sys.exit(-2);
    i = 1
    for part in mail.walk():
        if i == 3: break
        i += 1
    octets = part.get_payload(decode=True)
    dom = xml.dom.minidom.parseString(octets)
    ucetElement = dom.getElementsByTagName('S25_CISLO_UCTU')[0]
    ucet = ucetElement.childNodes[0].data
    (ucet_cislo, ucet_banka) = ucet.split('/')
    for node in dom.getElementsByTagName('FINSTA05'):
        for element in node.childNodes:            
            if element.nodeName == 'S28_POR_CISLO':
                ident = element.childNodes[0].data
            if element.nodeName == 'DPROCD':
                datetime = str(element.childNodes[0].data)
            if element.nodeName == 'S61_CASTKA':
                price = element.childNodes[0].data.replace(',','.')
            if element.nodeName == 'PART_BANK_ID':
                accbank = element.childNodes[0].data                
            if element.nodeName == 'S86_KONSTSYM':
                if element.childNodes:
                    konstsym = element.childNodes[0].data
                else:
                    konstsym = ""
            if element.nodeName == 'PART_ACCNO':
                if element.childNodes:
                    accno = element.childNodes[0].data
                else:
                    accno = ""
            if element.nodeName == 'PART_ACC_ID':
                if element.childNodes:
                    name = element.childNodes[0].data
                else:
                    name = ""
            if element.nodeName == 'S86_VARSYMPAR':
                varsym = element.childNodes[0].data
        if float(varsym) == 0 or float(price)<0:
            continue
        print "1;1;1;CZK;%s;%s 12:00:00;%s;%s;%s;%s;%s;%s;%s;2;%s;%s" % (price.replace('+','').replace('.',','),datetime,accno,accbank,ucet_cislo,ucet_banka,varsym,konstsym,name,name,ident)
server.close()
server.logout()

