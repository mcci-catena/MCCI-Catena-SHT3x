/*

Module: sht3x-simple.ino

Function:
        Simple example for SHT-3x sensor.

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

cSHT_3x gSht3x {Wire, cSHT_3x::Address_t::A};

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

    Serial.println("SHT-3x Simple Test");
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
    float t, rh;

    if (! gSht3x.getTemperatureHumidity(t, rh))
        {
        Serial.println("can't read T/RH");
        }
    else
        {
        Serial.print("T(F)=");
        Serial.print(t * 1.8 + 32);
        Serial.print("  RH=");
        Serial.print(rh);
        Serial.println("%");
        }

    delay(1000);
    }
