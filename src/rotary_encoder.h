#ifndef ROTARYENCODER_H_INCLUDED
#define ROTARYENCODER_H_INCLUDED

#include <Arduino.h>

class RotaryEncoder
{
public:  
  RotaryEncoder(uint8_t pinA, uint8_t pinB)
    : state_( 0)
    , pin_a_( pin_a_ )
    , pin_b_( pin_b_ )
  {
    pinMode(pin_a_, INPUT_PULLUP);
    pinMode(pin_b_, INPUT_PULLUP);    
  }

  // Read input pins and process for events. 
  // Call this function on a regular base (~every 5ms) to avoid rotary misses
  // Returns 0 on no event
  // RotaryEncoder::DirCW if rotated clockwise
  // RotaryEncoder::DirCCW if rotated counter clockwise
  uint8_t update() 
  {
    auto pinstate = (digitalRead(pin_b_) << 1) | digitalRead(pin_a_);
    state_ = ttable[state_ & 0xf][pinstate];
    return (state_ & 0x30);
  }

  static const uint8_t DirCCW;
  static const uint8_t DirCW;
  
private:
  uint8_t state_;
  uint8_t pin_a_;
  uint8_t pin_b_;
  static const uint8_t ttable[7][4];
};



#endif
