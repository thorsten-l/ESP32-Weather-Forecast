
#include "DisplayHandler.hpp"

GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display(GxEPD2_750c(/*CS=*/15, /*DC=*/27, /*RST=*/26, /*BUSY=*/25));

DisplayHandler displayHandler;

void DisplayHandler::setup()
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

void DisplayHandler::update()
{
  struct tm timeinfo;
  time_t now = time(0);
  // now += 45 * 86400;
  localtime_r(&now, &timeinfo);

  Serial.printf( "\n\n(%02d:%02d) Starting display update...\n", timeinfo.tm_hour, timeinfo.tm_min );
  bool displayError = false;

  DeserializationError err = getCurrentWeatherData();
  if ( err != DeserializationError::Ok )
  {
    Serial.print( "ERROR: Reading current weather data: " );
    Serial.println( err.c_str() );
    displayError = true;
  }

  err = getWeatherForecastData();
  if ( err != DeserializationError::Ok )
  {
    Serial.print( "ERROR: Reading weather forecast data: " );
    Serial.println( err.c_str() );
    displayError = true;
  }

  display.setFullWindow();
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();

  do
  {
    showCalendar(now, timeinfo, 2, 0);
    showClock(timeinfo, 2, 315);
    showSunMoon( timeinfo );
    showCurrentWeather();
    showWeatherForecast3h();
    showGrid();
    if ( displayError == true )
    {
      showError();
    } 
  } 
  while (display.nextPage());
  Serial.println("display update done.\n");
}
