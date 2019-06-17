# MCCI Catena&reg; SHT-3x Sensor Library

This library provides a simple interface to Sensirion SHT-31, SHT-32, and SHT-35 sensors.

## Introduction

Clients interact with Sensirion sensors via the following sequence.

1. Initially, the client creates an instance object for the sensor. When creating the object, the client passes a `Wire` object (representing the I2C bus used for communication), and optionally the address.  (For future use, the library also allows you to identify Arduino pins for use as NRESET and ALERT.)

2. If the client only needs to take occasional measurements, the client calls either the `cSHT_3x::getTemperatureHumidity()` method (which returns temperature and humidity scaled in engineering units), or `cSHT_3x::getTemperatureHumidityRaw()` (which returns temperature and humidity as `uint16_t` unscaled values).  Generally, the former is used if data is to be processed locally on the Arduino, and the latter is used if data is to be transmitted via a LPWAN network.

3. If the client needs to make periodic measurements, the client first calls `cSHT_3x::startPeriodicMeasurement()` to set the parameters for the periodic measurement, and start the acquisition process. The result of this call is the number of milliseconds per measurement.

   To collect results, the client occasionaly calls `cSHT_3x::getPeriodicMeasurement()` or `cSHT_3x::getPeriodicMeasurementRaw()`. If a measurement is available, it will be returned, and the method returns `true`; otherwise, the method returns `false`.  To save power, the client should delay the appropriate number of milliseconds between calls (as indicated by the result of `cSHT_3x::startPeriodicMeasurement()`).

A number of utility methods allow the client to manage the sensor.

- `cSHT_3x::reset()` issues a soft reset to the device.
- `cSHT_3x::end()` idles the device, and is typically used prior to sleeping the system.
- By default, the library checks CRCs on received data. `cSHT_3x::getCrcMode()` and `cSHT_3x::setCrcMode()` allow the client to query and change whether the library checks (`true`) or ignores (`false`) CRC.
- The sensor includes a heater that's intended for diagnostic purposes. (Turn on the heater, and make sure the temperature changes.) `cSHT_3x::getHeater()` queries the current state of the heater, and `cSHT_3x::setHeater(bool fOn)` turns it on or off.
- `cSHT_3x::getStatus()` reads the current value of the status register. The value is returned as an opaque structure of type `cSHT_3x::Status_t`. Methods are provided to allow clients to query individual bits. A status also has an explicit `invalid` state, which can be separately queried.
- For convenience, static methods are provided to convert between raw (`uint16_t`) data and engineering units. `cSHT_3x::rawToCelsius()` and `cSHT_3x::rawRHtoPercent()` convert raw data to engineering units. `cSHT_3x::celsiusToRawT()` and `cSHT_3x::percentRHtoRaw()` convert engineering units to raw data. (This may be useful for precalculating alarms, to save on floating point calculations at run time.)
- `cSHT_3x::isDebug()` returns `true` if this is a debug build, `false` otherwise. It's a `constexpr`, so using this in an `if()` statement is equivalent to a `#if` -- the compiler will optimize away the code if this is not a debug build.

## Header File

```c++
#include <Catena-SHT3x.h>
```

## Namespace

All definitions are wrapped in a namespace. Normally, after incluing the header file, you'll want to say:

```c++
using namespace McciCatenaSht3x;
```

## Instance Object

An instance object must be created for each SHT-3x sensor to be managed. The constructor must specify:

- The `Wire` object to be used to communicate with the sensor.
- The address of the sensor

The constructor may specify:

- the Arduino pin to be used for the `nAlert` function; use -1 if no pin is to be used.
- the Arduino pin to be used for the Reset function (use -1 if no Arduino pin is to be used).

Addresses are chosen from the special class `Address_t`; write `cSHT_3x::Address_t::A` for the first address (0x45), and `cSHT_3x::Address_t::B` for the alternate address (0x46).

```c++
enum class McciCatenaSht3x::cSHT_3x::Address_t : std::int8_t {
    Error = -1,
    A = 0x45,
    B = 0x46,
};
```

A typical initialization will look like this:

```c++
using namespace McciCatenaSht3x;

cSHT_3x mySHT_3x(
    Wire &wire,
    cSHT_3x::Address_t Address = cSHT_3x::Address_t::A,
    cSHT_3x::Pin_t pinAlert = -1,
    cSHT_3x::Pin_t pinReset = -1
    );
```

## Setting the mode

The SHT-3x datasheet doesn't give the algorighm (if any) for computing the internal checksums for commands, nor the internal bit structure of the commands. Despite the obvious regularity, we decided to resort to some hairy `constexpr` functions to allow us to build and decode commmands cleanly.

```c++
enum McciCatenaSht3x::cSHT_3x::Repeatability : std::int8_t { Error=-1, NA, Low, Medium, High };
enum McciCatenaSht3x::cSHT_3x::ClockStretching : std::uint8_t { Disabled, Enabled };
enum McciCatenaSht3x::cSHT_3x::Periodicity : std::int8_t {
    Error=-1, NA, Single, ART, HzHalf, HzOne, HzTwo HzFour, HzTen
    };

static constexpr McciCatenaSht3x::Command
McciCatenaSht3x::cSHT_3x::getCommand(
    McciCatenaSht3x::cSHT_3x::Periodicity,
    McciCatenaSht3x::cSHT_3x::Repeatability,
    McciCatenaSht3x::cSHT_3x::ClockStretching
    );

static constexpr McciCatenaSht3x::ClockStretching
McciCatenaSht3x::cSHT_3x::getClockStretching(Command);

static constexpr McciCatenaSht3x::Periodicity
McciCatenaSht3x::cSHT_3x::getPeriodicity(Command);

static constexpr McciCatenaSht3x::Repeatabilty
McciCatenaSht3x::cSHT_3x::getRepeatability(Command);

static constexpr McciCatenaSht3x::Periodicity
McciCatenaSht3x::cSHT_3x::millisToPeriodicity(uint32_t millis);

static constexpr
```

### The command constants

```c++
enum McciCatena::cSHT_3x::Command_t : std::uint16_t {
    Error                       = 0,
    ModePeriodic_Medium_HalfHz  = 0x2024,
    ModePeriodic_Low_HalfHz     = 0x202F,
    ModePeriodic_High_HalfHz    = 0x2032,
    ModePeriodic_Medium_1Hz     = 0x2126,
    ModePeriodic_Low_1Hz        = 0x212D,
    ModePeriodic_High_1Hz       = 0x2130,
    ModePeriodic_Medium_2Hz     = 0x2220,
    ModePeriodic_Low_2Hz        = 0x222B,
    ModePeriodic_High_2Hz       = 0x2236,
    ModePeriodic_Medium_4Hz     = 0x2322,
    ModePeriodic_Low_4Hz        = 0x2329,
    ModePeriodic_High_4Hz       = 0x2334,
    ModeSingle_High_Nack        = 0x2400,
    ModeSingle_Medium_Nack      = 0x240B,
    ModeSingle_Low_Nack         = 0x2416,
    ModePeriodic_Medium_10Hz    = 0x2721,
    ModePeriodic_Low_10Hz       = 0x272A,
    ModePeriodic_High_10Hz      = 0x2737,
    ModePeriodic_ART            = 0x2B32,
    ModeSingle_High_Stretch     = 0x2C06,
    ModeSingle_Medium_Stretch   = 0x2C0D,
    ModeSingle_Low_Stretch      = 0x2C10,
    ClearStatus                 = 0x3041,
    HeaterDisable               = 0x3066,
    HeaterEnable                = 0x306D,
    Break                       = 0x3093,
    SoftReset                   = 0x30A2,
    Fetch                       = 0xE000,
    GetStatus                   = 0xF32D,
};
```
