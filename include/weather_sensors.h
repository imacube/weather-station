#ifndef WEATHER_SENSORS_H
#define WEATHER_SENSORS_H

#include <Wire.h>
#include <cJSON.h>

#include <Adafruit_DS248x.h>
#include <SparkFunBME280.h>
#include <SparkFun_Weather_Meter_Kit_Arduino_Library.h>

#include "Timestamp.h"
#include "global.h"
#include "secret.h"

/** DS18B20 temperature sensor */
#define DS18B20_FAMILY_CODE 0x28
#define DS18B20_CMD_CONVERT_T 0x44
#define DS18B20_CMD_MATCH_ROM 0x55
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

/** Weather board sensor pins */
const uint8_t windDirectionPin = 35; // Wind direction
const uint8_t windSpeedPin = 14;     // Wind speed
const uint8_t rainfallPin = 27;      // Rain fall sensor
const uint8_t soilPin = A0;          // Pin number that measures analog moisture signal
const uint8_t soilPower = G0;        // Pin number that will power the soil moisture sensor

enum class InitResult
{
    Success,
    BME280Failed,
    DS248Failed,
    DS248NoOneWireDeviceFound,
    DS248NoTemperatureProbeFound,
    Unknown
};

InitResult setupWeatherSensors(BME280 &bme280, SFEWeatherMeterKit &weatherMeterKit, Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[]);
char *sendSensorReadings(BME280 &bme280, SFEWeatherMeterKit &weatherMeterKit, Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[]);
float readTemperature(Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[]);
uint16_t readSoil();

#endif