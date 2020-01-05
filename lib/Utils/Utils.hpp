#ifndef __UTILS_HPP__

#include <Arduino.h>

char *utf8ToIso8859( char *isobuffer, String utf8String );
String getTimeString(time_t time);
int zoneShift(int utc_hour, int zone);

#endif
