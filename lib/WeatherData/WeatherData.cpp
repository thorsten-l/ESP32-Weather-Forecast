#include <HTTPClient.h>
#include <WeatherData.hpp>
#include <WiFi.h>

extern String getTimeString(time_t time);

DynamicJsonDocument currentWeather(1000);
DynamicJsonDocument weatherForcast(32000);

DeserializationError readJSON(DynamicJsonDocument &doc, String url)
{
  HTTPClient http;
  DeserializationError err = DeserializationError::IncompleteInput;

  Serial.print("[HTTP] begin...\n");

  http.begin(url);

  Serial.print("[HTTP] GET...\n");

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
      weatherForcast,
      "http://api.openweathermap.org/data/2.5/"
      "forecast?id=2920632&appid=53166edfe73f27534840e137234035c7&units="
      "metric&lang=de");

  if (err == DeserializationError::Ok)
  {
    int cnt = weatherForcast["cnt"].as<int>();
    Serial.printf("cnt=%d\n", cnt );

    for( int i=0; i<cnt; i++)
    {
      JsonObject obj = weatherForcast["list"][i];
      showForecast( i, obj );
    }
  }

  return err;
}
