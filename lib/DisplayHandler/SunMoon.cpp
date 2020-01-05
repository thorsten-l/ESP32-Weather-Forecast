#include "DisplayHandler.hpp"

void DisplayHandler::showRiseSet( int x, int y, uint8_t icon, int hour, int minute )
{
  char buffer[6];
  display.setCursor( x, y );
  display.setFont(&WeatherIcons_SunMoonWind20pt7b);
  buffer[0] = icon;
  buffer[1] = 0;
  showCentered( x, y, 62, buffer );
  display.setFont(&DejaVuSans_Bold8pt8b);
  sprintf( buffer, "%02d:%02d", hour, minute );
  showCentered( x, y+18, 62, buffer );
}

void DisplayHandler::showMoonPhase( int x, int y, struct SUN_MOON &sunmoon )
{
  char buffer[6];
  uint8_t icon = OWM_SUNRISE;
  display.setTextColor(GxEPD_BLACK);
   
  switch( sunmoon.MoonPhase )
  {
    case LP_NEW_MOON:
      icon = OWM_NEWMOON;
      break;
    
    case LP_FULL_MOON:
      display.setTextColor(GxEPD_RED);
      icon = OWM_FULLMOON;
      break;
    
    case LP_WAXING_CRESCENT_MOON:
    case LP_FIRST_QUARTER_MOON:
    case LP_WAXING_GIBBOUS_MOON:
      display.setTextColor(GxEPD_RED);
      icon = OWM_MOONPHASES_WAXING;
      icon += (uint8_t)round(13.0 * sunmoon.MoonPhaseNumber);
      break;

    case LP_WANING_GIBBOUS_MOON:
    case LP_LAST_QUARTER_MOON:
    case LP_WANING_CRESCENT_MOON:
      icon = OWM_MOONPHASES_WANING;
      icon += 13 - ((uint8_t)round(13.0 * sunmoon.MoonPhaseNumber));
      break;
  }

  display.setCursor( x, y );
  display.setFont(&WeatherIcons_SunMoonWind20pt7b);
  buffer[0] = icon;
  buffer[1] = 0;
  showCentered( x, y, 62, buffer );
  display.setFont(&DejaVuSans_Bold8pt8b);
  sprintf( buffer, "%d%%", (int)round(sunmoon.MoonPhaseNumber * 100));
  showCentered( x, y+18, 62, buffer );  
}

void DisplayHandler::showSunMoon(struct tm timeinfo)
{
  struct SUN_MOON sunmoon;
  int zone = 1;
  sun_moon( HOME_LATITUDE, HOME_LONGITUDE, &timeinfo, 0, 0, &sunmoon);

  display.drawLine( 144, 0, 144, 384, GxEPD_BLACK );
  int y = 364;
  int x = 145;
  display.setTextColor(GxEPD_RED);
  showRiseSet( x, y, OWM_SUNRISE, zoneShift(sunmoon.SunRise.hh, zone), sunmoon.SunRise.mm );
  x += 62;
  display.setTextColor(GxEPD_BLACK);
  showRiseSet( x, y, OWM_SUNSET, zoneShift(sunmoon.SunSet.hh, zone), sunmoon.SunSet.mm );
  x += 62;
  display.setTextColor(GxEPD_RED);
  showRiseSet( x, y, OWM_MOONRISE, zoneShift(sunmoon.MoonRise.hh, zone), sunmoon.MoonRise.mm );
  x += 62;
  display.setTextColor(GxEPD_BLACK);
  showRiseSet( x, y, OWM_MOONSET, zoneShift(sunmoon.MoonSet.hh, zone), sunmoon.MoonSet.mm );
  x += 62;
  showMoonPhase( x, y, sunmoon );
}

