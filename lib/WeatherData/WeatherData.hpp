#ifndef __WEATHER_DATA_HPP__
#define __WEATHER_DATA_HPP__

#include <Arduino.h>
#include <ArduinoJson.h>

#define OWM_SUNRISE     0x0023
#define OWM_SUNSET      0x0024
#define OWM_MOONRISE    0x0025
#define OWM_MOONSET     0x0026
#define OWM_MOONPHASES_WAXING   0x0030
#define OWM_FULLMOON    0x003d
#define OWM_MOONPHASES_WANING   0x0040
#define OWM_NEWMOON     0x004d
#define OWM_WINDSPEED_0 0x0050

struct _weather_info
{
    char time[8];
    char weatherIcon[2];
    uint16_t weatherIconColor;
    char weatherDescription[64];
    char temperature[10];
    uint16_t tempColor;
    char temp_min[10];
    char temp_max[10];
    char humidity[8];
    char windSpeedIcon[2];
    char windDirectionIcon[2];
    char windDirectionDescription[3];
    double windSpeed;
    int windDegree;
};

class WeatherData
{
private:
  uint8_t findIcon( uint16_t icons[][2], uint16_t iconId );
  uint8_t getOWMicon( struct _weather_info *info, int weatherId, const char *iconName );
  DeserializationError readJSON(DynamicJsonDocument &doc, String url, int retries );
  DeserializationError getCurrentWeatherData();
  DeserializationError getWeatherForecastData();
  void get( struct _weather_info *info, JsonObject &obj );

public:
  bool update();
  void get( struct _weather_info *info ); // current weather
  void get( struct _weather_info *info, int index ); // weather forecast
};

extern WeatherData weatherData;

#endif
