// MIT License
// Copyright (c) 2025 Nitro_Ice
// Created by Nitro_Ice<snowhalation@gmail.com>
// 2025/2/18

#include "BS811X.h"

/*byte reverseBits(byte number) 
{
    number = (number & 0x55) << 1 | (number & 0xAA) >> 1;
    number = (number & 0x33) << 2 | (number & 0xCC) >> 2;
    number = (number & 0x0F) << 4 | (number & 0xF0) >> 4;

    return number;
}*/

uint8_t BS811X::requestFromWire(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop)
{
  if (isize > 0) {
    // send internal address; this mode allows sending a repeated start to access
    // some devices' internal registers. This function is executed by the hardware
    // TWI module on other processors (for example Due's TWI_IADR and TWI_MMR registers)

    Wire.beginTransmission(address);

    // the maximum size of internal address is 3 bytes
    if (isize > 3){
        isize = 3;
    }

    // write internal register address - most significant byte first
    while (isize-- > 0)
        Wire.write((uint8_t)(iaddress >> (isize*8)));
    Wire.endTransmission(false);
  }

  return Wire.requestFrom(address, quantity, true);
}

uint16_t BS811X::readKeys() 
{
    _prev_state = _keys;
    _keys = 0;
    requestFromWire(_address,(uint8_t) 2,(uint8_t) 0x08,(uint8_t) 1,(uint8_t) true);
    byte *buffer = (byte*) &_keys;
    uint8_t i = 0;
    while(Wire.available()) {
        buffer[i] = Wire.read();
        i++;
    }
    return _keys;
}

bool BS811X::readSetting(uint8_t * array) 
{
    requestFromWire(_address,(uint8_t) _length,(uint8_t) 0xB0,(uint8_t) 1,(uint8_t) true);
    uint8_t i = 0;
    while(Wire.available()) {
        byte buffer = Wire.read();
        array[i] = buffer;
        i++;
    }
    if(array[_length-1]>0) { return true; }
    else { return false; }
}

uint8_t BS811X::setSetting() 
{
    Wire.beginTransmission(_address);
    if(_length == 21) {
        Wire.write(_settings_1,(_length+2));
    }
    else if(_length == 17) {
        Wire.write(_settings_2,(_length+2));
    }
    return Wire.endTransmission(true);
}

uint8_t BS811X::getKey_active() 
{
    uint8_t i = 0;
    for (uint16_t mask = 0x0001; mask; mask <<= 1) {
        if (mask & _keys) {
            return i+1;
        }
        ++i;
    }
    return 0;
}

bool BS811X::getKey_passive(uint8_t key) 
{
    return bitRead(_keys, key-1);
}

bool BS811X::getKey_edge(uint8_t direction, uint8_t key) 
{
    if(direction == 1){ return !bitRead(_prev_state, key-1) && bitRead(_keys, key-1); }
    else if (direction == 2) { return bitRead(_prev_state, key-1) && !bitRead(_keys, key-1); }
    else { return false; }
}

bool BS811X::begin(String chip)
{
    if(chip == "8116") { _length = 21; }
    else if(chip == "8112") { _length = 17; }
    Wire.begin();
    delay(10);

    // One option-write attempt using whatever Wire timeout the caller has set. The chip only
    // accepts I2C while it is awake; if it is idle it clock-stretches and this times out (error 5).
    // The caller is expected to keep a short bus timeout and retry begin()/setSetting() until it
    // succeeds (which happens once the chip is woken, e.g. by a key touch).
    // setSetting() returns the I2C endTransmission() status:
    //   0 = success, 1 = data too long, 2 = NACK on address,
    //   3 = NACK on data, 4 = other error, 5 = timeout
    uint8_t result = setSetting();

    bool verified = false;
    if (result == 0)
    {
        Serial.println("BS811X: settings write successful");

        // An I2C ACK alone does not prove the chip stored the values (e.g. checksum rejected),
        // so read the option registers back and confirm what we wrote actually took effect.
        uint8_t readback[21] = {0};
        readSetting(readback);
        // readback[4] = register 0xB4 (Option2); bit6 is LSC. Expect the value we sent (DATA5).
        uint8_t expectedOption2 = (_length == 21) ? _settings_1[5] : _settings_2[5];
        verified = (readback[4] == expectedOption2);
        if (verified)
        {
            Serial.print("BS811X: settings verified, Option2 (0xB4) = 0x");
            Serial.println(readback[4], HEX);
        }
        else
        {
            Serial.print("BS811X: settings VERIFY FAILED, Option2 (0xB4) read back 0x");
            Serial.print(readback[4], HEX);
            Serial.print(", expected 0x");
            Serial.println(expectedOption2, HEX);
        }
    }
    else
    {
        Serial.print("BS811X: settings write FAILED, I2C error code ");
        Serial.println(result);
    }

    return (result == 0 && verified);
}
