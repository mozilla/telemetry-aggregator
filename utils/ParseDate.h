#ifndef PARSEDATE_H
#define PARSEDATE_H

#include <time.h>

/** Parse date on the format YYYYMMDD to local time */
time_t parseDate(const char* YYYYMMDD) {
  struct tm tinfo;
  tinfo.tm_yday  = 0;
  tinfo.tm_wday  = 0;
  tinfo.tm_isdst = -1; // Don't know or care about day light savings time
  tinfo.tm_year  = ((*(YYYYMMDD++)) - '0') * 1000;
  tinfo.tm_year += ((*(YYYYMMDD++)) - '0') * 100;
  tinfo.tm_year += ((*(YYYYMMDD++)) - '0') * 10;
  tinfo.tm_year += ((*(YYYYMMDD++)) - '0');
  tinfo.tm_mon   = ((*(YYYYMMDD++)) - '0') * 10;
  tinfo.tm_mon  += ((*(YYYYMMDD++)) - '0');
  tinfo.tm_mday  = ((*(YYYYMMDD++)) - '0') * 10;
  tinfo.tm_mday += ((*(YYYYMMDD++)) - '0');
  tinfo.tm_hour  = 0;
  tinfo.tm_min   = 0;
  tinfo.tm_sec   = 0;
  return mktime(&tinfo);
}

#endif // PARSEDATE_H