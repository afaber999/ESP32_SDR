
#ifndef ES8388_H_INCLUDED
#define ES8388_H_INCLUDED

#include "config.h"

#ifdef ES8388_ENABLED
    enum ES8388_INPUTS {
        ADC_CHANNEL_1,
        ADC_CHANNEL_2,
        ADC_CHANNEL_DIFF_1,
        ADC_CHANNEL_DIFF_2,
    };

    void ES8388_Setup();
    void ES8388_SetDACAttenuation(uint8_t att);
    void es8388_read_range( uint8_t start, uint8_t end);
    void ES8388_SetMicGain(uint8_t gain);
    void ES8388_SelectInput(ES8388_INPUTS ch);

    void ES8388_SetOUT1VOL(uint8_t vol);
    void ES8388_SetOUT2VOL(uint8_t vol);

#endif

#endif




