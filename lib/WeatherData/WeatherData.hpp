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

struct _wind_direction
{
    int degree;
    uint8_t icon;
    char description[3];
};

DeserializationError getCurrentWeatherData();
DeserializationError getWeatherForecastData();

extern DynamicJsonDocument currentWeather;
extern DynamicJsonDocument weatherForecast;
extern double windSpeeds[];

extern uint16_t dayOWMIconMap[61][2];
extern uint16_t nightOWMIconMap[61][2];

extern _wind_direction windDirection[];

#endif
