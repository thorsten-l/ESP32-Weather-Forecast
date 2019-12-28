#ifndef __WEATHER_DATA_HPP__
#define __WEATHER_DATA_HPP__

#include <Arduino.h>
#include <ArduinoJson.h>

DeserializationError getCurrentWeatherData();
DeserializationError getWeatherForecastData();

#endif
