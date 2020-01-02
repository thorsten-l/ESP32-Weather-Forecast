#ifndef __DISPLAY_HANDLER__
#define __DISPLAY_HANDLER__

#define ENABLE_GxEPD2_GFX 0

#include "App.hpp"

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>

#include <DejaVuSansMono-Bold7pt8b.h>
#include <DejaVuSansMono-Bold24pt8b.h>
#include <DejaVuSansCondensed-Bold8pt8b.h>
#include <DejaVuSansCondensed-Bold24pt8b.h>
#include <DejaVuSans-Bold8pt8b.h>
#include <DejaVuSans-Bold10pt8b.h>
#include <DejaVuSans-Bold36pt8b.h>
#include <WeatherIconsR-Regular20pt8b.h>
#include <WeatherIconsR-Regular24pt8b.h>

#include <sunmoon.h>
#include <math.h>
#include <WeatherData.hpp>
#include <Adafruit_GFX.h>

extern GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display;

class DisplayHandler
{
private:
  void showClock(struct tm timeinfo, int x, int y);
  void showCalendar(time_t now, struct tm timeinfo, int x, int y);
  void showRiseSet( int x, int y, uint8_t icon, int hour, int minute );
  void showMoonPhase( int x, int y, struct SUN_MOON &sunmoon );
  void showCentered(int x, int y, int cwidth, const char *text);
  void showSunMoon(struct tm timeinfo);
  void showCurrentWeather();
  void showWeatherForecast3h();
  void showGrid();
  void showError();

  String getTimeString(time_t time);
  int zoneShift(int utc_hour, int zone);
  char *utf8ToIso8859( char *isobuffer, String utf8String );

public:
  void setup();
  void update();
};

extern DisplayHandler displayHandler;

#endif
