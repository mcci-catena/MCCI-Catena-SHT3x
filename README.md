# MCCI Catena&reg; SHT-3x Sensor Library

This library provides a simple interface to Sensirion SHT-31, SHT-32, and SHT-35 sensors.

## Header File

```c++
#include <Catena-SHT3x.h>
```

## Namespace

using namespace McciCatenaSht3x;

## Instance Object

An instance object must be created for each SHT-3x sensor to be managed. The constructor must specify:

- The `Wire` object to be used to communicate with the sensor.
- The address of the sensor

The constructor may specify:

- the Arduino pin to be used for the `nAlert` function; use -1 if no pin is to be used.
- the Arduino pin to be used for the Reset function (use -1 if no Arduino pin is to be used).

```c++
enum class McciCatenaSht3x::cSHT_3x::Address_t : std::int8_t {
    Error = -1,
    A = 0x45,
    B = 0x46,
};

McciCatenaSht3x::cSHT_3x mySHT_3x(
    Wire &wire,
    McciCatenaSht3x::cSHT_3x::Address_t Address = McciCatenaSht3x::cSHT_3x::Address_t::A,
    McciCatenaSht3x::cSHT_3x::Pin_t pinAlert = -1,
    McciCatenaSht3x::cSHT_3x::Pin_t pinReset = -1
    );
```

## Setting the mode

The SHT-3x datasheet doesn't give the algorighm (if any) for computing the internal checksums for commands, nor the internal bit structure. Despite the obvious regularity, we have to resort to some hairy `constexpr` functions to allow us to build and decode commmands cleanly.

```c++
enum McciCatenaSht3x::cSHT_3x::Repeatability : std::int8_t { Error=-1, NA, Low, Medium, High };
enum McciCatenaSht3x::cSHT_3x::ClockStretching : std::uint8_t { Disabled, Enabled };
enum McciCatenaSht3x::cSHT_3x::Periodicity : std::int8_t {
    Error=-1, NA, Single, ART, HzHalf, HzOne, HzTwo HzFour, HzTen
    };

static constexpr McciCatenaSht3x::Command
getCommand(
    McciCatenaSht3x::cSHT_3x::Periodicity,
    McciCatenaSht3x::cSHT_3x::Repeatability,
    McciCatenaSht3x::cSHT_3x::ClockStretching
    );

static constexpr McciCatenaSht3x::ClockStretching
getClockStretching(Command);

static constexpr McciCatenaSht3x::Periodicity
McciCatenaSht3x::cSHT_3x::getPeriodicity(Command);

static constexpr McciCatenaSht3x::Repeatabilty
getRepeatability(Command);

constexpr McciCatenaSht3x::Periodicity
millisToPeriodicity(uint32_t millis);

static constexpr McciCatenaSht3x::cSHT_3x::Command McciCatenaSht3x::cSHT_3x::getCommand(
    McciCatenaSht3x::cSHT_3x::Periodicity &,
    McciCatenaSht3x::cSHT_3x::Repeatability &,
    McciCatenaSht3x::cSHT_3x::ClockStretching &
    );
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

## The methods

### The constructor

