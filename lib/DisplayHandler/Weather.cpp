#include "DisplayHandler.hpp"

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

void DisplayHandler::showWeatherForecast3h()
{
  int x = 145;
  int y = 120;
  
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
    showCentered( x, y+76, 62, buffer );

    sprintf( buffer, "%d%%", obj["main"]["humidity"].as<int>() );
    display.setTextColor(GxEPD_BLACK);
    showCentered( x, y+96, 62, buffer );

    display.setFont(&WeatherIconsR_Regular20pt8b);

    double speed = obj["wind"]["speed"].as<double>();
    uint8_t w = 0;
    for ( ; w<12 && speed > windSpeeds[w]; w++ );

    buffer[0] = 0xd0 + w;
    buffer[1] = 0;
    showCentered( x, y+136, 62, buffer );

    int windDir = obj["wind"]["deg"].as<int>();
    int wd = windDir / 45;
    wd *= 45;

    w = 0;
    for( ; w<8; w++ )
    {
      if ( windDirection[w].degree == wd )
      {
        break;
      }
    }

    if ( w < 8 )
    {
      buffer[0] = windDirection[w].icon;
      buffer[1] = 0;
      showCentered( x, y+172, 62, buffer );
    }

    display.setFont(&DejaVuSans_Bold8pt8b);
    showCentered( x, y+192, 62, windDirection[w].description );

    display.setFont(&WeatherIconsR_Regular20pt8b);
    buffer[0] = getOWMicon( obj["weather"][0]["id"].as<int>(), obj["weather"][0]["icon"].as<String>().c_str());
    buffer[1] = 0;
    showCentered( x, y+48, 62, buffer );

    x += 62;
  }
}

void DisplayHandler::showGrid()
{
  int y0 = 100;
  int y1 = 320;
  int x = 144;

  for( int i=x; i<640; i ++ )
  {
    if (( i % 2 ) == 0 )
    {
      display.drawPixel( i, 224, GxEPD_BLACK );
    }
  }

  display.drawLine( x, y0, 640, y0, GxEPD_BLACK );
  display.drawLine( x, y1, 640, y1, GxEPD_BLACK );

  for( int i = 1; i<=7; i++ )
  {
    display.drawLine( x + (i*62), y0, x + (i*62), y1, GxEPD_BLACK );
  }
}

void DisplayHandler::showCurrentWeather()
{
  char buffer[64];

  int x = 150;
  int y = 0;

  display.setFont(&WeatherIconsR_Regular24pt8b);
  display.setTextColor(GxEPD_BLACK);

  buffer[0] = getOWMicon( currentWeather["weather"][0]["id"].as<int>(), currentWeather["weather"][0]["icon"].as<String>().c_str());
  buffer[1] = 0;
  display.setCursor( x, y+60 );
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

  int windDir = currentWeather["wind"]["deg"].as<int>();
  int wd = windDir / 45;
  wd *= 45;
  w = 0;
  for( ; w<8; w++ )
  {
    if ( windDirection[w].degree == wd )
    {
      break;
    }
  }
  if ( w < 8 )
  {
    buffer[0] = windDirection[w].icon;
    buffer[1] = 0;
    showCentered( 578, 60, 62, buffer );
  }
  display.setFont(&DejaVuSans_Bold8pt8b);
  showCentered( 578, 90, 62, windDirection[w].description );

  display.setFont(&DejaVuSansCondensed_Bold8pt8b);
  display.setCursor( x, y+90 );
  display.print( utf8ToIso8859( buffer, currentWeather["weather"][0]["description"].as<String>()));

  display.setCursor( x+248, y+90 );
  display.setTextColor(GxEPD_RED);
  sprintf( buffer, "Max %0.1foC",  currentWeather["main"]["temp_max"].as<double>() );
  buffer[strlen(buffer)-2] = 176; // ° Sign
  display.print( buffer );
  display.setTextColor(GxEPD_BLACK);
  sprintf( buffer, "  Min %0.1foC",  currentWeather["main"]["temp_min"].as<double>() );
  buffer[strlen(buffer)-2] = 176; // ° Sign
  display.print( buffer );
}

