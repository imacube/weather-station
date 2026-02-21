#ifndef LIGHTNING_H
#define LIGHTNING_H

#include <SPI.h>
#include <SparkFun_AS3935.h>
#include <Wire.h>
#include <cJSON.h>

#include "global.h"
#include "secret.h"

#include "Timestamp.h"
#include "logging.h"

#define INDOOR 0x12
#define OUTDOOR 0xE

/** Lightning configuration */
const uint8_t lightningInt = G3;
const uint8_t spiCS = G1;

/** Tuneable values */
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04 // Value between 1-10
#define NOISE_INT 0x01     // Value between 1-7
#define WATCHDOG_THRESHOLD 3

char *check_lightning(uint8_t lightningInt, SparkFun_AS3935 &lightning);

void setupLightning(uint8_t spiCS, uint8_t lightningInt, SparkFun_AS3935 &lightning);

void lightning_settings(SparkFun_AS3935 &lightning);

#endif