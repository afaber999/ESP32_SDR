#include <Arduino.h>
#include <Wire.h>
#include "i2c_scanner.h"


void scanner_scan(int sda, int scl, uint32_t frequency)
{
    bool ok = Wire.begin(sda, scl, frequency);

    Serial.println ();
    Serial.print("I2C scanner, SDA ");
    Serial.print(sda);
    Serial.print(" SCL ");
    Serial.print(sda);
    Serial.print(" frequency ");
    Serial.print(frequency);
    Serial.println(" scannng ... ");

    byte count = 0;

    Wire.begin();
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
}
