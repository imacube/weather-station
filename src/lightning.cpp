#include "lightning.h"

char *check_lightning(uint8_t lightningInt, SparkFun_AS3935 &lightning)
{
    // Hardware has alerted us to an event, now we read the interrupt register
    if (digitalRead(lightningInt) == HIGH)
    {
        cJSON *root = cJSON_CreateObject();
        if (!root)
        {
            LOGE("Failed to allocate root JSON object");
            return NULL;
        }

        // Add the timestamp
        char timeStamp[20];
        formatLocalTime(timeStamp, 20);
        if (strlen(timeStamp) == 0)
        {
            LOGI("Empty string from formatLocalTime()");
        }
        else
        {
            cJSON_AddStringToObject(root, "timestamp", timeStamp);
        }

        uint8_t intVal = lightning.readInterruptReg();
        if (intVal == NOISE_INT)
        {
            LOGI("Noise.");
            cJSON_AddStringToObject(root, "event", "noise");
            // Too much noise? Uncomment the code below, a higher number means better
            // noise rejection.
            //      lightning.setNoiseLevel(noise);
        }
        else if (intVal == DISTURBER_INT)
        {
            LOGI("Disturber.");
            cJSON_AddStringToObject(root, "event", "disturber");
            // Too many disturbers? Uncomment the code below, a higher number means better
            // disturber rejection.
            //      lightning.watchdogThreshold(disturber);
        }
        else if (intVal == LIGHTNING_INT)
        {
            // Lightning! Now how far away is it? Distance estimation takes into
            // account any previously seen events in the last 15 seconds.
            char distance[4];
            uint8_t distanceToStorm = lightning.distanceToStorm();
            snprintf(distance, sizeof(distance), "%d", distanceToStorm);
            cJSON_AddStringToObject(root, "event", "strike");
            cJSON_AddStringToObject(root, "distance", distance);
            LOGI("Lightning Strike Detected! Approximately: %dkm away!", distanceToStorm);

            char lightningEnergy[11];
            snprintf(lightningEnergy, sizeof(lightningEnergy), "%d", lightning.lightningEnergy());
            cJSON_AddStringToObject(root, "energy", lightningEnergy);
            LOGI("Lightning energy: %d", lightningEnergy);
        }
        else
        {
            LOGI("Unknown interrupt val: %d", intVal);
            cJSON_Delete(root);
            return NULL;
        }

        cJSON_AddStringToObject(root, "type", "lightning");
        cJSON_AddNumberToObject(root, "millis", round(millis() / 1000));
        cJSON_AddStringToObject(root, "version", VERSION);

        char *jsonStr = cJSON_PrintUnformatted(root); // compact form
        cJSON_Delete(root);

        return jsonStr;
    }

    return NULL;
}
void setupLightning(uint8_t spiCS, uint8_t lightningInt, SparkFun_AS3935 &lightning)
{
    LOGI("MicroMod Weather Carrier Board - AS3935 Lightning Detector");

    // When lightning is detected the interrupt pin goes HIGH.
    pinMode(lightningInt, INPUT);

    SPI.begin();

    if (lightning.beginSPI(spiCS, 2000000) == false)
    {
        LOGI("Lightning Detector did not start up, freezing!");
        while (1)
            ;
    }
    else
        LOGI("Schmow-ZoW, Lightning Detector Ready!");

    // The lightning detector defaults to an indoor setting at
    // the cost of less sensitivity, if you plan on using this outdoors
    // uncomment the following line:
    //  lightning.setIndoorOutdoor(OUTDOOR);

    lightning_settings(lightning);
}

void lightning_settings(SparkFun_AS3935 &lightning)
{
    LOGI("/*******************************  Lightning Sensor Settings  *******************************/");

    // "Disturbers" are events that are false lightning events. If you find
    // yourself seeing a lot of disturbers you can have the chip not report those
    // events on the interrupt lines.

    lightning.maskDisturber(true);

    int maskVal = lightning.readMaskDisturber();
    char maskDisturber[8];
    if (maskVal == 1)
        snprintf(maskDisturber, sizeof(maskDisturber), "YES");
    else if (maskVal == 0)
        snprintf(maskDisturber, sizeof(maskDisturber), "NO");
    LOGI("Are disturbers being masked: %s", maskDisturber);

    // The lightning detector defaults to an indoor setting (less
    // gain/sensitivity), if you plan on using this outdoors
    // uncomment the following line:

    lightning.setIndoorOutdoor(INDOOR);

    uint8_t enviVal = lightning.readIndoorOutdoor();
    char logBuf[64] = "Are we set for indoor or outdoor: ";
    if (enviVal == INDOOR)
        snprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), "Indoor.");
    else if (enviVal == OUTDOOR)
        snprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), "Outdoor.");
    else
        snprintf(logBuf + strlen(logBuf), sizeof(logBuf) - strlen(logBuf), "Unkown value, %d", enviVal, BIN);
    LOGI("%s", logBuf);

    // Noise floor setting from 1-7, one being the lowest. Default setting is
    // two. If you need to check the setting, the corresponding function for
    // reading the function follows.

    lightning.setNoiseLevel(5);

    uint8_t noiseVal = lightning.readNoiseLevel();
    LOGI("Noise Level is set at: %d", noiseVal);

    // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
    // two. If you need to check the setting, the corresponding function for
    // reading the function follows.

    lightning.watchdogThreshold(WATCHDOG_THRESHOLD);

    uint8_t watchVal = lightning.readWatchdogThreshold();
    LOGI("Watchdog Threshold is set to: %d", watchVal);

    // Spike Rejection setting from 1-11, one being the lowest. Default setting is
    // two. If you need to check the setting, the corresponding function for
    // reading the function follows.
    // The shape of the spike is analyzed during the chip's
    // validation routine. You can round this spike at the cost of sensitivity to
    // distant events.

    // lightning.spikeRejection(spike);

    uint8_t spikeVal = lightning.readSpikeRejection();
    LOGI("Spike Rejection is set to: %d", spikeVal);

    // This setting will change when the lightning detector issues an interrupt.
    // For example you will only get an interrupt after five lightning strikes
    // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.
    // Followed by its corresponding read function. Default is zero.

    // lightning.lightningThreshold(lightningThresh);

    uint8_t lightVal = lightning.readLightningThreshold();
    LOGI("The number of strikes before interrupt is triggerd: %d", lightVal);

    // When the distance to the storm is estimated, it takes into account other
    // lightning that was sensed in the past 15 minutes. If you want to reset
    // time, then you can call this function.

    // lightning.clearStatistics();

    // The power down function has a BIG "gotcha". When you wake up the board
    // after power down, the internal oscillators will be recalibrated. They are
    // recalibrated according to the resonance frequency of the antenna - which
    // should be around 500kHz. It's highly recommended that you calibrate your
    // antenna before using these two functions, or you run the risk of schewing
    // the timing of the chip.

    // lightning.powerDown();
    // delay(1000);
    // if( lightning.wakeUp() )
    //  Serial.println("Successfully woken up!");
    // else
    // Serial.println("Error recalibrating internal osciallator on wake up.");

    // Set too many features? Reset them all with the following function.
    // lightning.resetSettings();
}