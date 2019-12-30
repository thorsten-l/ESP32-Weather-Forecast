
#define ENABLE_GxEPD2_GFX 0

#include "App.hpp"
#include "DisplayHandler.hpp"

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <DejaVuSansMono-Bold7pt8b.h>
#include <DejaVuSansMono-Bold24pt8b.h>
#include <DejaVuSansCondensed-Bold24pt8b.h>
#include <DejaVuSans-Bold8pt8b.h>
#include <DejaVuSans-Bold10pt8b.h>
#include <DejaVuSansCondensed-Bold8pt8b.h>
#include <DejaVuSans-Bold36pt8b.h>

#include <WeatherIconsR-Regular20pt8b.h>
#include <WeatherIconsR-Regular24pt8b.h>

#include <sunmoon.h>
#include <math.h>
#include <WeatherData.hpp>

GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=*/15, /*DC=*/27, /*RST=*/26, /*BUSY=*/25));

#define CX 20
#define CY 16
#define CWIDTH 144

char *cAbDays[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
char *cFlDays[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
char *cFlMonth[] = {"Januar", "Februar", "M�rz", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 'Ä', 'Ö', 'Ü', 'ä', 'ö', 'ü', 'ß', '§'
uint16_t umlaute_utf8[] = { 0xc384, 0xc396, 0xc39c, 0xc3a4, 0xc3b6, 0xc3bc, 0xc39f, 0xc2a7 };
uint8_t umlaute_iso8859[] = { 0xc4, 0xd6, 0xdc, 0xe4, 0xf6, 0xfc, 0xdf, 0xa7 };

struct bitmap_pair
{
  const unsigned char *black;
  const unsigned char *red;
};

char *utf8ToIso8859( char *isobuffer, String utf8String )
{
  isobuffer[0] = 0;
  int bi = 0;

  for ( int i=0; i<utf8String.length(); i++ )
  {
    uint16_t c = (uint16_t)utf8String.charAt(i);
    
    if ( c == 0xc2 || c == 0xc3 )
    {
      i++;
      c <<= 8;
      c |= (uint16_t)utf8String.charAt(i);

      for( int j=0; j<8; j++)
      {
        if ( umlaute_utf8[j] == c )
        {
          c = umlaute_iso8859[j];
          break;
        }
      }
    }
    
    isobuffer[bi++] = c;
    isobuffer[bi] = 0;
  }

  return isobuffer;
}

void showCentered(int x, int y, int cwidth, const char *text)
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
  display.setFont(&DejaVuSans_Bold8pt8b);
  sprintf( buffer, "%02d:%02d", hour, minute );
  showCentered( x, y+18, 62, buffer );
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
  display.setFont(&DejaVuSans_Bold8pt8b);
  sprintf( buffer, "%d%%", (int)round(sunmoon.MoonPhaseNumber * 100));
  showCentered( x, y+18, 62, buffer );  
}


uint8_t findIcon( uint16_t icons[][2], uint16_t iconId )
{
  uint8_t icon = 0x20;

  for ( int i=0; i<61; i++ )
  {
    if ( icons[i][0] == iconId )
    {
      icon = (uint8_t)icons[i][1];
      break;
    }
  }

  return icon;
}

uint8_t getOWMicon( int weatherId, const char *iconName )
{
  uint8_t icon = findIcon( stdOWMIconMap, weatherId );

  char lastChar = 0;
  int i = 0;
  while( iconName[i] != 0 )
  {
    lastChar = iconName[i++];
  }

  if ( lastChar == 'd' ) 
  {
    display.setTextColor(GxEPD_RED);
    icon = findIcon( dayOWMIconMap, weatherId );
  }

  if ( lastChar == 'n' ) icon = findIcon( nightOWMIconMap, weatherId );

  return icon;
}

void showWeatherForecast3h()
{
  int x = 145;
  int y = 100;
  

  for ( int i = 0; i< 8; i++ )
  {
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DejaVuSansCondensed_Bold8pt8b);
    JsonObject obj = weatherForecast["list"][i];
    time_t dt = obj["dt"].as<long>();
    display.setCursor( x, y );
    showCentered( x, y, 62, getTimeString(dt).c_str() );

    char buffer[16];

    double temp = obj["main"]["temp"].as<double>();
    sprintf( buffer, "%0.1foC",  temp );
    buffer[strlen(buffer)-2] = 176; // ° Sign

    if( temp > 0.0 )
    {
      display.setTextColor(GxEPD_RED);
    }
    else
    {
      display.setTextColor(GxEPD_BLACK);
    }
    showCentered( x, y+68, 62, buffer );

    display.setTextColor(GxEPD_RED);
    temp = obj["main"]["temp_max"].as<double>();
    sprintf( buffer, "%0.1foC",  temp );
    buffer[strlen(buffer)-2] = 176; // ° Sign
    showCentered( x, y+150, 62, buffer );

    display.setTextColor(GxEPD_BLACK);
    temp = obj["main"]["temp_min"].as<double>();
    sprintf( buffer, "%0.1foC",  temp );
    buffer[strlen(buffer)-2] = 176; // ° Sign
    showCentered( x, y+170, 62, buffer );



    sprintf( buffer, "%d%%", obj["main"]["humidity"].as<int>() );
    display.setTextColor(GxEPD_BLACK);
    showCentered( x, y+88, 62, buffer );

    display.setFont(&WeatherIconsR_Regular20pt8b);

    double speed = obj["wind"]["speed"].as<double>();
    uint8_t w = 0;
    for ( ; w<12 && speed > windSpeeds[w]; w++ );

    buffer[0] = 0xd0 + w;
    buffer[1] = 0;
    showCentered( x, y+122, 62, buffer );

    buffer[0] = getOWMicon( obj["weather"][0]["id"].as<int>(), obj["weather"][0]["icon"].as<String>().c_str());
    buffer[1] = 0;
    showCentered( x, y+44, 62, buffer );

    x += 62;
  }
}

void showGrid()
{
  int y0 = 80;
  int y1 = 320;
  int x = 144;

  for( int i=x; i<640; i ++ )
  {
    if (( i % 2 ) == 0 )
    {
      display.drawPixel( i, 230, GxEPD_BLACK );
    }
  }

  display.drawLine( x, y0, 640, y0, GxEPD_BLACK );
  display.drawLine( x, y1, 640, y1, GxEPD_BLACK );

  for( int i = 1; i<=7; i++ )
  {
    display.drawLine( x + (i*62), y0, x + (i*62), y1, GxEPD_BLACK );
  }
}

void showCurrentWeather()
{
  char buffer[64];

  int x = 146;
  int y = 0;

  display.setFont(&WeatherIconsR_Regular24pt8b);
  display.setTextColor(GxEPD_BLACK);

  buffer[0] = getOWMicon( currentWeather["weather"][0]["id"].as<int>(), currentWeather["weather"][0]["icon"].as<String>().c_str());
  buffer[1] = 0;
  display.setCursor( x, y+50 );
  display.print( buffer );

  display.setFont(&DejaVuSansCondensed_Bold24pt8b);
  double temp = currentWeather["main"]["temp"].as<double>();
  if ( temp > 0 )
  {
    display.setTextColor(GxEPD_RED);
  }
  else
  {
    display.setTextColor(GxEPD_BLACK);
  }
  
  sprintf( buffer, " %0.1foC",  temp );
  buffer[strlen(buffer)-2] = 176; // ° Sign
  display.print( buffer );
  display.setTextColor(GxEPD_BLACK);
  sprintf( buffer, " %d%% ",  currentWeather["main"]["humidity"].as<int>() );
  display.print( buffer );

  display.setFont(&WeatherIconsR_Regular24pt8b);

  double speed = currentWeather["wind"]["speed"].as<double>();
  uint8_t w = 0;
  for ( ; w<12 && speed > windSpeeds[w]; w++ );

  buffer[0] = 0xd0 + w;
  buffer[1] = 0;
  display.print( buffer );

  // display.setTextColor(GxEPD_BLACK);
  display.setFont(&DejaVuSansCondensed_Bold8pt8b);
  display.setCursor( x, y+74 );
  display.print( utf8ToIso8859( buffer, currentWeather["weather"][0]["description"].as<String>()));

  display.setCursor( x+248, y+74 );
  display.setTextColor(GxEPD_RED);
  sprintf( buffer, "Max %0.1foC",  currentWeather["main"]["temp_max"].as<double>() );
  buffer[strlen(buffer)-2] = 176; // ° Sign
  display.print( buffer );
  display.setTextColor(GxEPD_BLACK);
  sprintf( buffer, "  Min %0.1foC",  currentWeather["main"]["temp_min"].as<double>() );
  buffer[strlen(buffer)-2] = 176; // ° Sign
  display.print( buffer );
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

    showCurrentWeather();
    showWeatherForecast3h();
    showGrid();

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
