#include <Arduino.h>
#include "es8388.h"
#include "i2c_interface.h"

// User doc
// https://dl.radxa.com/rock2/docs/hw/ds/ES8388%20user%20Guide.pdf


/* ES8388 register */
const uint8_t ES8388_CONTROL1    =0x00;    // 00   SCPReset LRCM DACMCLK SameFs SeqEn EnRef VMIDSEL
const uint8_t ES8388_CONTROL2    =0x01;
const uint8_t ES8388_CHIPPOWER   =0x02;
const uint8_t ES8388_ADCPOWER    =0x03;
const uint8_t ES8388_DACPOWER    =0x04;
const uint8_t ES8388_CHIPLOPOW1  =0x05;
const uint8_t ES8388_CHIPLOPOW2  =0x06;
const uint8_t ES8388_ANAVOLMANAG =0x07;
const uint8_t ES8388_MASTERMODE  =0x08;

/* ADC */
const uint8_t ES8388_ADCCONTROL1  = 0x09;
const uint8_t ES8388_ADCCONTROL2  = 0x0a;
const uint8_t ES8388_ADCCONTROL3  = 0x0b;
const uint8_t ES8388_ADCCONTROL4  = 0x0c;
const uint8_t ES8388_ADCCONTROL5  = 0x0d;
const uint8_t ES8388_ADCCONTROL6  = 0x0e;
const uint8_t ES8388_ADCCONTROL7  = 0x0f;
const uint8_t ES8388_ADCCONTROL8  = 0x10;
const uint8_t ES8388_ADCCONTROL9  = 0x11;

// ADC ALC SECTION
const uint8_t ES8388_ADCCONTROL10 = 0x12;
const uint8_t ES8388_ADCCONTROL11 = 0x13;
const uint8_t ES8388_ADCCONTROL12 = 0x14;
const uint8_t ES8388_ADCCONTROL13 = 0x15;
const uint8_t ES8388_ADCCONTROL14 = 0x16;

/* DAC */
const uint8_t ES8388_DACCONTROL1  = 0x17;
const uint8_t ES8388_DACCONTROL2  = 0x18;
const uint8_t ES8388_DACCONTROL3  = 0x19;
const uint8_t ES8388_DACCONTROL4  = 0x1a;
const uint8_t ES8388_DACCONTROL5  = 0x1b;
const uint8_t ES8388_DACCONTROL6  = 0x1c;
const uint8_t ES8388_DACCONTROL7  = 0x1d;
const uint8_t ES8388_DACCONTROL8  = 0x1e;
const uint8_t ES8388_DACCONTROL9  = 0x1f;
const uint8_t ES8388_DACCONTROL10 = 0x20;
const uint8_t ES8388_DACCONTROL11 = 0x21;
const uint8_t ES8388_DACCONTROL12 = 0x22;
const uint8_t ES8388_DACCONTROL13 = 0x23;
const uint8_t ES8388_DACCONTROL14 = 0x24;
const uint8_t ES8388_DACCONTROL15 = 0x25;
const uint8_t ES8388_DACCONTROL16 = 0x26;
const uint8_t ES8388_DACCONTROL17 = 0x27; // LD2LO, LI2LO, LI2LOVOL
const uint8_t ES8388_DACCONTROL18 = 0x28;
const uint8_t ES8388_DACCONTROL19 = 0x29;
const uint8_t ES8388_DACCONTROL20 = 0x2a;
const uint8_t ES8388_DACCONTROL21 = 0x2b;
const uint8_t ES8388_DACCONTROL22 = 0x2c;
const uint8_t ES8388_DACCONTROL23 = 0x2d;
const uint8_t ES8388_DACCONTROL24 = 0x2e;
const uint8_t ES8388_DACCONTROL25 = 0x2f;
const uint8_t ES8388_DACCONTROL26 = 0x30;
const uint8_t ES8388_DACCONTROL27 = 0x31;
const uint8_t ES8388_DACCONTROL28 = 0x32;
const uint8_t ES8388_DACCONTROL29 = 0x33;
const uint8_t ES8388_DACCONTROL30 = 0x34;


uint8_t ES8388_ReadReg(uint8_t reg)
{
    return i2c_read_reg(ES8388_ADDR, reg);
}

void ES8388_WriteReg(uint8_t reg, uint8_t val)
{
    i2c_write_reg(ES8388_ADDR, reg, val);
}


void es8388_read_range(uint8_t start, uint8_t end)
{
    delay(1);
    for (int i = start; i < end; i++)
    {
        uint8_t reg = 0;
        reg = ES8388_ReadReg(i);
        Serial.printf("Reg 0x%02x = 0x%02x\n", i, reg);
    }
}

void ES8388_SetMicGain(uint8_t gain)
{
    if (gain > 8) gain = 8;
    ES8388_WriteReg(ES8388_ADCCONTROL1, gain + (gain << 4)); // MicAmpL, MicAmpR
}

// Select input
void ES8388_SelectInput(ES8388_INPUTS ch)
{
    uint8_t reg2 = 0;
    uint8_t reg3 = ES8388_ReadReg(ES8388_ADCCONTROL3);

    switch (ch) {
        default:
        case ADC_CHANNEL_1:
            reg2 = 0b00000000;
        break;

        case ADC_CHANNEL_2:
            reg2 = 0b01010000;
        break;

        case ADC_CHANNEL_DIFF_1:
            reg2 = 0b11110000;
            reg3 &= 0b01111111; 
        break;
        case ADC_CHANNEL_DIFF_2:
            reg2 = 0b11110001;
            reg3 |= 0b10000000; 
        break;
    }
    ES8388_WriteReg(ES8388_ADCCONTROL2, reg2);
    ES8388_WriteReg(ES8388_ADCCONTROL3, reg3);
}

void ES8388_SetMixInCh(uint8_t ch, float var)
{
    if (var > 0)
    {
        uint8_t in = 0;
        switch (ch)
        {
        case 0:
            in = 0;
            Serial.printf("MixCh0!\n");
            break;

        case 1:
            in = 1;
            Serial.printf("MixCh1!\n");
            break;

        case 2:
            in = 3;
            Serial.printf("MixChAMPL!\n");
            break;

        default:
            Serial.printf("Illegal Mix Input ch!\n");
            return;
        }
        // ES8388_DACCONTROL16
        ES8388_WriteReg(ES8388_DACCONTROL16, in + (in << 3)); // LMIXSEL, RMIXSEL
#ifdef STATUS_ENABLED
        Status_ValueChangedInt("Mix In Ch", in);
#endif
    }
}


void ES8388_SetOUT1VOL(uint8_t vol)
{
    if (vol> 33) vol = 33;
    ES8388_WriteReg(ES8388_DACCONTROL24, vol); // LOUT1VOL
    ES8388_WriteReg(ES8388_DACCONTROL25, vol); // ROUT1VOL
}

void ES8388_SetOUT2VOL(uint8_t vol)
{
    if (vol> 33) vol = 33;
    ES8388_WriteReg(ES8388_DACCONTROL26, vol); // LOUT2VOL
    ES8388_WriteReg(ES8388_DACCONTROL27, vol); // ROUT2VOL
}


void ES8388_Standby() {
    // See section 10.5 of user guide
    // Power Down Sequence (To Standby Mode)
    ES8388_WriteReg(ES8388_ADCCONTROL7, 0x34);  // ADC mute
    ES8388_WriteReg(ES8388_DACCONTROL3, 0x36);  // DAC mute
    ES8388_WriteReg(ES8388_CHIPPOWER, 0xF3);    // Power down DEM and STM
    ES8388_WriteReg(ES8388_ADCPOWER, 0xFC);     // Power down ADC
    ES8388_WriteReg(ES8388_DACPOWER, 0xC0);     // Power down DAC
}

void ES8388_Resume() {
    // See section 10.6 of user guide
    // Resume from standby
    ES8388_WriteReg(ES8388_ADCPOWER, 0x00);     // Power up ADC
    ES8388_WriteReg(ES8388_DACPOWER, 0x3C);     // Power up DAC
    ES8388_WriteReg(ES8388_ADCCONTROL7, 0x30);  // ADC unmute
    ES8388_WriteReg(ES8388_DACCONTROL3, 0x32);  // DAC unmute
    ES8388_WriteReg(ES8388_CHIPPOWER, 0x00);    // Power up DEM and STM
}

void ES8388_SetupCodec() {
        // see user guide section 10.1 (startup for codec)
    ES8388_WriteReg(ES8388_MASTERMODE, 0x00);   // slave mode
    ES8388_WriteReg(ES8388_CHIPPOWER, 0x3F);    // power down DEM and STM
    ES8388_WriteReg(ES8388_DACCONTROL21, 0x80); // same LRCK
    ES8388_WriteReg(ES8388_CONTROL1, 0x05);     // play and record mode
    ES8388_WriteReg(ES8388_CONTROL2, 0x40);     // power analog and ibias
    ES8388_WriteReg(ES8388_ADCPOWER, 0x00);     // Power up ADC
    ES8388_WriteReg(ES8388_DACPOWER, 0x3C);     // Power up DAC

    // ADC setup
    ES8388_WriteReg(ES8388_ADCCONTROL2, 0x00);  // Select inputs Lin1/Rin1
    ES8388_WriteReg(ES8388_ADCCONTROL1, 0x88);  // input gain for ADC (24 dB)
    ES8388_WriteReg(ES8388_ADCCONTROL4, 0x0C);  // DATSEL, ADCLRP, ADCWL, ADCFORMAT (16 bits)
    ES8388_WriteReg(ES8388_ADCCONTROL5, 0x02);  // ADCFsMode , ADCFsRatio

    // Set ADC Digital Volume
    ES8388_WriteReg(ES8388_ADCCONTROL8, 0x00);  // LADCVOL 0 dB
    ES8388_WriteReg(ES8388_ADCCONTROL9, 0x00);  // RADCVOL 0 dB

    // ALC keep default (no ALC)
    // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL10, 0x16); // ALC OFF, 
    // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL11, 0xb0);
    // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL12, 0x32);
    // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL13, 0x06);
    // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL14, 0x00);

    // DAC setup
    ES8388_WriteReg(ES8388_DACCONTROL1, 0x18);  // DACLRSWAP, DACLRP, DACWL, DACFORMAT (16 bits)
    ES8388_WriteReg(ES8388_DACCONTROL2, 0x02);  // DACFsMode , DACFsRatio (256)
    ES8388_WriteReg(ES8388_DACCONTROL4, 0x00);  // Left volume (0 dB)
    ES8388_WriteReg(ES8388_DACCONTROL5, 0x00);  // Right volume (0 dB)

    // Setup Mixer
    // default ES8388_WriteReg(ES8388_DACCONTROL16, 0x09); // NOT USED
    ES8388_WriteReg(ES8388_DACCONTROL17, 0x80); // enable DAC out and disable bypass 
    ES8388_WriteReg(ES8388_DACCONTROL20, 0x80); // enable DAC out and disable bypass

    // DAC output volume
    ES8388_WriteReg(ES8388_DACCONTROL24, 0x1E); // LOUT1VOL 0 dB
    ES8388_WriteReg(ES8388_DACCONTROL25, 0x1E); // ROUT1VOL 0 dB

    ES8388_WriteReg(ES8388_DACCONTROL26, 0x1E); // LOUT2VOL 0 dB
    ES8388_WriteReg(ES8388_DACCONTROL27, 0x1E); // ROUT2VOL 0 dB

    // Set same LRCK
    ES8388_WriteReg(ES8388_DACCONTROL21, 0x80);

    // Start up
    ES8388_WriteReg(ES8388_CHIPPOWER, 0x00);    // power up DEM and STM
}


void ES8388_Setup()
{
    Serial.printf("Connect to ES8388 codec... ");

    delay(100);
    ES8388_SetupCodec();


    Serial.printf("ES8388 setup finished!\n");
}

// void program_defaults() {
//     ES8388_WriteReg(0x00, 0x05); // PLAY AND RECORD MODE
//     ES8388_WriteReg(0x01, 0x40); // POWER UP ANALOG BIAS
//     ES8388_WriteReg(ES8388_CHIPPOWER, 0x00);
//     ES8388_WriteReg(ES8388_ADCPOWER, 0x00); // POWER UP ADC ANALOG INPUT
//     ES8388_WriteReg(ES8388_DACPOWER, 0x3c); // POWER UP DAC AND ENABLE LOUT/ROUT
//     // DEFAULT ES8388_WriteReg(ES8388_CHIPLOPOW1, 0x00);
//     // DEFAULT ES8388_WriteReg(ES8388_CHIPLOPOW2, 0x00);
//     ES8388_WriteReg(ES8388_ANAVOLMANAG, 0x7c);
//     ES8388_WriteReg(ES8388_MASTERMODE, 0x00); // SLAVE MODE

//     ES8388_WriteReg(ES8388_ADCCONTROL1, 0x33);  
//     ES8388_WriteReg(ES8388_ADCCONTROL2, 0x50);
//     ES8388_WriteReg(0x0b, 0x02);
//     ES8388_WriteReg(ES8388_ADCCONTROL4, 0x0c);
//     ES8388_WriteReg(ES8388_ADCCONTROL5, 0x02);
//     ES8388_WriteReg(0x0e, 0x30);
//     ES8388_WriteReg(0x0f, 0x20);

//     ES8388_WriteReg(ES8388_ADCCONTROL8, 0x00);
//     ES8388_WriteReg(ES8388_ADCCONTROL9, 0x00);

//     // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL10, 0x16); // ALC OFF, 
//     // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL11, 0xb0);
//     // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL12, 0x32);
//     // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL13, 0x06);
//     // DEFAULT ES8388_WriteReg(ES8388_ADCCONTROL14, 0x00);

//     ES8388_WriteReg(ES8388_DACCONTROL1, 0x18);
//     ES8388_WriteReg(0x18, 0x02);
//     ES8388_WriteReg(0x19, 0x22);
//     ES8388_WriteReg(0x1a, 0x00);
//     ES8388_WriteReg(0x1b, 0xc0);
//     ES8388_WriteReg(0x1c, 0x08);
//     ES8388_WriteReg(0x1d, 0x00);
//     ES8388_WriteReg(0x1e, 0x1f);
//     ES8388_WriteReg(0x1f, 0xf7);
//     ES8388_WriteReg(0x20, 0xfd);
//     ES8388_WriteReg(0x21, 0xff);
//     ES8388_WriteReg(0x22, 0x1f);
//     ES8388_WriteReg(0x23, 0xf7);
//     ES8388_WriteReg(0x24, 0xfd);
//     ES8388_WriteReg(0x25, 0xff);
//     ES8388_WriteReg(0x26, 0x1b);

//     ES8388_WriteReg(ES8388_DACCONTROL17, 0xb8);

//     // DEFAULT ES8388_WriteReg(ES8388_DACCONTROL18, 0x28);
//     // DEFAULT ES8388_WriteReg(ES8388_DACCONTROL19, 0x28);

//     ES8388_WriteReg(ES8388_DACCONTROL20, 0xb8); // 1011 1000 

//     ES8388_WriteReg(ES8388_DACCONTROL21, 0x80); // DACLRC = ADLCR DAC LRCK 

//     ES8388_WriteReg(ES8388_DACCONTROL22, 0x00); // DC offset 0

//     ES8388_WriteReg(ES8388_DACCONTROL23, 0x00); // VROI 1.5k VREF

//     ES8388_WriteReg(ES8388_DACCONTROL24, 0x21); // LOUT1VOL -4.5 dB
//     ES8388_WriteReg(ES8388_DACCONTROL25, 0x21); // ROUT1VOL -4.5 dB

//     ES8388_WriteReg(ES8388_DACCONTROL26, 0x21); // LOUT2VOL -4.5 dB
//     ES8388_WriteReg(ES8388_DACCONTROL27, 0x21); // ROUT2VOL -4.5 dB

//     // DEFAULLTS
//     // ES8388_WriteReg(ES8388_DACCONTROL28, 0x00);
//     // ES8388_WriteReg(ES8388_DACCONTROL29, 0xaa);
//     // ES8388_WriteReg(ES8388_DACCONTROL30, 0xaa);    
// }


void program_defaults() {
    ES8388_WriteReg(0x00, 0x05);
    ES8388_WriteReg(0x01, 0x40);
    ES8388_WriteReg(0x02, 0x00);
    ES8388_WriteReg(0x03, 0x00);
    ES8388_WriteReg(0x04, 0x3c);
    ES8388_WriteReg(0x05, 0x00);
    ES8388_WriteReg(0x06, 0x00);
    ES8388_WriteReg(0x07, 0x7c);
    ES8388_WriteReg(0x08, 0x00);
    ES8388_WriteReg(0x09, 0x33);
    ES8388_WriteReg(0x0a, 0x50);
    ES8388_WriteReg(0x0b, 0x02);
    ES8388_WriteReg(0x0c, 0x0c);
    ES8388_WriteReg(0x0d, 0x02);
    ES8388_WriteReg(0x0e, 0x30);
    ES8388_WriteReg(0x0f, 0x20);
    ES8388_WriteReg(0x10, 0x00);
    ES8388_WriteReg(0x11, 0x00);
    ES8388_WriteReg(0x12, 0x16);
    ES8388_WriteReg(0x13, 0xb0);
    ES8388_WriteReg(0x14, 0x32);
    ES8388_WriteReg(0x15, 0x06);
    ES8388_WriteReg(0x16, 0x00);
    ES8388_WriteReg(0x17, 0x18);
    ES8388_WriteReg(0x18, 0x02);
    ES8388_WriteReg(0x19, 0x22);
    ES8388_WriteReg(0x1a, 0x00);
    ES8388_WriteReg(0x1b, 0xc0);
    ES8388_WriteReg(0x1c, 0x08);
    ES8388_WriteReg(0x1d, 0x00);
    ES8388_WriteReg(0x1e, 0x1f);
    ES8388_WriteReg(0x1f, 0xf7);
    ES8388_WriteReg(0x20, 0xfd);
    ES8388_WriteReg(0x21, 0xff);
    ES8388_WriteReg(0x22, 0x1f);
    ES8388_WriteReg(0x23, 0xf7);
    ES8388_WriteReg(0x24, 0xfd);
    ES8388_WriteReg(0x25, 0xff);
    ES8388_WriteReg(0x26, 0x1b);
    ES8388_WriteReg(0x27, 0xb8);
    ES8388_WriteReg(0x28, 0x28);
    ES8388_WriteReg(0x29, 0x28);
    ES8388_WriteReg(0x2a, 0xb8);
    ES8388_WriteReg(0x2b, 0x80);
    ES8388_WriteReg(0x2c, 0x00);
    ES8388_WriteReg(0x2d, 0x00);
    ES8388_WriteReg(0x2e, 0x21);
    ES8388_WriteReg(0x2f, 0x21);
    ES8388_WriteReg(0x30, 0x21);
    ES8388_WriteReg(0x31, 0x21);
    ES8388_WriteReg(0x32, 0x00);
    ES8388_WriteReg(0x33, 0xaa);
    ES8388_WriteReg(0x34, 0xaa);    
}
