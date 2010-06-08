#!/bin/bash
#hybrid postservice mail upload test
echo
echo "#login start#"
echo
curl -F "cc=us \n" -F "loginame=dreplech" -F "password=dreplech" \
-F "standzak=hpcb_Jednorazova_zakazka" -F "poznamka=Testovaci prenos!!!" \
-F "jobzak=2010" -F "verzeOS=Linux" -F "verzeProg=20100315001" -b ./tmpdir/phpcookie \
-A '' --cacert ./cert/postsignum_qca_root.pem -i \
-v  https://online.postservis.cz/Command/over.php 2>&1 1> ./tmpdir/dump.txt

echo
echo "#login end#"
echo

echo
echo "#upload start#"
echo

curl -F "cc=us \n" -F "pocetupl=1\n" -F "crc=C74E16D9" \
-F "upfile=@./tmpdir/test.txt" -b ./tmpdir/dump.txt \
-H 'User-Agent: CommandLine klient HP' --cacert ./cert/postsignum_qca_root.pem \
-v https://online.postservis.cz/Command/command.php

echo
echo "#upload end#"
echo

echo
echo "#konec start#"
echo

curl -F "cc=us \n" -F "soubor=./tmpdir/test.txt\n" -F "pocetsouboru=1\n" \
-F "stav=OK\n" -b ./tmpdir/dump.txt \
-H 'User-Agent: CommandLine klient HP' --cacert ./cert/postsignum_qca_root.pem \
-v https://online.postservis.cz/Command/konec.php

echo
echo "#upload end#"
echo
