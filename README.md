# MCCI Catena(r) SHT-3x Sensor Library

## Header File

```c++
#include <Catena-SHT-3x.h>
```

## Namespace

using namespace McciCatena;

## Instance Object

```c++
class McciCatena::cSHT_3x : public McciCatena::cPollableObject;

McciCatena::cSHT_3x gMySHT_3x(
    McicCatena::cSHT_3x::Address_t Address = McicCatena::cSHT_3x::Address_t::A,
    std::int8_t pinAlert = -1,
    std:int8_t pinReset = -1
    );

enum class McicCatena::cSHT_3x::Address_t : std::int8_t {
    Error = -1,
    A = 0x45,
    B = 0x46,
}
```

## Setting the mode

```c++
enum McciCatena::cSHT_3x::Repeatability : std::uint8_t { NA, Low, Medium, High };
enum McciCatena::cSHT_3x::ClockStretching : std::uint8_t { Disabled, Enabled };
enum McciCatena::cSHT_3x::Periodicity : std::uint8_t {
    Single, ART, HzHalf, HzOne, HzTwo HzFour, HzTen
    };

static McciCatenaSht3x::Command_t GetModeCommand(
    McciCatena::cSHT_3x::Periodicity,
    McciCatena::cSHT_3x::Repeatability,
    McciCatena::cSHT_3x::ClockStretching
    );

void McciCatena::cSHT_3x::SetMode(
    McciCatena::cSHT_3x::Periodicity,
    McciCatena::cSHT_3x::Repeatability,
    McciCatena::cSHT_3x::ClockStretching
    );

void McciCatena::cSHT_3x::SetMode(
    McciCatena::cSHT_3x::Periodicity,
    McciCatena::cSHT_3x::Repeatability,
    McciCatena::cSHT_3x::ClockStretching
    );

void McciCatena::cSHT_3x::GetMode(
    McciCatena::cSHT_3x::Periodicity &,
    McciCatena::cSHT_3x::Repeatability &,
    McciCatena::cSHT_3x::ClockStretching &
    ) const;
```

## The commands

```c++
enum McciCatena::cSHT_3x::Command_t : std::uint16_t {
    GetStatus                   = 0xF32D,
    ClearStatus                 = 0x3041,
    SoftReset                   = 0x30A2,
    Break                       = 0x3093,
    ModePeriodic_ART            = 0x2B32,
    ModePeriodic_High_HalfHz    = 0x2032,
    ModePeriodic_Medium_HalfHz  = 0x2024,
    ModePeriodic_Low_HalfHz     = 0x202F,
    ModePeriodic_High_1Hz       = 0x2130,
    ModePeriodic_Medium_1Hz     = 0x2126,
    ModePeriodic_Low_1Hz        = 0x212D,
    ModePeriodic_High_2Hz       = 0x2236,
    ModePeriodic_Medium_2Hz     = 0x2220,
    ModePeriodic_Low_2Hz        = 0x222B,
    ModePeriodic_High_4Hz       = 0x2334,
    ModePeriodic_Medium_4Hz     = 0x2322,
    ModePeriodic_Low_4Hz        = 0x2329,
    ModePeriodic_High_10Hz      = 0x2737,
    ModePeriodic_Medium_10Hz    = 0x2721,
    ModePeriodic_Low_10Hz       = 0x272A,
    ModeSingle_High_Stretch     = 0x2C06,
    ModeSingle_Medium_Stretch   = 0x2C0D,
    ModeSingle_Low_Stretch      = 0x2C10,
    ModeSingle_High_Nack        = 0x2400,
    ModeSingle_Medium_Nack      = 0x240B,
    ModeSingle_Low_Nack         = 0x2416,
};
```
