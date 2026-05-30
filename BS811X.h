// MIT License
// Copyright (c) 2025 Nitro_Ice
// Created by Nitro_Ice<snowhalation@gmail.com>
// 2025/2/18

#ifndef BS811X_H
#define BS811X_H

#include <Arduino.h>
#include <Wire.h>

#define BS811X_LSF 0x50
// Device address (the address without the R/W bit0) bs811x identify code: 1010, device address: 000 -> 1010000 = 0x50


class BS811X {
    uint8_t _address;
    uint16_t _keys = 0;
    uint16_t _prev_state = 0;
    uint8_t _length;
    // DATA5 = Option2 (reg 0xB4): bit6 = LSC. LSC=1 (0xD8) -> low-power slow scan (~0.5-1s
    // response time); the chip sleeps between scans and stretches the I2C clock while asleep,
    // blocking the master for the full Wire timeout. LSC=0 (0x98) -> fast/active mode, always
    // responsive. Last byte is CheckSum = (DATA1+...+DATAn) & 0xFF; recomputed for LSC=0.
    uint8_t _settings_1[23] = {0xB0,0,0,0x83,0xF3,0x98,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0x8E};
    uint8_t _settings_2[19] = {0xB0,0,0,0x83,0xF3,0x98,8,8,8,8,8,8,8,8,8,8,8,8,0x6E};

public:
    // Constructor
    // param address I2C address of BS811X device
    explicit BS811X(uint8_t address = BS811X_LSF) : _address(address) {}

    // Read data from BS811X. Should call before processing data.
    // Return states of 16 keys in bitmap representation
    uint16_t readKeys();
    // Read setting Register from BS811X. 
    // Return true if the reading is accomplished
    // put 21/17 bytes register data into the array
    bool readSetting(uint8_t * array);
    // Write to the setting register in BS811X. 
    // Return endTransmission param
    // predefined in "_settings"
    uint8_t setSetting();
    // Return number of first pressed key 
    // return 0 if none key is pressed
    uint8_t getKey_active();
    // Return true if a key is pressed
    // param key Number of key
    bool getKey_passive(uint8_t key);
    // Return true if a edge is detected
    // param edge direction/1/2, key Number of key
    bool getKey_edge(uint8_t direction, uint8_t key) ;
    // Return true if the initialize process has been a success
    // param chip type
    bool begin(String chip);

private:
    uint8_t requestFromWire(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop);

};


#endif //BS811X_H
