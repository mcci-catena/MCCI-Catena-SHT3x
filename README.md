# MCCI Catena&reg; SHT3x Sensor Library

This library provides a simple interface to Sensirion SHT31, SHT32, and SHT35 sensors. Although we tested this on the [MCCI Catena 4618](https://mcci.io/catena4618), there are no dependencies on MCCI hardware; this should work equally well with Adafruit breakout boards, etc.

[![GitHub release](https://img.shields.io/github/release/mcci-catena/MCCI-Catena-SHT3x.svg)](https://github.com/mcci-catena/MCCI-Catena-SHT3x/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/mcci-catena/MCCI-Catena-SHT3x/latest.svg)](https://github.com/mcci-catena/MCCI-Catena-SHT3x/compare/v0.2.1...master) <!-- [![Arduino CI](https://img.shields.io/github/workflow/status/mcci-catena/MCCI-Catena-SHT3x/Arduino%20CI)](https://github.com/mcci-catena/MCCI-Catena-SHT3x/actions) -->

<!-- TOC depthFrom:2 updateOnSave:true -->

- [Introduction](#introduction)
- [Header File](#header-file)
- [Library Dependencies](#library-dependencies)
- [Example Scripts](#example-scripts)
- [Namespace](#namespace)
- [Instance Object](#instance-object)
- [Converting between modes and command words](#converting-between-modes-and-command-words)
        - [The command constants](#the-command-constants)

<!-- /TOC -->
## Introduction

Clients interact with Sensirion sensors via the following sequence.

1. Initially, the client creates an instance object for the sensor. When creating the object, the client passes a `Wire` object (representing the I2C bus used for communication), and optionally the address.  (For future use, the library also allows you to identify Arduino pins for use as NRESET and ALERT.)

2. If the client only needs to take occasional measurements, the client calls either the `cSHT3x::getTemperatureHumidity()` method (which returns temperature and humidity scaled in engineering units), or `cSHT3x::getTemperatureHumidityRaw()` (which returns temperature and humidity as `uint16_t` unscaled values).  Generally, the former is used if data is to be processed locally on the Arduino, and the latter is used if data is to be transmitted via a LPWAN network.

3. If the client needs to make periodic measurements, the client first calls `cSHT3x::startPeriodicMeasurement()` to set the parameters for the periodic measurement, and start the acquisition process. The result of this call is the number of milliseconds per measurement.

   To collect results, the client occasionally calls `cSHT3x::getPeriodicMeasurement()` or `cSHT3x::getPeriodicMeasurementRaw()`. If a measurement is available, it will be returned, and the method returns `true`; otherwise, the method returns `false`.  To save power, the client should delay the appropriate number of milliseconds between calls (as indicated by the result of `cSHT3x::startPeriodicMeasurement()`).

Measurements are returned in structures (`cSHT3x::Measurements` or `cSHT3x::MeasurementsRaw`, respectively.) These structures have some utility methods:

- `cSHT3x::Measurments::set(const cSHT3x::MeasurementsRaw &mRaw)` sets the target `Measurement` to the engineering-units equivalent of `mRaw`.
- `cSHT3x::Measurments::extract(float &t, float &rh) const` sets `t` to the temperature (in Celsius), and `rh` to the relative humidity (in percent).
- `cSHT3x::MeasurmentsRaw::extract(std::uint16_t &t, std::uint16_t &rh) const` sets `t` and `rh` to the raw measurement from the device.

A number of utility methods allow the client to manage the sensor.

- `cSHT3x::reset()` issues a soft reset to the device.
- `cSHT3x::end()` idles the device, and is typically used prior to sleeping the system.
- By default, the library checks CRCs on received data. `cSHT3x::getCrcMode()` and `cSHT3x::setCrcMode()` allow the client to query and change whether the library checks (`true`) or ignores (`false`) CRC.
- The sensor includes a heater that's intended for diagnostic purposes. (Turn on the heater, and make sure the temperature changes.) `cSHT3x::getHeater()` queries the current state of the heater, and `cSHT3x::setHeater(bool fOn)` turns it on or off.
- `cSHT3x::getStatus()` reads the current value of the status register. The value is returned as an opaque structure of type `cSHT3x::Status_t`. Methods are provided to allow clients to query individual bits. A status also has an explicit `invalid` state, which can be separately queried.
- For convenience, static methods are provided to convert between raw (`uint16_t`) data and engineering units. `cSHT3x::rawToCelsius()` and `cSHT3x::rawRHtoPercent()` convert raw data to engineering units. `cSHT3x::celsiusToRawT()` and `cSHT3x::percentRHtoRaw()` convert engineering units to raw data. (This may be useful for pre-calculating alarms, to save on floating point calculations at run time.)
- `cSHT3x::isDebug()` returns `true` if this is a debug build, `false` otherwise. It's a `constexpr`, so using this in an `if()` statement is equivalent to a `#if` -- the compiler will optimize away the code if this is not a debug build.

## Header File

```c++
#include <Catena-SHT3x.h>
```

## Library Dependencies

None, beyond the normal Arduino library `<Wire.h>`.  It can be used with [Catena-Arduino-Platform](https://github.com/mcci-catena/Catena-Arduino-Platform), but it doesn't require it.

## Example Scripts

See [`sht3x-simple`](./examples/sht3x-simple/sht3x-simple.ino). `Sht3x-simple` reads and displays the temperature and humidity once a second, using the simple APIs.

![Screenshot of `sht3x-simple.ino` in operation](./assets/sht3x-simple-screenshot.png)

## Namespace

All definitions are wrapped in a namespace. Normally, after including the header file, you'll want to say:

```c++
using namespace McciCatenaSht3x;
```

## Instance Object

An instance object must be created for each SHT3x sensor to be managed. The constructor must specify:

- The `Wire` object to be used to communicate with the sensor.
- The address of the sensor

The constructor may specify:

- the Arduino pin to be used for the `nAlert` function; use -1 if no pin is to be used.
- the Arduino pin to be used for the Reset function (use -1 if no Arduino pin is to be used).

Addresses are chosen from the special class `Address_t`; write `cSHT3x::Address_t::A` for the first address (0x45), and `cSHT3x::Address_t::B` for the alternate address (0x46).

```c++
enum class McciCatenaSht3x::cSHT3x::Address_t : std::int8_t {
    Error = -1,
    A = 0x45,
    B = 0x46,
};
```

A typical initialization will look like this:

```c++
using namespace McciCatenaSht3x;

cSHT3x mySHT3x(
    Wire &wire,
    cSHT3x::Address_t Address = cSHT3x::Address_t::A,
    cSHT3x::Pin_t pinAlert = -1,
    cSHT3x::Pin_t pinReset = -1
    );
```

## Converting between modes and command words

The SHT3x datasheet doesn't give the algorithm (if any) for computing the internal checksums for commands, nor the internal bit structure of the commands. Despite the obvious regularity, we decided to resort to some hairy `constexpr` functions to allow us to build and decode commands cleanly.

```c++
enum McciCatenaSht3x::cSHT3x::Repeatability : std::int8_t { Error=-1, NA, Low, Medium, High };
enum McciCatenaSht3x::cSHT3x::ClockStretching : std::uint8_t { Disabled, Enabled };
enum McciCatenaSht3x::cSHT3x::Periodicity : std::int8_t {
    Error=-1, NA, Single, ART, HzHalf, HzOne, HzTwo HzFour, HzTen
    };

static constexpr McciCatenaSht3x::Command
McciCatenaSht3x::cSHT3x::getCommand(
    McciCatenaSht3x::cSHT3x::Periodicity,
    McciCatenaSht3x::cSHT3x::Repeatability,
    McciCatenaSht3x::cSHT3x::ClockStretching
    );

static constexpr McciCatenaSht3x::ClockStretching
McciCatenaSht3x::cSHT3x::getClockStretching(Command);

static constexpr McciCatenaSht3x::Periodicity
McciCatenaSht3x::cSHT3x::getPeriodicity(Command);

static constexpr McciCatenaSht3x::Repeatabilty
McciCatenaSht3x::cSHT3x::getRepeatability(Command);

static constexpr McciCatenaSht3x::Periodicity
McciCatenaSht3x::cSHT3x::millisToPeriodicity(uint32_t millis);

static constexpr
```

### The command constants

```c++
enum McciCatena::cSHT3x::Command_t : std::uint16_t {
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

## Meta

### Release History

- v0.2.1 is a patch release

  - No changes to the library apart from version.
  - Change `name=` value in `library.properties` so that IDE library installation uses the same name as a `git clone` of the repository ([#3]((https://github.com/mcci-catena/MCCI-Catena-SHT3x/issues/3))

  - Add a `library.properties` dependency on [`Catena Arduino Platform`](https://github.com/mcci-catena/Catena-Arduino-Platform).

- v0.2.0 is the first official release.

### License

This repository is released under the [MIT](./LICENSE) license. Commercial licenses are also available from MCCI Corporation.

### Support Open Source Hardware and Software

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [store.mcci.com](https://store.mcci.com/).

### Trademarks

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. LoRaWAN is a registered trademark of the LoRa Alliance. LoRa is a registered trademark of Semtech Corporation. All other marks are the property of their respective owners.
