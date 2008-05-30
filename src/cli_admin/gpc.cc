/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpc.h"
#include "old_utils/log.h"

void convert_date(
  char *dateStr, const char *date)
{
  snprintf(dateStr, MAX_DATE_STR, "20%c%c-%c%c-%c%c", date[4], date[5],
      date[2], date[3], date[0], date[1]);
}

int CopyStr(
  char *dst, char *src, int from, int len, bool erase)
{
  int l;

  if (erase) {
    // no nulls at the beginnig of string
    for (l = 0; l < len -1; l ++) {
      if (src[from+l] > '0')
        break;
    }

  } else
    l = 0;

  strncpy(dst, src+from+l, len -l);
  dst[len-l] =0;

  return from + len;
}

int NonZeroCopyString(
  char *dst, char *src, int len)
{
  return CopyStr(dst, src, 0, len, true);
}

int CopyString(
  char *dst, char *src, int from, int len)
{
  // delete nulls at the beginning 
  return CopyStr(dst, src, from, len, false);
}

void GPC::GetHead(
  ST_Head *head)
{
  // client account number without starting zeros at the beginning 
  NonZeroCopyString(head->account, gpc_head.accountNumber, GPC_MAX_ACCOUNT);

  strncpy(head->name, gpc_head.accountName, GPC_MAX_NAME); // client name
  head->oldBalnce = atol(gpc_head.oldBalance) ; // old balances
  head->newBalance = atol(gpc_head.newBalance); // new balances
  head->credit = atol(gpc_head.credit); // credit turn
  head->debet = atol(gpc_head.debet); // debit turn
  convert_date(head->oldDate, gpc_head.dateOldBalance); // old balances date 
  convert_date(head->date, gpc_head.dateList); // statement date
  head->num = gpc_head.num; // statement order
}

void GPC::GetItem(
  ST_Item *item)
{
  // without starting zeros
  NonZeroCopyString(item->account, gpc_item[recItem].accountOther,
      GPC_MAX_ACCOUNT);

  strncpy(item->bank, gpc_item[recItem].konstSymbol+2, MAX_CODE-1); // bank code against account from KS 
  item->bank[MAX_CODE-1] = 0;

  strncpy(item->ks, gpc_item[recItem].konstSymbol+6, MAX_KS-1); // bank code against account from KS 
  item->ks[MAX_KS-1] = 0;

  strncpy(item->vs, gpc_item[recItem].varSymbol, MAX_VS-1);
  item->vs[MAX_VS-1] = 0;
  strncpy(item->ss, gpc_item[recItem].specSymbol, MAX_SS-1);
  item->ss[MAX_SS-1] = 0;

  strncpy(item->memo, gpc_item[recItem].memo, MAX_MEMO-1);
  item->memo[MAX_MEMO-1] = 0;
  strncpy(item->evid, gpc_item[recItem].evidNum, MAX_MEMO);
  item->evid[MAX_MEMO-1] = 0;

  // convert date
  convert_date(item->date, gpc_item[recItem].date);
  // price in pennies
  item->price = atol(gpc_item[recItem].price);

  if (gpc_item[numrecItem].accountCode == '1')
    item->code = DEBET;
  else if (gpc_item[numrecItem].accountCode == '2')
    item->code = CREDIT;
  else
    item->code =0;

}

void GPC::ParseItem(
  char *tmp)
{
  int seek;

  seek =3;

  seek = CopyString(gpc_item[numrecItem].accountNumber, tmp, seek,
      GPC_MAX_ACCOUNT);

  seek= CopyString(gpc_item[numrecItem].accountOther, tmp, seek,
      GPC_MAX_ACCOUNT);

  seek= CopyString(gpc_item[numrecItem].evidNum, tmp, seek, GPC_MAX_EVID);

  seek= CopyString(gpc_item[numrecItem].price, tmp, seek, GPC_MAX_PRICE);

  gpc_item[numrecItem].accountCode = tmp[seek]; // account code 1 debit 2 credit ( 3 , 4 cancel )
  seek ++;

  seek= CopyString(gpc_item[numrecItem].varSymbol, tmp, seek, GPC_MAX_SYMBOL);

  seek= CopyString(gpc_item[numrecItem].konstSymbol, tmp, seek, GPC_MAX_SYMBOL);
  seek= CopyString(gpc_item[numrecItem].specSymbol, tmp, seek, GPC_MAX_SYMBOL);

  seek= CopyString(gpc_item[numrecItem].dateValuta, tmp, seek, GPC_MAX_DATE);

  seek= CopyString(gpc_item[numrecItem].memo, tmp, seek, GPC_MAX_MEMO);

  gpc_item[numrecItem].changeCode = tmp[seek];
  seek ++;

  strncpy(gpc_item[numrecItem].dataType, tmp+seek, 4);
  seek += 4;

  seek= CopyString(gpc_item[numrecItem].date, tmp, seek, GPC_MAX_DATE);

#ifdef DEBUG
  debug("cislo protiuctu: %s\n" , gpc_item[numrecItem].accountOther );
  debug("cislo dokladu  : %s\n" , gpc_item[numrecItem].evidNum );
  debug("castka  %s kod zauctovani[%c]\n" , gpc_item[numrecItem].price , gpc_item[numrecItem].accountCode );
  debug("VS [%s] KS [%s] SS[%s]\n" , gpc_item[numrecItem].varSymbol , gpc_item[numrecItem].konstSymbol , gpc_item[numrecItem].specSymbol );
  debug("valuta date [%s]\n" , gpc_item[numrecItem].dateValuta );
  debug("doplnujici udaj %s\n" , gpc_item[numrecItem].memo );

  debug("kod zmeny [%c]  druh dat[%c%c%c%c] \n" , gpc_item[numrecItem].changeCode ,
      gpc_item[numrecItem].dataType[0] , gpc_item[numrecItem].dataType[1] , gpc_item[numrecItem].dataType[2] , gpc_item[numrecItem].dataType[3] );
  debug("datum zuctovani [%s]\n" , gpc_item[numrecItem].date );
#endif

  numrecItem++;

}

void GPC::ParseHead(
  char *tmp)
{
  char numStr[4];
  int seek;

  seek =3;

  seek= CopyString(gpc_head.accountNumber, tmp, seek, GPC_MAX_ACCOUNT);

  seek= CopyString(gpc_head.accountName, tmp, seek, GPC_MAX_NAME);

  seek= CopyString(gpc_head.dateOldBalance, tmp, seek, GPC_MAX_DATE);

  seek= CopyString(gpc_head.oldBalance, tmp, seek, GPC_MAX_BALANCE);
  gpc_head.oldBalanceSign = tmp[seek];
  seek ++;

  seek= CopyString(gpc_head.newBalance, tmp, seek, GPC_MAX_BALANCE);
  gpc_head.newBalanceSign = tmp[seek];
  seek ++;

  seek= CopyString(gpc_head.debet, tmp, seek, GPC_MAX_BALANCE);
  gpc_head.debetSign = tmp[seek];
  seek ++;

  seek= CopyString(gpc_head.credit, tmp, seek, GPC_MAX_BALANCE);
  gpc_head.creditSign = tmp[seek];
  seek ++;

  seek= CopyString(numStr, tmp, seek, 3);
  gpc_head.num = atoi(numStr); // ordinal number of statement

  seek = CopyString(gpc_head.dateList, tmp, seek, GPC_MAX_DATE);

#ifdef DEBUG
  debug("cislo uctu klienta: %s\n" , gpc_head.accountNumber );
  debug("nazev uctu klienta: %s\n" , gpc_head.accountName );
  debug("stary zustatek: [%c]%s datum [%s]\n" , gpc_head.oldBalanceSign , gpc_head.oldBalance , gpc_head.dateOldBalance );
  debug("novy zustatek: [%c]%s\n" , gpc_head.newBalanceSign , gpc_head.newBalance );
  debug("kreditni obrat: [%c]%s\n" , gpc_head.creditSign , gpc_head.credit );
  debug("debetni obrat: [%c]%s\n" , gpc_head.debetSign , gpc_head.debet );
  debug("datum vypisu  [%s] poradi %d\n" , gpc_head.dateList , gpc_head.num );
#endif

}

#define TMP_SIZE GPC_MAX_RECORD+3

int GPC::ReadGPCFile(
  char * filename)
{
  FILE *fd;
  int numrec;
  char tmp[TMP_SIZE];

  if ((fd = fopen(filename, "r")) == NULL) {
    if (errno) {
      strerror_r(errno, tmp, sizeof(tmp));
      LOG(LOG_ERR, "GPC read failed: %s");
    } else {
      LOG(LOG_ERR, "GPC read failed: file not found");
    }
    return -1; // open error
  }

  // vycistime si pro jistotu buffer
  bzero(tmp, TMP_SIZE);

  // nacteme prvni řádku
  if (fgets(tmp, TMP_SIZE, fd) == NULL) {
    if (errno) {
      strerror_r(errno, tmp, sizeof(tmp));
      LOG(LOG_ERR, "GPC read failed: %s");
    } else {
      LOG(LOG_ERR, "GPC read failed: file empty");
    }
    fclose(fd);
    return -2; // read error
  }

  if (strncmp(tmp, GPC_LIST_HEAD, 3) != 0) {
    LOG(LOG_ERR, "GPC read failed: header not found");
    fclose(fd);
    return -3;
  }
  ParseHead(tmp);
  LOG(LOG_DEBUG, "GPC read head: %s", tmp);

  numrec = 0;
  do {
    // vycistime si pro jistotu buffer
    bzero(tmp, TMP_SIZE);
    if (fgets(tmp, TMP_SIZE, fd) == NULL) {
      if (errno) {
        strerror_r(errno, tmp, sizeof(tmp));
        LOG(LOG_ERR, "GPC read failed: %s", tmp);
        numrec = -2;
      }
    }

    if (((tmp[0] == '\r') && (tmp[1] == '\n')) || (strlen(tmp) == 0)) {
      // final newline or EOF
      break;
    }

    if (strlen(tmp) != 130) {
      LOG(LOG_ERR, "GPC read failed: invalid line len = %l", strlen(tmp));
      numrec = -3;
      break;
    }

    if (strncmp(tmp, GPC_LIST_ITEM, 3) == 0) {
      ParseItem(tmp);
      numrec ++;
      LOG(LOG_DEBUG, "GPC read item: %s", tmp);
    } else if (strncmp(tmp, GPC_LIST_VZP, 3) == 0) {
      // skip this line
    } else if (strncmp(tmp, "078", 3) == 0) {
      // skip this line
    } else if (strncmp(tmp, GPC_LIST_HEAD, 3) == 0) {
      // next account => finish
      break;
    } else {
      // unknown line
      LOG(LOG_ERR, "GPC read failed: unknown line: %s", tmp);
      numrec = -5;
      break;
    }
  } while (numrec < MAX_ITEMS);

  if (numrec == MAX_ITEMS) {
    LOG(LOG_ERR, "GPC read failed: too much lines = %l", numrec);
    // too much lines
    numrec = -4;
  }

  fclose(fd);
  LOG(LOG_DEBUG, "GPC read end: number of items: %d", numrec);
  return numrec;
}

