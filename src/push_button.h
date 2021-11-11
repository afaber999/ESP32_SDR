
#ifndef PUSHBUTTON_H_INCLUDED
#define PUSHBUTTON_H_INCLUDED

#include <Arduino.h>
#include <algorithm>


class InvPushButton
{
public:
    InvPushButton( uint8_t pin)
        : pin_(pin)  
        , cnt_(0)
    {
        pinMode(pin_, INPUT_PULLUP);
        update();
    }

    // call every ~10 ms
    void update() {
         if ( digitalRead(pin_) ) {
            cnt_ = std::min<int8_t>(cnt_ + 1, bounces_);
        } else {
            cnt_ = std::max(cnt_ - 1, -bounces_);
         }
    }

    bool is_pressed() const {return cnt_ == -bounces_; }
    bool is_released() const {return !is_pressed();}

private:
    uint8_t pin_;
    int8_t  cnt_;
    const int8_t bounces_ = 2;
};

#endif
