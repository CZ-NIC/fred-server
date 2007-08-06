#!/bin/sh
# actual build folder
BDIR=/tmp/build
REVFILE=/tmp/svn.revision
SCHEMAS=/etc/apache2/schemas/

clean_ccreg()
{
rm -fr $BDIR
mkdir $BDIR
cd $BDIR
}

# stop ccReg_server and apache
stop_ccreg()
{
/etc/init.d/apache2 stop
/etc/init.d/ccReg stop
}

# start ccReg_server and apache
start_ccreg()
{
/etc/init.d/ccReg start
# wait for start
sleep 5
/etc/init.d/apache2 start
}

# get source from SVN
get_ccreg()
{
# only download from SVN public what is need
svn co --non-interactive    svn+ssh://pblaha@public.nic.cz/svn/enum/trunk/cr/
svn co --non-interactive    svn+ssh://pblaha@public.nic.cz/svn/enum/trunk/mod_eppd
svn co --non-interactive    svn+ssh://pblaha@public.nic.cz/svn/enum/trunk/epp_client/ccReg/schemas
svn co --non-interactive    svn+ssh://pblaha@public.nic.cz/svn/enum/trunk/epp_client
}

build_ccreg()
{
cd $BDIR/cr/ccReg
make
cd $BDIR/mod_eppd
make
}

install_ccreg()
{
cd $BDIR/cr/ccReg
make install
cd $BDIR/mod_eppd
make install
rm -fr $SCHEMAS
mkdir $SCHEMAS
cp -vr $BDIR/schemas/*.xsd $SCHEMAS
}

unit_test()
{
cd $BDIR/epp_client
python  unitest_contact.py 
python  unitest_domain.py 
python  unitest_login.py
python  unitest_nsset.py
}

SVN_REVISION=`svn info  svn+ssh://pblaha@public.nic.cz/svn/enum/trunk/ | grep Revision | awk '{ print $2}'`
echo get SVN rev $SVN_REVISION 
OLD_REVISION=`cat $REVFILE`


if  test "$SVN_REVISION" !=  "$OLD_REVISION" ; then
echo $SVN_REVISION > $REVFILE
export SVN_REVISION
echo Start rebuild at SVN rev $SVN_REVISION old $OLD_REVISION
clean_ccreg
get_ccreg
build_ccreg
# restart and install
stop_ccreg
#install_ccreg
start_ccreg
#wait 10 sec
sleep 10
# run unit test
unit_test
fi



