#include "weather_sensors.h"

InitResult setupWeatherSensors(BME280 &bme280, SFEWeatherMeterKit &weatherMeterKit, Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[])
{
    /** Setup the BME280 sensor */
    Wire.begin();

    if (!bme280.beginI2C())
    {
        LOGE("BME280 sensor did not respond");
        return InitResult::BME280Failed;
    }

    SFEWeatherMeterKitCalibrationParams parameters = weatherMeterKit.getCalibrationParams();
    parameters.mmPerRainfallCount = 0.3339;
    weatherMeterKit.setCalibrationParams(parameters);

    weatherMeterKit.begin();

    /** Setup the external DS18B20 temperature sensor */
    if (!ds248x.begin(&Wire, DS248X_ADDRESS))
    {
        LOGE("DS248x initialization failed");
        return InitResult::DS248Failed;
    }
    LOGI("DS248x OK!");

    if (!ds248x.OneWireSearch(temperatureProbeAddr))
    {
        LOGE("No 1-Wire devices found!");
        return InitResult::DS248NoOneWireDeviceFound;
    }
    LOGI("One Wire bus reset OK");

    LOGI("Found 1-Wire Address");
    if (temperatureProbeAddr[0] == DS18B20_FAMILY_CODE)
    {
        LOGI("Found DS18B20 Thermometer");
    }
    else
    {
        LOGE("No temperature probe found!");
        return InitResult::DS248NoTemperatureProbeFound;
    }

    /** Setup of the soil probe */
    pinMode(soilPower, OUTPUT);
    digitalWrite(soilPower, LOW);

    /** All done */
    return InitResult::Success;
}

char *sendSensorReadings(BME280 &bme280, SFEWeatherMeterKit &weatherMeterKit, Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[])
{
    LOGI("Checking sensors at %d", millis());

    LOGV("creating JSON object");
    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        LOGE("Failed to allocate root JSON object");
        return NULL;
    }

    LOGV("Add the timestamp");
    LOGV("Calling formatLocalTime()");
    char timeStamp[20];
    formatLocalTime(timeStamp, 20);
    if (strlen(timeStamp) == 0)
    {
        LOGI("Empty string from formatLocalTime()");
    }
    else
    {
        LOGV("Adding timestamp to JSON");
        cJSON_AddStringToObject(root, "timestamp", timeStamp);
    }
    // free(timeStamp);

    LOGV("Collect data from weather meter kit");
    char windDirection[32];
    char windSpeed[32];
    char totalRainfall[32];
    char rainfallCounts[12]; // uint32_t has max size of 4294967296
    LOGV("Calling snprintf for wind and rain data");
    snprintf(windDirection, sizeof(windDirection), "%.6g", weatherMeterKit.getWindDirection());
    snprintf(windSpeed, sizeof(windSpeed), "%.6g", weatherMeterKit.getWindSpeed());
    snprintf(totalRainfall, sizeof(totalRainfall), "%.6g", weatherMeterKit.getTotalRainfall());
    snprintf(rainfallCounts, sizeof(rainfallCounts), "%.6g", weatherMeterKit.getRainfallCounts());
    LOGV("Adding wind and rain data to JSON");
    cJSON_AddStringToObject(root, "wind_direction", windDirection);
    cJSON_AddStringToObject(root, "wind_speed", windSpeed);
    cJSON_AddStringToObject(root, "total_rainfall", totalRainfall);
    cJSON_AddStringToObject(root, "rainfall_counts", rainfallCounts);

    LOGV("Collecting BME280 measurements");
    LOGV("Creating BME280 object");
    BME280_SensorMeasurements measurements;
    LOGV("Reading measurements");
    bme280.readAllMeasurements(&measurements, 0);
    char humidity[32];
    char pressure[32];
    char board_temp_c[32];
    LOGV("Using snprintf to write BME280 values to char[]");
    snprintf(humidity, sizeof(humidity), "%.6g", (round(measurements.humidity * 100) / 100));
    snprintf(pressure, sizeof(pressure), "%.6g", (round((measurements.pressure * 100)) / 100 / 100)); // hPA for the returned value
    snprintf(board_temp_c, sizeof(board_temp_c), "%.6g", (round(measurements.temperature * 100) / 100));
    LOGV("Adding BME280 values to JSON");
    cJSON_AddStringToObject(root, "humidity", humidity);
    cJSON_AddStringToObject(root, "pressure", pressure);
    cJSON_AddStringToObject(root, "board_temp_c", board_temp_c);

    /** External temperature sensor */
    LOGV("Reading temperature from external sensor");
    char external_temp_c[32];
    LOGV("Using snprintf to store the temperature value");
    snprintf(external_temp_c, sizeof(external_temp_c), "%.6g", (round(readTemperature(ds248x, temperatureProbeAddr) * 100) / 100));
    LOGV("Adding external temperature to JSON");
    cJSON_AddStringToObject(root, "external_temp_c", external_temp_c);

    /** Soil moisture reading */
    LOGV("Reading soil moisture");
    char soil_moisture[10];
    LOGV("Using snprintf to store soil mosture value");
    snprintf(soil_moisture, sizeof(soil_moisture), "%d", readSoil());
    LOGV("Adding soil mositure to JSON");
    cJSON_AddStringToObject(root, "soil_moisture", soil_moisture);

    /** Set the type and send */
    LOGV("Adding type, millis, and version to JSON");
    cJSON_AddStringToObject(root, "type", "weather_data");
    cJSON_AddNumberToObject(root, "millis", round(millis() / 1000));
    cJSON_AddStringToObject(root, "version", VERSION);

    LOGV("Converting JSON to char*");
    char *jsonStr = cJSON_PrintUnformatted(root); // compact form
    LOGV("Deleting JSON object");
    cJSON_Delete(root);
    
    LOGV("Returning JSON string to calling function");
    return jsonStr;
}

float readTemperature(Adafruit_DS248x &ds248x, uint8_t temperatureProbeAddr[])
{
    // Select the DS18B20 device
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM); // Match ROM command
    for (int i = 0; i < 8; i++)
    {
        ds248x.OneWireWriteByte(temperatureProbeAddr[i]);
    }

    // Start temperature conversion
    ds248x.OneWireWriteByte(DS18B20_CMD_CONVERT_T); // Convert T command
    delay(750);                                     // Wait for conversion (750ms for maximum precision)

    // Read scratchpad
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(DS18B20_CMD_MATCH_ROM); // Match ROM command
    for (int i = 0; i < 8; i++)
    {
        ds248x.OneWireWriteByte(temperatureProbeAddr[i]);
    }
    ds248x.OneWireWriteByte(DS18B20_CMD_READ_SCRATCHPAD); // Read Scratchpad command

    uint8_t data[9];
    for (int i = 0; i < 9; i++)
    {
        ds248x.OneWireReadByte(&data[i]);
    }

    // Calculate temperature
    int16_t raw = (data[1] << 8) | data[0];
    float celsius = (float)raw / 16.0;

    return celsius;
}

uint16_t readSoil()
{
    uint16_t moistVal = 0; // Variable for storing moisture value
    // Power the sensor
    digitalWrite(soilPower, HIGH);
    delay(10);
    moistVal = analogRead(soilPin); // Read the SIG value from sensor. Moisture value depends on PB's operating voltage and ADC resolution
                                    // Check out the SparkFun Soil Moisture Sensor hookup guide for tips on calibrating your sensor
                                    // https://learn.sparkfun.com/tutorials/soil-moisture-sensor-hookup-guide?_ga=2.27184827.782829043.1597935536-2045279763.156684911
    digitalWrite(soilPower, LOW);   // Turn the sensor off
    return moistVal;                // Return current moisture value
}