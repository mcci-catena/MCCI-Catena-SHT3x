/*

Module: sht3x-simple.ino

Function:
        Simple example for SHT3x sensor.

Copyright and License:
        See accompanying LICENSE file.

Author:
        Terry Moore, MCCI Corporation   June 2019

*/

#include <Catena-SHT3x.h>

#include <Arduino.h>
#include <Wire.h>

/****************************************************************************\
|
|   Manifest constants & typedefs.
|
\****************************************************************************/

using namespace McciCatenaSht3x;

/****************************************************************************\
|
|   Read-only data.
|
\****************************************************************************/

/****************************************************************************\
|
|   Variables.
|
\****************************************************************************/

cSHT3x gSht3x {Wire, cSHT3x::Address_t::A};

/****************************************************************************\
|
|   Code.
|
\****************************************************************************/

void setup()
    {
    Serial.begin(115200);

    // wait for USB to be attached.
    while (! Serial)
        yield();

    Serial.println("SHT3x Simple Test");
    if (! gSht3x.begin())
        {
        for (;;)
            {
            Serial.println("gSht3x.begin() failed\n");
            delay(2000);
            }
        }
    }

void loop()
    {
    cSHT3x::Measurements m;

    if (! gSht3x.getTemperatureHumidity(m))
        {
        Serial.println("can't read T/RH");
        }
    else
        {
        Serial.print("T(F)=");
        Serial.print(m.Temperature * 1.8 + 32);
        Serial.print("  RH=");
        Serial.print(m.Humidity);
        Serial.println("%");
        }

    delay(1000);
    }
