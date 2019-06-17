/*

Module: Catena-SHT3x.cpp

Function:
        Code for Catnea-SHT3x library

Copyright and License:
        See accompanying LICENSE file.

Author:
        Terry Moore, MCCI Corporation   June 2019

*/

#include <Catena-SHT3x.h>

using namespace McciCatenaSht3x;


bool cSHT_3x::begin(void)
    {
    this->m_wire->begin();
    return this->reset();
    }

bool cSHT_3x::end(void)
    {
    this->reset();
    }

bool cSHT_3x::reset(void) const
    {
    if (this->writeCommand(Command::SoftReset))
        {
        delay(10);
        return true;
        }
    else
        return false;
    }

cSHT_3x::Status_t cSHT_3x::getStatus() const
    {
    bool ok;
    std::uint8_t buf[3];

    ok = this->writeCommand(Command::GetStatus);

    if (ok)
        {
        ok = this->readResponse(buf, sizeof(buf));
        }

    if (ok && ! this->m_noCrc)
        ok = this->crc(buf, 2) == buf[2];

    if (ok)
        {
        return Status_t((buf[0] << 8) | buf[1]);
        }
    else
        {
        return Status_t();
        }
    }

bool cSHT_3x::getHeater(void) const
    {
    Status_t s;
    
    s = this->getStatus();

    if (s.isValid() && s.isHeaterOn())
        return true;
    else
        return false;
    }

bool cSHT_3x::getTemperatureHumidity(
    float &t,
    float &rh,
    cSHT_3x::Repeatability r
    ) const
    {
    bool fResult;
    std::uint16_t tfrac, rhfrac;

    fResult = this->getTemperatureHumidityRaw(tfrac, rhfrac, r);

    if (fResult)
        {
        t = this->rawTtoCelsius(tfrac);
        rh = this->rawRHtoPercent(rhfrac);
        }
    else
        {
        t = rh = NAN;
        }

    return fResult;
    }

bool cSHT_3x::getTemperatureHumidityRaw(
    std::uint16_t &t,
    std::uint16_t &rh,
    cSHT_3x::Repeatability r
    ) const
    {
    bool fResult;
    Command const c = this->getCommand(
                            Periodicity::Single,
                            r,
                            ClockStretching::Disabled
                            );

    fResult = true;

    if (c == Command::Error)
        fResult = false;

    std::uint8_t buf[6];

    if (fResult)
        fResult = this->writeCommand(c);

    if (fResult)
        {
        delay(20);
        fResult = this->readResponse(buf, sizeof(buf));
        }

    if (fResult)
        fResult = this->processResultsRaw(buf, t, rh);

    return fResult;
    }

std::uint32_t cSHT_3x::startPeriodicMeasurement(Command c) const
    {
    std::uint32_t result = this->PeriodicityToMillis(this->getPeriodicity(c));

    if (result == 0)
        return result;

    // getPeriodicity() of any non-periodic command returns 0, so we're
    // ok.

    // break any previous measurement
    if (! this->writeCommand(Command::Break))
        return 0;

    // start this measurement
    if (! this->writeCommand(c))
        return 0;

    return result;
    }

bool cSHT_3x::getPeriodicMeasurement(float &t, float &rh) const
    {
    bool fResult;
    std::uint16_t tfrac, rhfrac;

    fResult = this->getPeriodicMeasurementRaw(tfrac, rhfrac);
    if (! fResult)
        return false;
    
    if (fResult)
        {
        t = this->rawTtoCelsius(tfrac);
        rh = this->rawRHtoPercent(rhfrac);
        }

    return fResult;
    }

bool cSHT_3x::getPeriodicMeasurementRaw(std::uint16_t &tfrac, std::uint16_t &rhfrac) const
    {
    bool fResult;
    std::uint8_t buf[6];

    fResult = this->writeCommand(Command::Fetch);
    if (fResult)
        fResult = this->readResponse(buf, sizeof(buf));
    if (fResult)
        fResult = this->processResultsRaw(buf, tfrac, rhfrac);
    return fResult;
    }

bool cSHT_3x::processResultsRaw(
    const std::uint8_t (&buf)[6], std::uint16_t &tfrac, std::uint16_t &rhfrac
    ) const
    {
    tfrac = (buf[0] << 8) | buf[1];
    rhfrac = (buf[3] << 8) | buf[4];

    // check CRC? use a flag to control
    if (! this->m_noCrc)
        {
        if (this->crc(buf, 2) != buf[2] ||
            this->crc(buf + 3, 2) != buf[5])
                return false;
        }

    return true;
    }

bool cSHT_3x::writeCommand(Command c) const
    {
    std::uint16_t const cbits = static_cast<std::uint16_t>(c);
    std::uint8_t result;
    const std::int8_t addr = this->getAddress();

    if (addr < 0)
        return false;

    this->m_wire->beginTransmission(addr);
    this->m_wire->write(std::uint8_t(cbits >> 8));
    this->m_wire->write(std::uint8_t(cbits & 0xFF));
    result = this->m_wire->endTransmission();

    return result == 0;
    }

bool cSHT_3x::readResponse(std::uint8_t *buf, size_t nBuf) const
    {
    bool ok;
    unsigned nResult;
    const std::int8_t addr = this->getAddress();

    if (buf == nullptr || nBuf > 32 || addr < 0)
        return false;

    this->m_wire->requestFrom(std::uint8_t(addr), /* bytes */ std::uint8_t(nBuf));

    nResult = this->m_wire->available();

    for (unsigned i = 0; i < nResult; ++i)
        buf[i] = this->m_wire->read();

    return (nResult != nBuf);
    }

std::uint8_t cSHT_3x::crc(const std::uint8_t * buf, size_t nBuf, std::uint8_t crc8)
    {
    static std::uint8_t crcTable[256] =
        {
        0x0, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97, 
        0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e, 
        0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4, 
        0xfa, 0xcb, 0x98, 0xa9, 0x3e, 0xf, 0x5c, 0x6d, 
        0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20, 0x11, 
        0x3f, 0xe, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8, 
        0xc5, 0xf4, 0xa7, 0x96, 0x1, 0x30, 0x63, 0x52, 
        0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb, 
        0x3d, 0xc, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa, 
        0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71, 0x22, 0x13, 
        0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9, 
        0xc7, 0xf6, 0xa5, 0x94, 0x3, 0x32, 0x61, 0x50, 
        0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c, 
        0x2, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95, 
        0xf8, 0xc9, 0x9a, 0xab, 0x3c, 0xd, 0x5e, 0x6f, 
        0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6, 
        0x7a, 0x4b, 0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed, 
        0xc3, 0xf2, 0xa1, 0x90, 0x7, 0x36, 0x65, 0x54, 
        0x39, 0x8, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae, 
        0x80, 0xb1, 0xe2, 0xd3, 0x44, 0x75, 0x26, 0x17, 
        0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x9, 0x5a, 0x6b, 
        0x45, 0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2, 
        0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a, 0x19, 0x28, 
        0x6, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91, 
        0x47, 0x76, 0x25, 0x14, 0x83, 0xb2, 0xe1, 0xd0, 
        0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0xb, 0x58, 0x69, 
        0x4, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93, 
        0xbd, 0x8c, 0xdf, 0xee, 0x79, 0x48, 0x1b, 0x2a, 
        0xc1, 0xf0, 0xa3, 0x92, 0x5, 0x34, 0x67, 0x56, 
        0x78, 0x49, 0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef, 
        0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24, 0x15, 
        0x3b, 0xa, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac,
        };

    for (size_t i = nBuf; i > 0; --i, ++buf)
        {
        crc8 = crcTable[*buf ^ crc8];
        }

    return crc8;
    }
