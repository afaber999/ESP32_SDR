#include "rotary_encoder.h"

const uint8_t RotaryEncoder::DirCCW = 0x10;
const uint8_t RotaryEncoder::DirCW  = 0x20;

// State table (emits a code at 00)
const uint8_t  RotaryEncoder::ttable[7][4] = 
{
  {0x00, 0x02, 0x04, 0x00}, 
  {0x03, 0x00, 0x01, DirCW},
  {0x03, 0x02, 0x00, 0x00}, 
  {0x03, 0x02, 0x01, 0x00},
  {0x06, 0x00, 0x04, 0x00}, 
  {0x06, 0x05, 0x00, DirCCW},
  {0x06, 0x05, 0x04, 0x00},
};