#include <App.hpp>
#include <HTTPClient.h>
#include <WeatherData.hpp>
#include <WiFi.h>

#define NUMBER_OF_RETRIES 3

DynamicJsonDocument currentWeather(1000);
DynamicJsonDocument weatherForecast(32000);

_wind_direction windDirection[] = {
  {   0, 0x70,  "N" },
  {  45, 0x71, "NO" },
  {  90, 0x72,  "O" },
  { 135, 0x73, "SO" },
  { 180, 0x74,  "S" },
  { 225, 0x75, "SW" },
  { 270, 0x76,  "W" },
  { 315, 0x77, "NW" }
};

// in m/s        Beaufort 0    1    2    3    4     5     6     7     8     9    10    11
double windSpeeds[] = { 0.3, 1.5, 3.3, 5.4, 7.9, 10.7, 13.8, 17.1, 20.7, 24.4, 28.4, 32.6 };

uint16_t dayOWMIconMap[61][2] PROGMEM = {
    {200, 0x4D}, {201, 0x4D}, {202, 0x4D}, {210, 0x45}, {211, 0x45}, {212, 0x45},
    {221, 0x45}, {230, 0x4D}, {231, 0x4D}, {232, 0x4D}, {300, 0x4B}, {301, 0x4B},
    {302, 0x48}, {310, 0x48}, {311, 0x48}, {312, 0x48}, {313, 0x48}, {314, 0x48},
    {321, 0x4B}, {500, 0x4B}, {501, 0x48}, {502, 0x48}, {503, 0x48}, {504, 0x48},
    {511, 0x46}, {520, 0x49}, {521, 0x49}, {522, 0x49}, {531, 0x4C}, {600, 0x4A},
    {601, 0x71}, {602, 0x4A}, {611, 0x46}, {612, 0x46}, {615, 0x46}, {616, 0x46},
    {620, 0x46}, {621, 0x4A}, {622, 0x4A}, {701, 0x41}, {711, 0x26}, {721, 0x29},
    {731, 0x2A}, {741, 0x43}, {761, 0x2A}, {762, 0x2A}, {771, 0x40}, {781, 0x25},
    {800, 0x21}, {801, 0x40}, {802, 0x40}, {803, 0x40}, {804, 0x42}, {900, 0x25},
    {901, 0x4C}, {902, 0x2d}, {903, 0x2b}, {904, 0x28}, {905, 0x23}, {906, 0x44},
    {957, 0x24}};

uint16_t nightOWMIconMap[61][2] PROGMEM = {
    {200, 0x3D}, {201, 0x3D}, {202, 0x3D}, {210, 0x35}, {211, 0x35}, {212, 0x35},
    {221, 0x35}, {230, 0x3D}, {231, 0x3D}, {232, 0x3D}, {300, 0x3B}, {301, 0x3B},
    {302, 0x38}, {310, 0x38}, {311, 0x38}, {312, 0x38}, {313, 0x38}, {314, 0x38},
    {321, 0x3B}, {500, 0x3B}, {501, 0x38}, {502, 0x38}, {503, 0x38}, {504, 0x38},
    {511, 0x36}, {520, 0x39}, {521, 0x39}, {522, 0x39}, {531, 0x3C}, {600, 0x3A},
    {601, 0x70}, {602, 0x3A}, {611, 0x36}, {612, 0x36}, {615, 0x36}, {616, 0x36},
    {620, 0x36}, {621, 0x3A}, {622, 0x3A}, {701, 0x31}, {711, 0x26}, {721, 0x29},
    {731, 0x2A}, {741, 0x33}, {761, 0x2A}, {762, 0x2A}, {771, 0x30}, {781, 0x25},
    {800, 0x22}, {801, 0x30}, {802, 0x30}, {803, 0x30}, {804, 0x32}, {900, 0x25},
    {901, 0x3C}, {902, 0x2d}, {903, 0x2b}, {904, 0x28}, {905, 0x23}, {906, 0x34},
    {957, 0x24}};

DeserializationError readJSON(DynamicJsonDocument &doc, String url, int retries )
{
  HTTPClient http;
  DeserializationError err = DeserializationError::IncompleteInput;

  Serial.print("[HTTP] begin...\n");

  http.begin(url);

  Serial.print("[HTTP] GET...\n");

  // http.addHeader( "Accept-Charset", "ISO-8859-1", true ); // not working

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      WiFiClient *stream = http.getStreamPtr();

      err = deserializeJson(doc, *stream);
      Serial.printf("[JSON] %s\n", err.c_str());
      Serial.print("[HTTP] connection closed or file end.\n");
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n",
                  http.errorToString(httpCode).c_str());
  }
  http.end();

  if ( err != DeserializationError::Ok && retries >= 1 )
  {
    Serial.printf( "ERROR: Parsing weather data. (%s, retries=%d)\n", err.c_str(), retries-- );
    delay( 5000 );
    err = readJSON( doc,  url, retries );
  }

  return err;
}

DeserializationError getCurrentWeatherData()
{
  DeserializationError err = readJSON(
      currentWeather,
      "http://api.openweathermap.org/data/2.5/"
      "weather?id=" OPEN_WEATHER_MAP_CITY_ID 
      "&appid=" OPEN_WEATHER_MAP_APPID "&units="
      "metric&lang=de", NUMBER_OF_RETRIES );

  return err;
}

DeserializationError getWeatherForecastData()
{
  DeserializationError err = readJSON(
      weatherForecast,
      "http://api.openweathermap.org/data/2.5/"
      "forecast?id=" OPEN_WEATHER_MAP_CITY_ID 
      "&appid=" OPEN_WEATHER_MAP_APPID "&units="
      "metric&lang=de", NUMBER_OF_RETRIES );

  return err;
}
