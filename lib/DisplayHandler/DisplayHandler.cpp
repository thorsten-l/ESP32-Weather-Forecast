
#define ENABLE_GxEPD2_GFX 0

#include "App.hpp"
#include "DisplayHandler.hpp"

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <DejaVuSansMono-Bold7pt8b.h>
#include <DejaVuSansMono-Bold24pt8b.h>
#include <DejaVuSans-Bold8pt8b.h>
#include <DejaVuSans-Bold10pt8b.h>
#include <DejaVuSans-Bold36pt8b.h>

#include <WeatherIconsR-Regular20pt8b.h>

#include <sunmoon.h>
#include <math.h>

GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=*/15, /*DC=*/27, /*RST=*/26, /*BUSY=*/25));

#define CX 20
#define CY 16
#define CWIDTH 144

char *cAbDays[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
char *cFlDays[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
char *cFlMonth[] = {"Januar", "Februar", "M�rz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct bitmap_pair
{
  const unsigned char *black;
  const unsigned char *red;
};

void showCentered(int x, int y, int cwidth, char *text)
{
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(text,
                        (uint16_t)x, (uint16_t)y, &x1, &y1, &w, &h);
  display.setCursor((x - 1) + ((cwidth - w) / 2), y);
  display.print(text);
}

void showClock(struct tm timeinfo, int x, int y)
{
  display.setFont(&DejaVuSans_Bold8pt8b);
  display.setCursor(x, y);
  display.print("Aktualisierung");
  display.setFont(&DejaVuSansMono_Bold24pt8b);
  display.setCursor(x, y + 45);
  display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
}

void showCalendar(time_t now, struct tm timeinfo, int x, int y)
{
  char buffer[32];
  int day = timeinfo.tm_mday;
  char *dayName = cFlDays[timeinfo.tm_wday];
  char *monthName = cFlMonth[timeinfo.tm_mon];

  int noDays = daysOfMonth[timeinfo.tm_mon];
  int year = timeinfo.tm_year + 1900;

  if (timeinfo.tm_mon == 1 && timeinfo.tm_year % 4 == 0)
  {
    noDays = 29;
  }

  time_t firstDayOfMonth = now - ((timeinfo.tm_mday - 1) * 86400);
  localtime_r(&firstDayOfMonth, &timeinfo);

  int firstDay = (timeinfo.tm_wday + 6) % 7;

  y += 22;
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&DejaVuSans_Bold10pt8b);
  showCentered(x, y, CWIDTH, dayName);

  y += 68;
  display.setFont(&DejaVuSans_Bold36pt8b);
  sprintf(buffer, "%02d", day);
  showCentered(x, y, CWIDTH, buffer);

  y += 32;
  display.setFont(&DejaVuSans_Bold10pt8b);
  showCentered(x, y, CWIDTH, monthName);

  y += 32;
  sprintf(buffer, "%02d", year);
  showCentered(x, y, CWIDTH, buffer);

  y += 32;
  display.setFont(&DejaVuSansMono_Bold7pt8b);
  for (int i = 0; i < 7; i++)
  {
    display.setCursor(x + (i * CX), y);
    display.print(cAbDays[i]);
  }

  y += 4;
  display.drawLine(0, y, CWIDTH, y, GxEPD_BLACK);

  y += CY - 1;
  for (int i = 0; i < noDays; i++)
  {
    display.setCursor(x + ((((i + firstDay) % 7) * CX)), y + ((((i + firstDay) / 7) * CY)));

    if (i + 1 == day)
    {
      display.setTextColor(GxEPD_RED);
      display.printf("%02d", (i + 1));
      display.setTextColor(GxEPD_BLACK);
    }
    else
    {
      display.printf("%02d", (i + 1));
    }
  }
}

String getTimeString(time_t time)
{
  struct tm timeinfo;
  char buffer[6];
  localtime_r(&time, &timeinfo);
  sprintf(buffer, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(buffer);
}

int zoneShift(int utc_hour, int zone)
{
  return (utc_hour + zone) % 24;
}

void showRiseSet( int x, int y, uint8_t icon, int hour, int minute )
{
  char buffer[6];
  display.setCursor( x, y );
  display.setFont(&WeatherIconsR_Regular20pt8b);
  buffer[0] = icon;
  buffer[1] = 0;
  showCentered( x, y, 62, buffer );
  display.setFont(&DejaVuSansMono_Bold7pt8b);
  sprintf( buffer, "%02d:%02d", hour, minute );
  showCentered( x, y+16, 62, buffer );
}

void showMoonPhase( int x, int y, struct SUN_MOON &sunmoon )
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

  Serial.printf( "icon=%02x\n", icon );

  display.setCursor( x, y );
  display.setFont(&WeatherIconsR_Regular20pt8b);
  buffer[0] = icon;
  buffer[1] = 0;
  showCentered( x, y, 62, buffer );
  display.setFont(&DejaVuSansMono_Bold7pt8b);
  sprintf( buffer, "%d%%", (int)round(sunmoon.MoonPhaseNumber * 100));
  showCentered( x, y+16, 62, buffer );  
}

void displayUpdate()
{
  struct tm timeinfo;
  time_t now = time(0);
  // now += 45 * 86400;
  localtime_r(&now, &timeinfo);

  Serial.print("\n time = ");
  Serial.println(getTimeString(1575838661));

  struct SUN_MOON sunmoon;
  int zone = 1;
  sun_moon( HOME_LATITUDE, HOME_LONGITUDE, &timeinfo, 0, 0, &sunmoon);

  Serial.printf("\n\nmoon phase number = %d%%\n", (int)round(sunmoon.MoonPhaseNumber * 100));
  Serial.printf("moon phase = %d\n", sunmoon.MoonPhase);
  Serial.printf("moon rise = %02d:%02d\n", zoneShift(sunmoon.MoonRise.hh, zone), sunmoon.MoonRise.mm);
  Serial.printf("moon set = %02d:%02d\n", zoneShift(sunmoon.MoonSet.hh, zone), sunmoon.MoonSet.mm);
  Serial.printf("sun rise = %02d:%02d\n", zoneShift(sunmoon.SunRise.hh, zone), sunmoon.SunRise.mm);
  Serial.printf("sun set = %02d:%02d\n", zoneShift(sunmoon.SunSet.hh, zone), sunmoon.SunSet.mm);

  display.setFullWindow();
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  do
  {
    showCalendar(now, timeinfo, 2, 0);
    showClock(timeinfo, 2, 315);

    display.drawLine( 144, 0, 144, 384, GxEPD_BLACK );

    int y = 360;
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

  } while (display.nextPage());

  display.powerOff();
  delay(200);
  SPI.end();
  Serial.println("display update done.");
}

void displaySetup()
{
  Serial.println();
  Serial.println("display setup");

  display.init();
  SPI.end();
  SPI.begin(13, 12, 14, 15);

  Serial.printf("has partial update = %s\n", (display.epd2.hasPartialUpdate) ? "true" : "false");
  Serial.printf("has fast partial update = %s\n", (display.epd2.hasFastPartialUpdate) ? "true" : "false");
  Serial.println("display setup done");
}
