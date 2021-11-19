#include <Arduino.h>
#include <Wire.h>
#include "i2c_interface.h"

void i2c_init(int sda, int scl, uint32_t frequency)
{
    Serial.printf("i2c_init %d %d %d\n", sda, scl, frequency);

    Wire.begin(sda, scl, frequency);
}

int i2c_scan()
{
    Wire.begin();
    byte count = 0;
    
    for (byte i = 8; i < 120; i++)
    {
        Wire.beginTransmission( i );
        if (Wire.endTransmission () == 0)
        {
            Serial.print("Found address: ");
            Serial.print(i, DEC);
            Serial.print(" (0x");
            Serial.print(i, HEX);
            Serial.println(")");
            count++;
        }
    }
    Serial.print ("Found ");      
    Serial.print (count, DEC);
    Serial.println (" device(s).");
    return count;
}

void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t val) {   // write reg via i2c
    //Serial.printf("i2c_write_reg %d %d %d\n", addr, reg, val);
    Wire.begin();
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void i2c_write_blocking(uint8_t addr, uint8_t* vals, uint8_t vcnt) {
    //Serial.printf("i2c_write_blocking %d %d\n", addr, vcnt);
    Wire.begin();
    Wire.beginTransmission(addr);
    while (vcnt--) Wire.write(*vals++);
    Wire.endTransmission();
}

uint8_t i2c_read_reg(uint8_t addr, uint8_t reg)
{
    //Serial.printf("i2c_read_reg %d %d\n", addr, reg);
    Wire.begin();
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);

    uint8_t val = 0u;
    if (1 == Wire.requestFrom(uint16_t(addr), sizeof(uint8_t), true))
    {
    val = Wire.read();
    }
    Wire.endTransmission(true);
    return val;
}

