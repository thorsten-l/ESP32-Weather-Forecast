#include "DisplayHandler.hpp"

void DisplayHandler::showWeatherForecast3h()
{
  int x = 145;
  int y = 120;
  
  for ( int i = 0; i< 8; i++ )
  {
    struct _weather_info wi;
    weatherData.get( &wi, i );

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&DejaVuSansCondensed_Bold8pt8b);
    display.setCursor( x, y );
    showCentered( x, y, 62, wi.time );

    display.setTextColor(wi.tempColor);
    showCentered( x, y+76, 62, wi.temperature );

    display.setTextColor(GxEPD_BLACK);
    showCentered( x, y+96, 62, wi.humidity );

    display.setFont(&WeatherIcons_SunMoonWind20pt7b);
    showCentered( x, y+136, 62, wi.windSpeedIcon );
    showCentered( x, y+172, 62, wi.windDirectionIcon );
    
    display.setFont(&DejaVuSans_Bold8pt8b);
    showCentered( x, y+192, 62, wi.windDirectionDescription );

    display.setFont(&WeatherIcons_CloudsNightDay20pt7b);
    display.setTextColor(wi.weatherIconColor);
    showCentered( x, y+48, 62, wi.weatherIcon );

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
  int x = 150;
  int y = 0;

  struct _weather_info wi;
  weatherData.get(&wi);

  display.setFont(&WeatherIcons_CloudsNightDay28pt7b);
  display.setTextColor(wi.weatherIconColor);
  display.setCursor( x, y+60 );
  display.print( wi.weatherIcon );

  display.setFont(&DejaVuSansCondensed_Bold24pt8b);
  display.setTextColor(wi.tempColor);
  display.print( " " );
  display.print( wi.temperature );
  display.setTextColor(GxEPD_BLACK);
  display.print( "  " );
  display.print( wi.humidity );
  display.setFont(&WeatherIcons_SunMoonWind28pt7b);
  display.print( " " );
  display.print( wi.windSpeedIcon );

  showCentered( x+428, y+60, 62, wi.windDirectionIcon );
  display.setFont(&DejaVuSans_Bold8pt8b);
  showCentered( x+428, y+90, 62, wi.windDirectionDescription );

  display.setFont(&DejaVuSansCondensed_Bold8pt8b);
  display.setCursor( x, y+90 );
  display.print( wi.weatherDescription );

  display.setCursor( x+248, y+90 );
  display.setTextColor(GxEPD_RED);
  display.print( "Max " );
  display.print( wi.temp_max );
  display.setTextColor(GxEPD_BLACK);
  display.print( "  Min " );
  display.print( wi.temp_min );
}

