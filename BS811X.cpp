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

uint16_t BS811X::readKeys() 
{
    _prev_state = _keys;
    _keys = 0;
    //Wire.requestFrom(_address,(uint8_t) 2,(uint8_t) 0x08,(uint8_t) 1,(uint8_t) true);
    Wire.requestFrom(_address, 2, true);
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
    //Wire.requestFrom(_address,(uint8_t) _length,(uint8_t) 0xB0,(uint8_t) 1,(uint8_t) true);
    Wire.requestFrom(_address, _length, true);
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
    setSetting();
    delay(10);
    return (Wire.available() != 0);
}
