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
    showCentered( x, y+70, 62, wi.temperature );

    display.setTextColor(GxEPD_BLACK);
    // showCentered( x, y+88, 62, wi.humidity );
    showCentered( x, y+88, 62, wi.cloudsAll );
    
    // if ( wi.isRaining )
    {
      showCentered( x, y+106, 62, wi.rain );
    }
    
    display.setFont(&WeatherIcons_SunMoonWind20pt7b);
    showCentered( x, y+140, 62, wi.windSpeedIcon );
    showCentered( x, y+174, 62, wi.windDirectionIcon );
    
    display.setFont(&DejaVuSans_Bold8pt8b);
    showCentered( x, y+192, 62, wi.windDirectionDescription );

    display.setFont(&WeatherIcons_CloudsNightDay20pt7b);
    display.setTextColor(wi.weatherIconColor);
    showCentered( x, y+46, 62, wi.weatherIcon );

    x += 62;
  }
}

void DisplayHandler::showGrid()
{
  int y0 = 100;
  int y1 = 320;
  int x = 144;

  display.drawLine( x, y0, 640, y0, GxEPD_BLACK );
  display.drawLine( x, y1, 640, y1, GxEPD_BLACK );

  for( int i = 1; i<=7; i++ )
  {
    display.drawLine( x + (i*62), y0, x + (i*62), y1, GxEPD_BLACK );
  }
}

void DisplayHandler::showCurrentWeather()
{
  int x = 148;
  int y = 0;

  struct _weather_info wi;
  weatherData.get(&wi);

  display.setFont(&WeatherIcons_CloudsNightDay28pt7b);
  display.setTextColor(wi.weatherIconColor);
  display.setCursor( x, y+60 );
  display.print( wi.weatherIcon );

  display.setFont(&DejaVuSansCondensed_Bold24pt8b);
  display.setTextColor(wi.tempColor);
  display.setCursor( x+82, y+60 );
  display.print( wi.temperature );
  display.setTextColor(GxEPD_BLACK);
  display.setCursor( x+250, y+40 );
  display.print( wi.humidity );
  display.setFont(&WeatherIcons_SunMoonWind28pt7b);
  display.print( " " );
  display.print( wi.windSpeedIcon );

  showCentered( x+430, y+40, 62, wi.windDirectionIcon );
  display.setFont(&DejaVuSans_Bold8pt8b);
  showCentered( x+430, y+60, 62, wi.windDirectionDescription );

  display.setFont(&DejaVuSansCondensed_Bold8pt8b);
  display.setCursor( x, y+90 );
  display.print( wi.weatherDescription );

  display.setCursor( x+250, y+60 );
  display.setTextColor(GxEPD_RED);
  display.print( "Max " );
  display.print( wi.tempMax );
  display.setTextColor(GxEPD_BLACK);
  display.print( "  Min " );
  display.print( wi.tempMin );

  display.setCursor( x+250, y+76 );
  display.print( "Gef " );
  display.print( wi.feelsLike );
  display.print( "  Wol " );
  display.print( wi.cloudsAll );

  display.setCursor( x+250, y+94 );
  display.print( "Reg " );
  display.print( wi.rain );
  display.print( "mm" );

}

