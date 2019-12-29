#ifndef __WEATHER_DATA_HPP__
#define __WEATHER_DATA_HPP__

#include <Arduino.h>
#include <ArduinoJson.h>

#define OWM_SUNRISE     0x00ac
#define OWM_SUNSET      0x00ad
#define OWM_MOONRISE    0x00ae
#define OWM_MOONSET     0x00af
#define OWM_MOONPHASES_WAXING   0x00b0
#define OWM_FULLMOON    0x00bd
#define OWM_MOONPHASES_WANING   0x00c0
#define OWM_NEWMOON     0x00cd

DeserializationError getCurrentWeatherData();
DeserializationError getWeatherForecastData();

#endif
