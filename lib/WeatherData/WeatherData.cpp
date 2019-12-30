#include <HTTPClient.h>
#include <WeatherData.hpp>
#include <WiFi.h>

// in m/s        Beaufort 0    1    2    3    4     5     6     7     8     9    10    11
double windSpeeds[] = { 0.3, 1.5, 3.3, 5.4, 7.9, 10.7, 13.8, 17.1, 20.7, 24.4, 28.4, 32.6 };

uint16_t stdOWMIconMap[61][2] PROGMEM = {
    {200, 0x3e}, {201, 0x3e}, {202, 0x3e}, {210, 0x36}, {211, 0x36}, {212, 0x36},
    {221, 0x36}, {230, 0x3e}, {231, 0x3e}, {232, 0x3e}, {300, 0x3c}, {301, 0x3c},
    {302, 0x39}, {310, 0x37}, {311, 0x39}, {312, 0x39}, {313, 0x3a}, {314, 0x39},
    {321, 0x3c}, {500, 0x3c}, {501, 0x39}, {502, 0x39}, {503, 0x39}, {504, 0x39},
    {511, 0x37}, {520, 0x3a}, {521, 0x3a}, {522, 0x3a}, {531, 0x3d}, {600, 0x3b},
    {601, 0x3b}, {602, 0xce}, {611, 0x37}, {612, 0x37}, {615, 0x37}, {616, 0x37},
    {620, 0x37}, {621, 0x3b}, {622, 0x3b}, {701, 0x32}, {711, 0x7d}, {721, 0xcf},
    {731, 0x7e}, {741, 0x34}, {761, 0x7e}, {762, 0x7e}, {771, 0x31}, {781, 0x72},
    {800, 0x2e}, {801, 0x31}, {802, 0x31}, {803, 0x32}, {804, 0x33}, {900, 0x72},
    {901, 0x3d}, {902, 0x8d}, {903, 0x90}, {904, 0x8c}, {905, 0x3f}, {906, 0x35},
    {957, 0x6c}};

uint16_t dayOWMIconMap[61][2] PROGMEM = {
    {200, 0x30}, {201, 0x30}, {202, 0x30}, {210, 0x26}, {211, 0x26}, {212, 0x26},
    {221, 0x26}, {230, 0x30}, {231, 0x30}, {232, 0x30}, {300, 0x2c}, {301, 0x2c},
    {302, 0x29}, {310, 0x29}, {311, 0x29}, {312, 0x29}, {313, 0x29}, {314, 0x29},
    {321, 0x2c}, {500, 0x2c}, {501, 0x29}, {502, 0x29}, {503, 0x29}, {504, 0x29},
    {511, 0x27}, {520, 0x2a}, {521, 0x2a}, {522, 0x2a}, {531, 0x2f}, {600, 0x2b},
    {601, 0xe1}, {602, 0x2b}, {611, 0x27}, {612, 0x27}, {615, 0x27}, {616, 0x27},
    {620, 0x27}, {621, 0x2b}, {622, 0x2b}, {701, 0x22}, {711, 0x7d}, {721, 0xcf},
    {731, 0x7e}, {741, 0x24}, {761, 0x7e}, {762, 0x7e}, {771, 0x21}, {781, 0x72},
    {800, 0x2e}, {801, 0x21}, {802, 0x21}, {803, 0x21}, {804, 0x23}, {900, 0x72},
    {901, 0x2f}, {902, 0x8d}, {903, 0x90}, {904, 0x8c}, {905, 0xdd}, {906, 0x25},
    {957, 0x6c}};

uint16_t nightOWMIconMap[61][2] PROGMEM = {
    {200, 0x4b}, {201, 0x4b}, {202, 0x4b}, {210, 0x43}, {211, 0x43}, {212, 0x43},
    {221, 0x43}, {230, 0x4b}, {231, 0x4b}, {232, 0x4b}, {300, 0x49}, {301, 0x49},
    {302, 0x46}, {310, 0x46}, {311, 0x46}, {312, 0x46}, {313, 0x46}, {314, 0x46},
    {321, 0x49}, {500, 0x49}, {501, 0x46}, {502, 0x46}, {503, 0x46}, {504, 0x46},
    {511, 0x44}, {520, 0x47}, {521, 0x47}, {522, 0x47}, {531, 0x4a}, {600, 0x48},
    {601, 0xe3}, {602, 0x48}, {611, 0x44}, {612, 0x44}, {615, 0x44}, {616, 0x44},
    {620, 0x44}, {621, 0x48}, {622, 0x48}, {701, 0x41}, {711, 0x7d}, {721, 0xcf},
    {731, 0x7e}, {741, 0x67}, {761, 0x7e}, {762, 0x7e}, {771, 0x40}, {781, 0x72},
    {800, 0x4c}, {801, 0x40}, {802, 0x40}, {803, 0x40}, {804, 0x9f}, {900, 0x72},
    {901, 0x4a}, {902, 0x8d}, {903, 0x90}, {904, 0x8c}, {905, 0x3f}, {906, 0x42},
    {957, 0x6c}};

extern String getTimeString(time_t time);

DynamicJsonDocument currentWeather(1000);
DynamicJsonDocument weatherForecast(32000);

DeserializationError readJSON(DynamicJsonDocument &doc, String url)
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
  return err;
}

DeserializationError getCurrentWeatherData()
{
  DeserializationError err = readJSON(
      currentWeather,
      "http://api.openweathermap.org/data/2.5/"
      "weather?id=2920632&appid=53166edfe73f27534840e137234035c7&units="
      "metric&lang=de");

/*
  if (err == DeserializationError::Ok)
  {
    time_t dt = currentWeather["dt"].as<long>();
    Serial.printf("dt=%ld %s\n", dt, getTimeString(dt).c_str());
    Serial.printf(
        "weather=%s\n",
        currentWeather["weather"][0]["description"].as<String>().c_str());
    dt = currentWeather["sys"]["sunset"].as<long>();
    Serial.printf("sunset=%ld %s\n", dt, getTimeString(dt).c_str());
  }

  Serial.println();
*/

  return err;
}

void showForecast( int i, JsonObject& obj )
{
  time_t dt = obj["dt"].as<long>();

  Serial.printf("%02d dt=%ld %s, dt_txt=%s\n", 
    i, 
    dt, 
    getTimeString(dt).c_str(), 
    obj["dt_txt"].as<String>().c_str());

  Serial.printf("temp=%.2f, humidity=%d%%\n", 
    obj["main"]["temp"].as<double>(), 
    obj["main"]["humidity"].as<int>());

  Serial.printf( "weather id=%d, weather description=%s\n",
    obj["weather"][0]["id"].as<int>(), 
    obj["weather"][0]["description"].as<String>().c_str());

  Serial.printf("wind speed=%.2f, wind deg=%d\n\n",
    obj["wind"]["speed"].as<double>(), 
    obj["wind"]["deg"].as<int>() );
}

DeserializationError getWeatherForecastData()
{
  DeserializationError err = readJSON(
      weatherForecast,
      "http://api.openweathermap.org/data/2.5/"
      "forecast?id=2920632&appid=53166edfe73f27534840e137234035c7&units="
      "metric&lang=de");

  /* if (err == DeserializationError::Ok)
  {
    int cnt = weatherForecast["cnt"].as<int>();
    // Serial.printf("cnt=%d\n", cnt );

    for( int i=0; i<cnt; i++)
    {
      JsonObject obj = weatherForecast["list"][i];
      showForecast( i, obj );
    }
    
  }
  */

  return err;
}
